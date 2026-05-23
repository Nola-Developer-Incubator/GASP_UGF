// Copyright © 2026 Nola Development Incubator. All Rights Reserved.

#include "TokenManager.h"
#include "JsonObjectConverter.h"
#include "Http.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Base64.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <bcrypt.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

FTokenManager::FTokenManager()
	: KeyRotationInterval(24.0f) // 24 hours default
	, bAllowInsecureTokenValidation(false)
	, TokenClockSkewSeconds(60)
{
}

FTokenManager::~FTokenManager()
{
}

bool FTokenManager::Initialize(
	const FString& InJwksURL,
	bool bInAllowInsecureTokenValidation,
	const FString& InExpectedAudience,
	const TArray<FString>& InAllowedIssuers,
	int32 InTokenClockSkewSeconds)
{
	if (InJwksURL.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: JWKS URL is empty"));
		return false;
	}

	JwksURL = InJwksURL;
	bAllowInsecureTokenValidation = bInAllowInsecureTokenValidation;
	ExpectedAudience = InExpectedAudience;
	AllowedIssuers.Reset();
	for (const FString& Issuer : InAllowedIssuers)
	{
		if (!Issuer.IsEmpty())
		{
			AllowedIssuers.Add(Issuer);
		}
	}
	TokenClockSkewSeconds = FMath::Max(0, InTokenClockSkewSeconds);
	UE_LOG(LogTemp, Log, TEXT("TokenManager: Initialized with JWKS URL: %s"), *JwksURL);
	if (bAllowInsecureTokenValidation)
	{
		UE_LOG(LogTemp, Warning, TEXT("TokenManager: Insecure token validation fallback is enabled for development use"));
	}
	if (ExpectedAudience.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("TokenManager: Expected token audience is not configured; audience validation will be skipped"));
	}
	if (AllowedIssuers.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("TokenManager: Allowed token issuers are not configured; issuer validation will be skipped"));
	}

	// Fetch initial JWKS
	return FetchJWKS();
}

bool FTokenManager::FetchJWKS()
{
	if (JwksURL.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: Cannot fetch JWKS, URL not set"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("TokenManager: Fetching JWKS from %s"), *JwksURL);

	FString JwksJson;

	// Allow local development to point directly at inline JSON or a local file.
	if (JwksURL.StartsWith(TEXT("{")))
	{
		JwksJson = JwksURL;
	}
	else if (FPaths::FileExists(JwksURL))
	{
		if (!FFileHelper::LoadFileToString(JwksJson, *JwksURL))
		{
			UE_LOG(LogTemp, Error, TEXT("TokenManager: Failed to load JWKS file from %s"), *JwksURL);
			return false;
		}
	}
	else
	{
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
		Request->SetVerb(TEXT("GET"));
		Request->SetURL(JwksURL);
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

		FEvent* CompletionEvent = FPlatformProcess::GetSynchEventFromPool(true);
		bool bRequestCompleted = false;
		bool bRequestSucceeded = false;

		Request->OnProcessRequestComplete().BindLambda(
			[&JwksJson, &bRequestCompleted, &bRequestSucceeded, CompletionEvent](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
			{
				bRequestSucceeded = bSucceeded && HttpResponse.IsValid() && EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode());
				if (bRequestSucceeded && HttpResponse.IsValid())
				{
					JwksJson = HttpResponse->GetContentAsString();
				}

				bRequestCompleted = true;
				CompletionEvent->Trigger();
			});

		if (!Request->ProcessRequest())
		{
			FPlatformProcess::ReturnSynchEventToPool(CompletionEvent);
			UE_LOG(LogTemp, Error, TEXT("TokenManager: Failed to start JWKS HTTP request"));
			return false;
		}

		const bool bSignaled = CompletionEvent->Wait(5000);
		FPlatformProcess::ReturnSynchEventToPool(CompletionEvent);

		if (!bSignaled || !bRequestCompleted || !bRequestSucceeded)
		{
			UE_LOG(LogTemp, Error, TEXT("TokenManager: JWKS fetch request failed or timed out"));
			return false;
		}
	}

	const bool bParsed = ParseJWKS(JwksJson);
	if (bParsed)
	{
		LastKeyFetch = FDateTime::UtcNow();
	}

	return bParsed;
}

bool FTokenManager::ValidateToken(const FString& TokenString, FJWTToken& OutToken)
{
	OutToken = FJWTToken();

	if (TokenString.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: Token string is empty"));
		return false;
	}

	TSharedPtr<FJsonObject> Header;
	TSharedPtr<FJsonObject> Payload;
	TArray<uint8> Signature;
	FString SignedData;

	if (Keys.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: No JWKS keys are loaded"));
		return false;
	}

	if (ShouldRotateKeys() && !FetchJWKS())
	{
		UE_LOG(LogTemp, Warning, TEXT("TokenManager: Key rotation refresh failed, continuing with existing keys"));
	}

	if (!ParseJWT(TokenString, Header, Payload, Signature, SignedData))
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: Failed to parse JWT"));
		return false;
	}

	// Extract header fields
	FString Algorithm = Header->GetStringField(TEXT("alg"));
	FString KeyId = Header->HasField(TEXT("kid")) ? Header->GetStringField(TEXT("kid")) : FString();

	if (Algorithm != TEXT("RS256"))
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: Unsupported algorithm: %s"), *Algorithm);
		return false;
	}

	// Find the appropriate key
	const FJWK* Key = FindKey(KeyId);
	if (!Key)
	{
		if (bAllowInsecureTokenValidation)
		{
			UE_LOG(LogTemp, Warning, TEXT("TokenManager: Key not found for kid '%s'; insecure development fallback is in use"), *KeyId);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("TokenManager: Key not found for kid: %s"), *KeyId);
			return false;
		}
	}

	const bool bSignatureVerified = Key ? VerifyRS256Signature(SignedData, Signature, *Key) : false;
	if (!bSignatureVerified)
	{
		if (!bAllowInsecureTokenValidation)
		{
			UE_LOG(LogTemp, Error, TEXT("TokenManager: Signature verification failed for token with kid '%s'"), *KeyId);
			return false;
		}

		UE_LOG(LogTemp, Warning, TEXT("TokenManager: Signature verification bypassed because insecure development fallback is enabled"));
	}

	// Extract claims
	OutToken.RawToken = TokenString;
	OutToken.Subject = Payload->HasField(TEXT("sub")) ? Payload->GetStringField(TEXT("sub")) : FString();
	OutToken.Issuer = Payload->HasField(TEXT("iss")) ? Payload->GetStringField(TEXT("iss")) : FString();

	if (Payload->HasField(TEXT("exp")))
	{
		OutToken.ExpirationTime = static_cast<int64>(Payload->GetNumberField(TEXT("exp")));
	}

	if (Payload->HasField(TEXT("iat")))
	{
		OutToken.IssuedAt = static_cast<int64>(Payload->GetNumberField(TEXT("iat")));
	}

	if (Payload->HasField(TEXT("nbf")))
	{
		OutToken.NotBefore = static_cast<int64>(Payload->GetNumberField(TEXT("nbf")));
	}

	if (Payload->HasField(TEXT("aud")))
	{
		const TSharedPtr<FJsonValue> AudienceField = Payload->TryGetField(TEXT("aud"));
		if (AudienceField.IsValid())
		{
			if (AudienceField->Type == EJson::String)
			{
				OutToken.Audiences.Add(AudienceField->AsString());
			}
			else if (AudienceField->Type == EJson::Array)
			{
				for (const TSharedPtr<FJsonValue>& AudienceValue : AudienceField->AsArray())
				{
					const FString Audience = AudienceValue.IsValid() ? AudienceValue->AsString() : FString();
					if (!Audience.IsEmpty())
					{
						OutToken.Audiences.Add(Audience);
					}
				}
			}
		}
	}

	// Extract capabilities array
	if (Payload->HasField(TEXT("capabilities")))
	{
		const TArray<TSharedPtr<FJsonValue>>* CapabilitiesArray;
		if (Payload->TryGetArrayField(TEXT("capabilities"), CapabilitiesArray))
		{
			for (const TSharedPtr<FJsonValue>& Value : *CapabilitiesArray)
			{
				OutToken.Capabilities.Add(Value->AsString());
			}
		}
	}

	// Extract policy version
	if (Payload->HasField(TEXT("policy_version")))
	{
		OutToken.PolicyVersion = Payload->GetStringField(TEXT("policy_version"));
	}

	if (IsTokenExpired(OutToken))
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: Token is expired"));
		return false;
	}

	if (!ValidateConfiguredClaims(OutToken))
	{
		return false;
	}

	OutToken.bIsValid = true;

	UE_LOG(LogTemp, Log, TEXT("TokenManager: Token validated - Subject: %s, AudienceCount: %d, Capabilities: %d"),
		*OutToken.Subject, OutToken.Audiences.Num(), OutToken.Capabilities.Num());

	return true;
}

bool FTokenManager::IsTokenExpired(const FJWTToken& Token) const
{
	if (!Token.bIsValid && Token.ExpirationTime == 0)
	{
		return true;
	}

	const int64 CurrentTime = FDateTime::UtcNow().ToUnixTimestamp();
	return Token.ExpirationTime > 0 && (CurrentTime - TokenClockSkewSeconds) >= Token.ExpirationTime;
}

TArray<FString> FTokenManager::GetCapabilities(const FJWTToken& Token) const
{
	return Token.Capabilities;
}

FString FTokenManager::GetPolicyVersion(const FJWTToken& Token) const
{
	return Token.PolicyVersion;
}

bool FTokenManager::ShouldRotateKeys() const
{
	FTimespan TimeSinceLastFetch = FDateTime::UtcNow() - LastKeyFetch;
	return LastKeyFetch > FDateTime::MinValue() && TimeSinceLastFetch.GetTotalHours() >= KeyRotationInterval;
}

void FTokenManager::RefreshKeys()
{
	UE_LOG(LogTemp, Log, TEXT("TokenManager: Refreshing keys"));
	FetchJWKS();
}

bool FTokenManager::ValidateConfiguredClaims(const FJWTToken& Token) const
{
	const int64 CurrentTime = FDateTime::UtcNow().ToUnixTimestamp();

	if (!ExpectedAudience.IsEmpty() && !Token.Audiences.Contains(ExpectedAudience))
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: Token audience does not include expected audience '%s'"), *ExpectedAudience);
		return false;
	}

	if (AllowedIssuers.Num() > 0 && !AllowedIssuers.Contains(Token.Issuer))
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: Token issuer '%s' is not in the configured allow-list"), *Token.Issuer);
		return false;
	}

	if (Token.NotBefore > 0 && (CurrentTime + TokenClockSkewSeconds) < Token.NotBefore)
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: Token cannot be used before nbf=%lld"), Token.NotBefore);
		return false;
	}

	if (Token.IssuedAt > 0 && Token.IssuedAt > (CurrentTime + TokenClockSkewSeconds))
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: Token iat=%lld is in the future beyond configured clock skew"), Token.IssuedAt);
		return false;
	}

	return true;
}

bool FTokenManager::ParseJWT(const FString& TokenString, TSharedPtr<FJsonObject>& OutHeader, TSharedPtr<FJsonObject>& OutPayload, TArray<uint8>& OutSignature, FString& OutSignedData)
{
	// JWT format: header.payload.signature
	TArray<FString> Parts;
	TokenString.ParseIntoArray(Parts, TEXT("."));

	if (Parts.Num() != 3)
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: Invalid JWT format, expected 3 parts, got %d"), Parts.Num());
		return false;
	}

	OutSignedData = FString::Printf(TEXT("%s.%s"), *Parts[0], *Parts[1]);

	// Decode header
	TArray<uint8> HeaderBytes = DecodeBase64Url(Parts[0]);
	FString HeaderJson = FString::FromBlob(HeaderBytes.GetData(), HeaderBytes.Num());
	TSharedRef<TJsonReader<>> HeaderReader = TJsonReaderFactory<>::Create(HeaderJson);
	if (!FJsonSerializer::Deserialize(HeaderReader, OutHeader) || !OutHeader.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: Failed to parse JWT header"));
		return false;
	}

	// Decode payload
	TArray<uint8> PayloadBytes = DecodeBase64Url(Parts[1]);
	FString PayloadJson = FString::FromBlob(PayloadBytes.GetData(), PayloadBytes.Num());
	TSharedRef<TJsonReader<>> PayloadReader = TJsonReaderFactory<>::Create(PayloadJson);
	if (!FJsonSerializer::Deserialize(PayloadReader, OutPayload) || !OutPayload.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: Failed to parse JWT payload"));
		return false;
	}

	// Decode signature
	OutSignature = DecodeBase64Url(Parts[2]);

	return true;
}

bool FTokenManager::VerifyRS256Signature(const FString& Data, const TArray<uint8>& Signature, const FJWK& Key)
{
#if PLATFORM_WINDOWS
	if (Key.KeyType != TEXT("RSA"))
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: Unsupported JWK key type for RS256 verification: %s"), *Key.KeyType);
		return false;
	}

	if (Key.ModulusBytes.IsEmpty() || Key.ExponentBytes.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: RSA key is missing modulus or exponent bytes"));
		return false;
	}

	FTCHARToUTF8 Utf8Data(*Data);
	TArray<uint8> DataBytes;
	DataBytes.Append(reinterpret_cast<const uint8*>(Utf8Data.Get()), Utf8Data.Length());

	TArray<uint8> DataHash;
	if (!ComputeSha256(DataBytes, DataHash))
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: Failed to compute SHA-256 for JWT signed data"));
		return false;
	}

	TArray<uint8> PublicKeyBlob;
	if (!BuildWindowsRsaPublicKeyBlob(Key, PublicKeyBlob))
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: Failed to build Windows RSA public key blob"));
		return false;
	}

	BCRYPT_ALG_HANDLE RsaAlgorithm = nullptr;
	BCRYPT_KEY_HANDLE PublicKeyHandle = nullptr;
	BCRYPT_PKCS1_PADDING_INFO PaddingInfo;
	PaddingInfo.pszAlgId = const_cast<wchar_t*>(BCRYPT_SHA256_ALGORITHM);

	const NTSTATUS OpenStatus = BCryptOpenAlgorithmProvider(&RsaAlgorithm, BCRYPT_RSA_ALGORITHM, nullptr, 0);
	if (OpenStatus < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: BCryptOpenAlgorithmProvider failed for RSA: 0x%08x"), OpenStatus);
		return false;
	}

	const NTSTATUS ImportStatus = BCryptImportKeyPair(
		RsaAlgorithm,
		nullptr,
		BCRYPT_RSAPUBLIC_BLOB,
		&PublicKeyHandle,
		PublicKeyBlob.GetData(),
		static_cast<ULONG>(PublicKeyBlob.Num()),
		0);

	if (ImportStatus < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: BCryptImportKeyPair failed: 0x%08x"), ImportStatus);
		BCryptCloseAlgorithmProvider(RsaAlgorithm, 0);
		return false;
	}

	const NTSTATUS VerifyStatus = BCryptVerifySignature(
		PublicKeyHandle,
		&PaddingInfo,
		const_cast<PUCHAR>(DataHash.GetData()),
		static_cast<ULONG>(DataHash.Num()),
		const_cast<PUCHAR>(Signature.GetData()),
		static_cast<ULONG>(Signature.Num()),
		BCRYPT_PAD_PKCS1);

	BCryptDestroyKey(PublicKeyHandle);
	BCryptCloseAlgorithmProvider(RsaAlgorithm, 0);

	if (VerifyStatus < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: BCryptVerifySignature failed: 0x%08x"), VerifyStatus);
		return false;
	}

	return true;
#else
	UE_LOG(LogTemp, Warning, TEXT("TokenManager: RS256 signature verification is only implemented for Windows builds in this plugin"));
	return false;
#endif
}

const FJWK* FTokenManager::FindKey(const FString& KeyId) const
{
	for (const FJWK& Key : Keys)
	{
		if (Key.KeyId == KeyId)
		{
			return &Key;
		}
	}
	return nullptr;
}

bool FTokenManager::ParseJWKS(const FString& JwksJson)
{
	TSharedPtr<FJsonObject> JwksObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JwksJson);

	if (!FJsonSerializer::Deserialize(Reader, JwksObject) || !JwksObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: Failed to parse JWKS JSON"));
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* KeysArray;
	if (!JwksObject->TryGetArrayField(TEXT("keys"), KeysArray))
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: JWKS missing 'keys' array"));
		return false;
	}

	Keys.Empty();

	for (const TSharedPtr<FJsonValue>& KeyValue : *KeysArray)
	{
		TSharedPtr<FJsonObject> KeyObject = KeyValue->AsObject();
		if (!KeyObject.IsValid())
		{
			continue;
		}

		FJWK NewKey;
		NewKey.KeyId = KeyObject->GetStringField(TEXT("kid"));
		NewKey.KeyType = KeyObject->GetStringField(TEXT("kty"));
		NewKey.Algorithm = KeyObject->GetStringField(TEXT("alg"));
		NewKey.Use = KeyObject->HasField(TEXT("use")) ? KeyObject->GetStringField(TEXT("use")) : FString();
		NewKey.Modulus = KeyObject->GetStringField(TEXT("n"));
		NewKey.Exponent = KeyObject->GetStringField(TEXT("e"));

		// Decode modulus and exponent
		NewKey.ModulusBytes = DecodeBase64Url(NewKey.Modulus);
		NewKey.ExponentBytes = DecodeBase64Url(NewKey.Exponent);

		Keys.Add(NewKey);
		UE_LOG(LogTemp, Log, TEXT("TokenManager: Loaded key - ID: %s, Type: %s, Algorithm: %s"),
			*NewKey.KeyId, *NewKey.KeyType, *NewKey.Algorithm);
	}

	return Keys.Num() > 0;
}

TArray<uint8> FTokenManager::DecodeBase64Url(const FString& Base64String) const
{
	// Base64URL uses '-' instead of '+' and '_' instead of '/'
	// Also, padding '=' characters may be omitted
	FString Base64 = Base64String;
	Base64 = Base64.Replace(TEXT("-"), TEXT("+"));
	Base64 = Base64.Replace(TEXT("_"), TEXT("/"));

	// Add padding if needed
	int32 Padding = (4 - (Base64.Len() % 4)) % 4;
	for (int32 i = 0; i < Padding; i++)
	{
		Base64.AppendChar('=');
	}

	TArray<uint8> DecodedData;
	FBase64::Decode(Base64, DecodedData);

	return DecodedData;
}

bool FTokenManager::ComputeSha256(const TArray<uint8>& Data, TArray<uint8>& OutHash) const
{
#if PLATFORM_WINDOWS
	BCRYPT_ALG_HANDLE HashAlgorithm = nullptr;
	BCRYPT_HASH_HANDLE HashHandle = nullptr;
	PUCHAR HashObjectBuffer = nullptr;
	DWORD HashObjectSize = 0;
	DWORD ResultSize = 0;
	DWORD HashSize = 0;

	const NTSTATUS OpenStatus = BCryptOpenAlgorithmProvider(&HashAlgorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
	if (OpenStatus < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: BCryptOpenAlgorithmProvider failed for SHA-256: 0x%08x"), OpenStatus);
		return false;
	}

	const NTSTATUS ObjectStatus = BCryptGetProperty(
		HashAlgorithm,
		BCRYPT_OBJECT_LENGTH,
		reinterpret_cast<PUCHAR>(&HashObjectSize),
		sizeof(HashObjectSize),
		&ResultSize,
		0);

	if (ObjectStatus < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: BCryptGetProperty(BCRYPT_OBJECT_LENGTH) failed: 0x%08x"), ObjectStatus);
		BCryptCloseAlgorithmProvider(HashAlgorithm, 0);
		return false;
	}

	const NTSTATUS SizeStatus = BCryptGetProperty(
		HashAlgorithm,
		BCRYPT_HASH_LENGTH,
		reinterpret_cast<PUCHAR>(&HashSize),
		sizeof(HashSize),
		&ResultSize,
		0);

	if (SizeStatus < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: BCryptGetProperty(BCRYPT_HASH_LENGTH) failed: 0x%08x"), SizeStatus);
		BCryptCloseAlgorithmProvider(HashAlgorithm, 0);
		return false;
	}

	HashObjectBuffer = static_cast<PUCHAR>(FMemory::Malloc(HashObjectSize));
	if (!HashObjectBuffer)
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: Failed to allocate SHA-256 hash object buffer"));
		BCryptCloseAlgorithmProvider(HashAlgorithm, 0);
		return false;
	}

	const NTSTATUS CreateStatus = BCryptCreateHash(HashAlgorithm, &HashHandle, HashObjectBuffer, HashObjectSize, nullptr, 0, 0);
	if (CreateStatus < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: BCryptCreateHash failed: 0x%08x"), CreateStatus);
		FMemory::Free(HashObjectBuffer);
		BCryptCloseAlgorithmProvider(HashAlgorithm, 0);
		return false;
	}

	const NTSTATUS HashStatus = BCryptHashData(
		HashHandle,
		const_cast<PUCHAR>(Data.GetData()),
		static_cast<ULONG>(Data.Num()),
		0);

	if (HashStatus < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: BCryptHashData failed: 0x%08x"), HashStatus);
		BCryptDestroyHash(HashHandle);
		FMemory::Free(HashObjectBuffer);
		BCryptCloseAlgorithmProvider(HashAlgorithm, 0);
		return false;
	}

	OutHash.SetNum(HashSize);
	const NTSTATUS FinishStatus = BCryptFinishHash(HashHandle, OutHash.GetData(), HashSize, 0);

	BCryptDestroyHash(HashHandle);
	FMemory::Free(HashObjectBuffer);
	BCryptCloseAlgorithmProvider(HashAlgorithm, 0);

	if (FinishStatus < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("TokenManager: BCryptFinishHash failed: 0x%08x"), FinishStatus);
		OutHash.Reset();
		return false;
	}

	return true;
#else
	OutHash.Reset();
	return false;
#endif
}

bool FTokenManager::BuildWindowsRsaPublicKeyBlob(const FJWK& Key, TArray<uint8>& OutBlob) const
{
#if PLATFORM_WINDOWS
	if (Key.ModulusBytes.IsEmpty() || Key.ExponentBytes.IsEmpty())
	{
		return false;
	}

	const uint32 BlobSize = sizeof(BCRYPT_RSAKEY_BLOB) + Key.ExponentBytes.Num() + Key.ModulusBytes.Num();
	OutBlob.SetNumZeroed(BlobSize);

	BCRYPT_RSAKEY_BLOB* BlobHeader = reinterpret_cast<BCRYPT_RSAKEY_BLOB*>(OutBlob.GetData());
	BlobHeader->Magic = BCRYPT_RSAPUBLIC_MAGIC;
	BlobHeader->BitLength = static_cast<ULONG>(Key.ModulusBytes.Num() * 8);
	BlobHeader->cbPublicExp = static_cast<ULONG>(Key.ExponentBytes.Num());
	BlobHeader->cbModulus = static_cast<ULONG>(Key.ModulusBytes.Num());
	BlobHeader->cbPrime1 = 0;
	BlobHeader->cbPrime2 = 0;

	uint8* BlobCursor = OutBlob.GetData() + sizeof(BCRYPT_RSAKEY_BLOB);
	FMemory::Memcpy(BlobCursor, Key.ExponentBytes.GetData(), Key.ExponentBytes.Num());
	BlobCursor += Key.ExponentBytes.Num();
	FMemory::Memcpy(BlobCursor, Key.ModulusBytes.GetData(), Key.ModulusBytes.Num());

	return true;
#else
	OutBlob.Reset();
	return false;
#endif
}
