# One-off helper: download the mac artifact of run #39688786618 and inspect
# both bundles (Info.plist + strings from the PiPL .rsrc files).
# The token is taken from the local git credential store and is NEVER printed.
$ErrorActionPreference = "Stop"
$env:GIT_TERMINAL_PROMPT = "0"
$cred = "protocol=https`nhost=github.com`n`n" | git credential fill 2>$null
$tok = ($cred | Select-String '^password=(.+)$').Matches[0].Groups[1].Value
$headers = @{ Authorization = "Bearer $tok"; Accept = "application/vnd.github+json" }

$arts = Invoke-RestMethod -Uri "https://api.github.com/repos/comkoff/ldpk-ae/actions/runs/39688786618/artifacts" -Headers $headers
$art = $arts.artifacts | Select-Object -First 1
Write-Output ("ARTIFACT: {0} ({1} bytes)" -f $art.name, $art.size_in_bytes)

$zip = "$env:TEMP\ld_mac_artifact.zip"
Invoke-WebRequest -Uri $art.archive_download_url -Headers $headers -OutFile $zip
$dest = "$env:TEMP\ld_mac_artifact"
if (Test-Path $dest) { Remove-Item -Recurse -Force $dest }
Expand-Archive -Path $zip -DestinationPath $dest

Write-Output "=== BUNDLE TREES ==="
Get-ChildItem -Recurse -File $dest | ForEach-Object {
    Write-Output ("{0,10}  {1}" -f $_.Length, $_.FullName.Substring($dest.Length + 1))
}

function Get-Strings([string]$path, [int]$min = 4) {
    $bytes = [IO.File]::ReadAllBytes($path)
    $sb = New-Object System.Text.StringBuilder
    $out = New-Object System.Collections.Generic.List[string]
    foreach ($b in $bytes) {
        if ($b -ge 32 -and $b -le 126) { [void]$sb.Append([char]$b) }
        else { if ($sb.Length -ge $min) { $out.Add($sb.ToString()) }; [void]$sb.Clear() }
    }
    if ($sb.Length -ge $min) { $out.Add($sb.ToString()) }
    return $out
}

Write-Output "=== INFO.PLIST FILES ==="
Get-ChildItem -Recurse -Filter "Info.plist" $dest | ForEach-Object {
    Write-Output ("--- {0}" -f $_.FullName.Substring($dest.Length + 1))
    Get-Content $_.FullName | ForEach-Object { Write-Output $_ }
}

Write-Output "=== RSRC STRINGS (filtered) ==="
Get-ChildItem -Recurse -Filter "*.rsrc" $dest | ForEach-Object {
    Write-Output ("--- {0} ({1} bytes)" -f $_.FullName.Substring($dest.Length + 1), $_.Length)
    Get-Strings $_.FullName 4 | Where-Object { $_ -match "3DE4|SDV|EffectMain|Lens|PiPL|BNDL|com\.|eFKT" } |
        ForEach-Object { Write-Output $_ }
}
