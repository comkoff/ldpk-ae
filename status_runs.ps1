# One-off helper: print the compile errors from the failed build step.
# The token is taken from the local git credential store and is NEVER printed.
$ErrorActionPreference = "Stop"
$env:GIT_TERMINAL_PROMPT = "0"
$cred = "protocol=https`nhost=github.com`n`n" | git credential fill 2>$null
$tok = ($cred | Select-String '^password=(.+)$').Matches[0].Groups[1].Value
$headers = @{ Authorization = "Bearer $tok"; Accept = "application/vnd.github+json" }

$jobs = Invoke-RestMethod -Uri "https://api.github.com/repos/comkoff/ldpk-ae/actions/runs/39686401196/jobs" -Headers $headers
$job = $jobs.jobs[0]
$logs = Invoke-RestMethod -Uri "https://api.github.com/repos/comkoff/ldpk-ae/actions/jobs/$($job.id)/logs" -Headers $headers
$lines = $logs -split "`n"
Write-Output ("LOG LINES: {0}" -f $lines.Count)
$hits = $lines | Select-String -Pattern "error:|Error |warning:|undefined|clang:|Rez |ld: |BUILD|ERROR"
$hits | Select-Object -First 60 | ForEach-Object { Write-Output $_.Line }
