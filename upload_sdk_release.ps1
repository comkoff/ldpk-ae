# One-off helper: replace the SDK zip asset in the AE-SDK-26.5-mac release.
# The token is taken from the local git credential store and is NEVER printed.
$ErrorActionPreference = "Stop"
$env:GIT_TERMINAL_PROMPT = "0"
$cred = "protocol=https`nhost=github.com`n`n" | git credential fill 2>$null
$tok = ($cred | Select-String '^password=(.+)$').Matches[0].Groups[1].Value
$headers = @{
    Authorization = "Bearer $tok"
    Accept        = "application/vnd.github+json"
}

$rel = Invoke-RestMethod -Uri "https://api.github.com/repos/comkoff/ldpk-ae/releases/tags/AE-SDK-26.5-mac" -Headers $headers
$old = $rel.assets | Where-Object { $_.name -eq "AfterEffectsSDK_26.5_mac.zip" }
if ($old) {
    Invoke-RestMethod -Method Delete -Uri "https://api.github.com/repos/comkoff/ldpk-ae/releases/assets/$($old.id)" -Headers $headers
    Write-Output "OLD ASSET DELETED (id=$($old.id))"
}

$uploadUri = ($rel.upload_url -replace '\{\?.*$', '') + "?name=AfterEffectsSDK_26.5_mac.zip"
$asset = Invoke-RestMethod -Method Post -Uri $uploadUri -Headers $headers -ContentType "application/zip" -InFile "AfterEffectsSDK_26.5_mac.zip"
Write-Output "ASSET:   $($asset.name) ($($asset.size) bytes, state=$($asset.state))"
