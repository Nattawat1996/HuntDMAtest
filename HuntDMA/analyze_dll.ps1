# PowerShell script to analyze GameHunt.dll for health offsets
$dllPath = "C:\Users\Admin\OneDrive\Documents\GitHub\HuntDMA\HuntDMA\GameHunt.dll"

Write-Host "Analyzing $dllPath..." -ForegroundColor Cyan
$data = [System.IO.File]::ReadAllBytes($dllPath)
Write-Host "File size: $($data.Length) bytes" -ForegroundColor Green

# Known offsets to search for
$knownOffsets = @(
    0x58,   # HpOffset5 (confirmed)
    0x18,   # HealthBar current_max_hp (confirmed)
    0x10,   # HealthBar current_hp
    0x14,   # HealthBar regenerable_max_hp
    0x198,  # Old HpOffset1
    0x20,   # Old HpOffset2
    0xD0,   # Old HpOffset3
    0x78    # Old HpOffset4
)

Write-Host "`n=== Searching for offset patterns ===" -ForegroundColor Yellow

foreach ($offset in $knownOffsets) {
    # Search for 32-bit little-endian pattern
    $pattern = [System.BitConverter]::GetBytes([uint32]$offset)
    
    $count = 0
    $positions = @()
    for ($i = 0; $i -lt ($data.Length - 4); $i++) {
        if ($data[$i] -eq $pattern[0] -and 
            $data[$i+1] -eq $pattern[1] -and 
            $data[$i+2] -eq $pattern[2] -and 
            $data[$i+3] -eq $pattern[3]) {
            $count++
            if ($positions.Count -lt 10) {
                $positions += $i
            }
        }
    }
    
    if ($count -gt 0) {
        Write-Host "Offset 0x$($offset.ToString('X')) found $count times" -ForegroundColor Green
        foreach ($pos in $positions) {
            Write-Host "  - Position: 0x$($pos.ToString('X'))"
        }
    }
}

# Search for health-related strings
Write-Host "`n=== Searching for health-related strings ===" -ForegroundColor Yellow
$healthKeywords = @('health', 'Health', 'HEALTH', 'hp', 'HP', 'hitpoint', 'HitPoint', 'damage', 'Damage', 'HealthBar')

foreach ($keyword in $healthKeywords) {
    $keywordBytes = [System.Text.Encoding]::ASCII.GetBytes($keyword)
    $found = $false
    
    for ($i = 0; $i -lt ($data.Length - $keywordBytes.Length); $i++) {
        $match = $true
        for ($j = 0; $j -lt $keywordBytes.Length; $j++) {
            if ($data[$i + $j] -ne $keywordBytes[$j]) {
                $match = $false
                break
            }
        }
        
        if ($match) {
            if (-not $found) {
                Write-Host "Found '$keyword' at position 0x$($i.ToString('X'))" -ForegroundColor Green
                $found = $true
                
                # Show context (20 bytes before, 50 after)
                $contextStart = [Math]::Max(0, $i - 20)
                $contextEnd = [Math]::Min($data.Length, $i + 50)
                $context = $data[$contextStart..$contextEnd]
                $printable = -join ($context | ForEach-Object { if ($_ -ge 32 -and $_ -lt 127) { [char]$_ } else { '.' } })
                Write-Host "  Context: $printable"
                break
            }
        }
    }
}

Write-Host "`n=== Analysis complete ===" -ForegroundColor Cyan
