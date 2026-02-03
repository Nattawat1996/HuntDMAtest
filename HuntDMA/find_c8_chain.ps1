# Search for offset chains containing 0xC8 (corrected old HpOffset3)
$dllPath = "C:\Users\Admin\OneDrive\Documents\GitHub\HuntDMA\HuntDMA\GameHunt.dll"
$data = [System.IO.File]::ReadAllBytes($dllPath)

Write-Host "Searching for offset chains with 0xC8..." -ForegroundColor Cyan
Write-Host "Old path: 0x198 -> 0x20 -> 0xC8 -> 0x78 -> 0x58 -> 0x18`n" -ForegroundColor Yellow

# Search for sequences containing old offset values
Write-Host "=== Looking for chains with old offset values ===" -ForegroundColor Yellow

$fullMatches = @()

for ($i = 20; $i -lt ($data.Length - 20); $i++) {
    if ($data[$i] -eq 0x58 -and $data[$i + 1] -eq 0x00 -and $data[$i + 2] -eq 0x00 -and $data[$i + 3] -eq 0x00) {
        
        # Check if we have 0x20, 0xC8, 0x78 in the preceding 16 bytes
        $has20 = $false
        $hasC8 = $false
        $has78 = $false
        
        for ($j = 1; $j -le 4; $j++) {
            $checkPos = $i - ($j * 4)
            if ($checkPos -ge 0) {
                $val = [BitConverter]::ToUInt32($data, $checkPos)
                if ($val -eq 0x20) { $has20 = $true }
                if ($val -eq 0xC8) { $hasC8 = $true }
                if ($val -eq 0x78) { $has78 = $true }
            }
        }
        
        if ($has20 -or $hasC8 -or $has78) {
            # Extract 5-offset chain
            $o1 = if ($i -ge 16) { [BitConverter]::ToUInt32($data, $i - 16) } else { 0 }
            $o2 = if ($i -ge 12) { [BitConverter]::ToUInt32($data, $i - 12) } else { 0 }
            $o3 = if ($i -ge 8) { [BitConverter]::ToUInt32($data, $i - 8) } else { 0 }
            $o4 = if ($i -ge 4) { [BitConverter]::ToUInt32($data, $i - 4) } else { 0 }
            
            $matchCount = 0
            if ($o1 -in @(0x198, 0x20, 0xC8, 0x78)) { $matchCount++ }
            if ($o2 -in @(0x198, 0x20, 0xC8, 0x78)) { $matchCount++ }
            if ($o3 -in @(0x198, 0x20, 0xC8, 0x78)) { $matchCount++ }
            if ($o4 -in @(0x198, 0x20, 0xC8, 0x78)) { $matchCount++ }
            
            if ($matchCount -ge 2) {
                $fullMatches += [PSCustomObject]@{
                    Position   = "0x$($i.ToString('X8'))"
                    HpOffset1  = "0x$($o1.ToString('X'))"
                    HpOffset2  = "0x$($o2.ToString('X'))"
                    HpOffset3  = "0x$($o3.ToString('X'))"
                    HpOffset4  = "0x$($o4.ToString('X'))"
                    HpOffset5  = "0x58"
                    Has20      = $has20
                    HasC8      = $hasC8
                    Has78      = $has78
                    MatchCount = $matchCount
                }
            }
        }
    }
}

Write-Host "`nFound $($fullMatches.Count) sequences with 2+ old offset values`n" -ForegroundColor Green

foreach ($match in $fullMatches | Sort-Object -Property MatchCount -Descending | Select-Object -First 20) {
    $markers = ""
    if ($match.Has20) { $markers += "[0x20] " }
    if ($match.HasC8) { $markers += "[0xC8] " }
    if ($match.Has78) { $markers += "[0x78] " }
    
    Write-Host "`nPosition: $($match.Position) - Matches: $($match.MatchCount) - Contains: $markers" -ForegroundColor Cyan
    Write-Host "  HpOffset1: $($match.HpOffset1)"
    Write-Host "  HpOffset2: $($match.HpOffset2)"
    Write-Host "  HpOffset3: $($match.HpOffset3)"
    Write-Host "  HpOffset4: $($match.HpOffset4)"
    Write-Host "  HpOffset5: $($match.HpOffset5) [CONFIRMED]"
}

Write-Host "`nDone!" -ForegroundColor Green
