# One-off helper: create the AE-SDK-26.5-mac release in the PRIVATE comkoff/ae-sdk
# repository and upload the SDK subset zip as an asset.
# The token is taken from the local git credential store and is NEVER printed.
$ErrorActionPreference = "Stop"
$env:GIT_TERMINAL_PROMPT = "0"
$cred = "protocol=https`nhost=github.com`n`n" | git credential fill 2>$null
$tok = ($cred | Select-String '^password=(.+)$').Matches[0].Groups[1].Value
$headers = @{ Authorization = "Bearer $tok"; Accept = "application/vnd.github+json" }

$body = @{
    tag_name = "AE-SDK-26.5-mac"
    name     = "AE SDK 26.5 (macOS) - build-only subset"
    body     = "Build-only subset of the After Effects SDK 26.5 (macOS): Examples/Headers, Examples/Resources, Examples/Util. Consumed by the build-mac / release-mac workflows of comkoff/ldpk-ae via the SDK_TOKEN secret. PRIVATE by the SDK license - do not make public."
} | ConvertTo-Json

$rel = Invoke-RestMethod -Method Post -Uri "https://api.github.com/repos/comkoff/ae-sdk/releases" -Headers $headers -ContentType "application/json" -Body $body
$uploadUri = ($rel.upload_url -replace '\{\?.*$', '') + "?name=AfterEffectsSDK_26.5_mac.zip"
$asset = Invoke-RestMethod -Method Post -Uri $uploadUri -Headers $headers -ContentType "application/zip" -InFile "AfterEffectsSDK_26.5_mac.zip"
Write-Output ("RELEASE: {0}" -f $rel.html_url)
Write-Output ("ASSET:   {0} ({1} bytes, state={2})" -f $asset.name, $asset.size, $asset.state)
