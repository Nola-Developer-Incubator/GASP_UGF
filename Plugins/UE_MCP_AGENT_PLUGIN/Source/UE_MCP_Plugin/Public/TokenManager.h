// Copyright © 2026 Nola Development Incubator. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

/**
 * Represents a JWT token with parsed claims
 */
struct FJWTToken
{
	FString RawToken;
	FString Subject;
	FString Issuer;
	TArray<FString> Audiences;
	int64 ExpirationTime;
	int64 IssuedAt;
	int64 NotBefore;
	TArray<FString> Capabilities;
	FString PolicyVersion;
	bool bIsValid;

	FJWTToken()
		: ExpirationTime(0)
		, IssuedAt(0)
		, NotBefore(0)
		, bIsValid(false)
	{}
};

/**
 * Represents a JSON Web Key (JWK)
 */
struct FJWK
{
	FString KeyId;
	FString KeyType;
	FString Algorithm;
	FString Use;
	FString Modulus;
	FString Exponent;
	TArray<uint8> ModulusBytes;
	TArray<uint8> ExponentBytes;

	FJWK()
	{}
};

/**
 * TokenManager handles JWT validation with JWKS support
 */
class UE_MCP_PLUGIN_API FTokenManager
{
public:
	FTokenManager();
	~FTokenManager();

	/**
	 * Initialize token manager with JWKS URL
	 * @param JwksURL - URL to fetch JSON Web Key Set
	 * @return true if initialization successful
	 */
	bool Initialize(
		const FString& JwksURL,
		bool bInAllowInsecureTokenValidation = false,
		const FString& InExpectedAudience = FString(),
		const TArray<FString>& InAllowedIssuers = TArray<FString>(),
		int32 InTokenClockSkewSeconds = 60);

	/**
	 * Fetch JWKS from configured URL
	 * @return true if JWKS fetched successfully
	 */
	bool FetchJWKS();

	/**
	 * Validate a JWT token
	 * @param TokenString - JWT token string
	 * @param OutToken - Parsed token if valid
	 * @return true if token is valid
	 */
	bool ValidateToken(const FString& TokenString, FJWTToken& OutToken);

	/**
	 * Check if token has expired
	 * @param Token - Token to check
	 * @return true if token is expired
	 */
	bool IsTokenExpired(const FJWTToken& Token) const;

	/**
	 * Extract capabilities from token
	 * @param Token - Token to extract from
	 * @return Array of capability strings
	 */
	TArray<FString> GetCapabilities(const FJWTToken& Token) const;

	/**
	 * Get policy version from token
	 * @param Token - Token to extract from
	 * @return Policy version string
	 */
	FString GetPolicyVersion(const FJWTToken& Token) const;

	/**
	 * Check if JWKS needs rotation (based on age)
	 * @return true if JWKS should be refreshed
	 */
	bool ShouldRotateKeys() const;

	/**
	 * Force refresh of JWKS
	 */
	void RefreshKeys();

private:
	/**
	 * Parse JWT token
	 * @param TokenString - JWT token string
	 * @param OutHeader - Parsed header
	 * @param OutPayload - Parsed payload
	 * @param OutSignature - Signature bytes
	 * @return true if parsed successfully
	 */
	bool ParseJWT(const FString& TokenString, TSharedPtr<FJsonObject>& OutHeader, TSharedPtr<FJsonObject>& OutPayload, TArray<uint8>& OutSignature, FString& OutSignedData);

	/**
	 * Verify RS256 signature
	 * @param Data - Data that was signed
	 * @param Signature - Signature to verify
	 * @param Key - JWK to use for verification
	 * @return true if signature is valid
	 */
	bool VerifyRS256Signature(const FString& Data, const TArray<uint8>& Signature, const FJWK& Key);

	/**
	 * Find JWK by key ID
	 * @param KeyId - Key ID to search for
	 * @return Pointer to JWK if found, nullptr otherwise
	 */
	const FJWK* FindKey(const FString& KeyId) const;

	/**
	 * Parse JWKS response
	 * @param JwksJson - JSON string containing JWKS
	 * @return true if parsed successfully
	 */
	bool ParseJWKS(const FString& JwksJson);

	/**
	 * Decode Base64URL
	 * @param Base64String - Base64URL encoded string
	 * @return Decoded bytes
	 */
	TArray<uint8> DecodeBase64Url(const FString& Base64String) const;

	/**
	 * Validate configured issuer, audience, and time-based claims.
	 * @param Token - Token to validate
	 * @return true if configured claims are satisfied
	 */
	bool ValidateConfiguredClaims(const FJWTToken& Token) const;

	/**
	 * Compute a SHA-256 digest for the provided UTF-8 payload.
	 * @param Data - Input bytes
	 * @param OutHash - Output digest
	 * @return true if hashing succeeded
	 */
	bool ComputeSha256(const TArray<uint8>& Data, TArray<uint8>& OutHash) const;

	/**
	 * Build a Windows CNG RSA public-key blob from a JWK.
	 * @param Key - RSA JWK
	 * @param OutBlob - Output blob
	 * @return true if blob creation succeeded
	 */
	bool BuildWindowsRsaPublicKeyBlob(const FJWK& Key, TArray<uint8>& OutBlob) const;

	// JWKS URL
	FString JwksURL;

	// Stored keys
	TArray<FJWK> Keys;

	// Last fetch time for key rotation
	FDateTime LastKeyFetch;

	// Key rotation interval (in hours)
	float KeyRotationInterval;

	// Explicit development-only override for local environments where signature verification is unavailable.
	bool bAllowInsecureTokenValidation;

	// Optional audience required when validating bearer tokens.
	FString ExpectedAudience;

	// Allowed issuers when configured.
	TSet<FString> AllowedIssuers;

	// Allowed clock skew for nbf/iat/exp validation.
	int32 TokenClockSkewSeconds;
};
