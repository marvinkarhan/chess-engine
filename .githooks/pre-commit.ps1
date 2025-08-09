#!/usr/bin/env pwsh
$ErrorActionPreference = 'Stop'

# Build (override with $env:ENGINE_BUILD_CMD if desired)
$buildCmd = if ([string]::IsNullOrWhiteSpace($env:ENGINE_BUILD_CMD)) { 'make -C src -j' } else { $env:ENGINE_BUILD_CMD }
Write-Host "[pre-commit] Building engine..."
pwsh -NoProfile -Command $buildCmd | Out-Host

# Pick engine binary
$bin = @('./src/uci-engine.exe','./src/uci-engine') | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $bin) { $bin = './src/uci-engine.exe' }
$binFull = (Resolve-Path $bin).Path
$binDir  = Split-Path -Parent $binFull

# Run UCI bench (stdin) first
Write-Host "[pre-commit] Running bench..."
$gitDir = (& git rev-parse --git-dir).Trim()
$lastOut = Join-Path $gitDir 'last_bench_output.txt'
$benchOut = ''

try {
  $pinfo = New-Object System.Diagnostics.ProcessStartInfo
  $pinfo.FileName = $binFull
  $pinfo.WorkingDirectory = $binDir
  $pinfo.RedirectStandardInput = $true
  $pinfo.RedirectStandardOutput = $true
  $pinfo.RedirectStandardError = $true
  $pinfo.UseShellExecute = $false
  $p = New-Object System.Diagnostics.Process
  $p.StartInfo = $pinfo
  $p.Start() | Out-Null
  $p.StandardInput.Write("uci`n" + "isready`n" + "bench`n" + "quit`n")
  $p.StandardInput.Close()
  $benchOut = $p.StandardOutput.ReadToEnd() + "`n" + $p.StandardError.ReadToEnd()
  $p.WaitForExit()
} catch { }

# Fallback: run with CLI arg 'bench'
if ([string]::IsNullOrWhiteSpace($benchOut)) {
  try {
    $p2 = New-Object System.Diagnostics.Process
    $p2.StartInfo = New-Object System.Diagnostics.ProcessStartInfo
    $p2.StartInfo.FileName = $binFull
    $p2.StartInfo.WorkingDirectory = $binDir
    $p2.StartInfo.RedirectStandardOutput = $true
    $p2.StartInfo.RedirectStandardError = $true
    $p2.StartInfo.UseShellExecute = $false
    $p2.StartInfo.Arguments = 'bench'
    $p2.Start() | Out-Null
    $benchOut = $p2.StandardOutput.ReadToEnd() + "`n" + $p2.StandardError.ReadToEnd()
    $p2.WaitForExit()
  } catch { }
}

Set-Content -Path $lastOut -Value $benchOut -NoNewline

# Parse value
$benchVal = ''
# Prefer the number before 'nps' (e.g., '... 12345 nps')
$matchPreNps = [regex]::Matches($benchOut, '(?im)([0-9,]+)\s*nps\b')
if ($matchPreNps.Count -gt 0) {
  $benchVal = ($matchPreNps[$matchPreNps.Count-1].Groups[1].Value -replace ',', '')
}
# Alternative: 'nps 12345' (from info lines)
if ([string]::IsNullOrWhiteSpace($benchVal)) {
  $matchPostNps = [regex]::Matches($benchOut, '(?im)\bnps\b[^0-9]*([0-9,]+)')
  if ($matchPostNps.Count -gt 0) {
    $benchVal = ($matchPostNps[$matchPostNps.Count-1].Groups[1].Value -replace ',', '')
  }
}
# Fallback: number before 'nodes'
if ([string]::IsNullOrWhiteSpace($benchVal)) {
  $matchNodes = [regex]::Matches($benchOut, '(?im)([0-9,]+)\s*nodes\b')
  if ($matchNodes.Count -gt 0) {
    $benchVal = ($matchNodes[$matchNodes.Count-1].Groups[1].Value -replace ',', '')
  }
}

if ([string]::IsNullOrWhiteSpace($benchVal)) {
  Write-Warning "[pre-commit] Could not parse bench value. See $lastOut"
  exit 0
}

Set-Content -Path (Join-Path $gitDir 'bench_value') -Value $benchVal -NoNewline
Write-Host "[pre-commit] BENCH: $benchVal"
exit 0
