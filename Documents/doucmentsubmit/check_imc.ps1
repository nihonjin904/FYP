$path = "c:\Users\Kelvin Lam\Documents\GitHub\FYP\FYP\Content\ThirdPerson\Input\IMC_Default.uasset"
$bytes = [IO.File]::ReadAllBytes($path)
$raw = [System.Text.Encoding]::ASCII.GetString($bytes)
$found = [regex]::Matches($raw, '[A-Za-z_][A-Za-z0-9_./]{2,}')
$words = @()
foreach ($m in $found) { $words += $m.Value }
Write-Output "=== IMC_Default ALL readable strings ==="
$words | Sort-Object -Unique
