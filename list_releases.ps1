# One-off helper: list GitHub releases of comkoff/ldpk-ae.
# The token is taken from the local git credential store and is NEVER printed.
$ErrorActionPreference = "Stop"
$env:GIT_TERMINAL_PROMPT = "0"
$cred = "protocol=https`nhost=github.com`n`n" | git credential fill 2>$null
$tok = ($cred | Select-String '^password=(.+)$').Matches[0].Groups[1].Value
$headers = @{ Authorization = "Bearer $tok"; Accept = "application/vnd.github+json" }

$rels = Invoke-RestMethod -Uri "https://api.github.com/repos/comkoff/ldpk-ae/releases?per_page=10" -Headers $headers
if (-not $rels) { Write-Output "NO RELEASES"; exit 0 }
foreach ($r in $rels) {
    Write-Output ("RELEASE: id={0} tag={1} name='{2}' draft={3} prerelease={4}" -f $r.id, $r.tag_name, $r.name, $r.draft, $r.prerelease)
    Write-Output ("  url: {0}" -f $r.html_url)
    foreach ($a in $r.assets) {
        Write-Output ("  asset: {0} ({1} bytes, downloads={2})" -f $a.name, $a.size, $a.download_count)
    }
    Write-Output "--- body ---"
    Write-Output $r.body
    Write-Output "--- end body ---"
}
