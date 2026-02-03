# Quick hex search for specific offset patterns
$dllPath = "C:\Users\Admin\OneDrive\Documents\GitHub\HuntDMA\HuntDMA\GameHunt.dll"

Write-Host "Quick offset search in GameHunt.dll" -ForegroundColor Cyan

# Search for specific interesting offset combinations
# These are likely to be near each other in structure definitions

$offsets = @{
    'HpOffset5'           = 0x58
    'HealthBar_CurrentHP' = 0x10
    'HealthBar_MaxHP'     = 0x18
    'Old_HpOffset2'       = 0x20
    'Old_HpOffset4'       = 0x78
    'Old_HpOffset3'       = 0xD0
    'Old_HpOffset1'       = 0x198
}

# Read file in chunks to avoid memory issues
$chunkSize = 1MB
$stream = [System.IO.File]::OpenRead($dllPath)
$buffer = New-Object byte[] $chunkSize
$chunkNumber = 0

Write-Host "Searching for offset 0x198 (old HpOffset1)..." -ForegroundColor Yellow
$pattern198 = [byte[]]@(0x98, 0x01, 0x00, 0x00)  # 0x198 in little-endian
$found198 = @()

while (($bytesRead = $stream.Read($buffer, 0, $chunkSize)) -gt 0) {
    for ($i = 0; $i -lt ($bytesRead - 4); $i++) {
        if ($buffer[$i] -eq 0x98 -and $buffer[$i + 1] -eq 0x01 -and $buffer[$i + 2] -eq 0x00 -and $buffer[$i + 3] -eq 0x00) {
            $position = ($chunkNumber * $chunkSize) + $i
            $found198 += $position
            if ($found198.Count -le 20) {
                Write-Host "  Found at 0x$($position.ToString('X8'))" -ForegroundColor Green
            }
        }
    }
    $chunkNumber++
}

$stream.Close()
Write-Host "Total occurrences of 0x198: $($found198.Count)" -ForegroundColor Cyan

Write-Host "`nDone!" -ForegroundColor Green
