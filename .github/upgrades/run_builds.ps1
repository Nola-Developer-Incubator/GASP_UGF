$engine = Join-Path $env:ProgramFiles 'Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat'
$targets = @('IoStoreOnDemandTests','EventLoopUnitTests','LiveLinkHub','DotNetPerforceLib')
foreach ($t in $targets) {
    $out = Join-Path $env:USERPROFILE "$t-build.log"
    Write-Output "Running $t... (output -> $out)"
    & $engine $t 'Win64' 'Development' '-FromMsBuild' '-WaitMutex' '-architecture=x64' '-Mode=Test' *> $out 2>&1
}
# GASPEditor with -Project
$t = 'GASPEditor'
$out = Join-Path $env:USERPROFILE "$t-build.log"
Write-Output "Running $t... (output -> $out)"
& $engine $t 'Win64' 'Development' "-Project=C:\\Unreal_Projects\\GASP\\GASP.uproject" '-FromMsBuild' '-WaitMutex' '-architecture=x64' *> $out 2>&1
Write-Output 'Done.'
