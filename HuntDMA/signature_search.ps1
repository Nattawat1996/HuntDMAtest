# Search for SystemGlobalEnvironment signature in GameHunt.dll
$dllPath = "C:\Users\Admin\OneDrive\Documents\GitHub\HuntDMA\HuntDMA\GameHunt.dll"

Write-Host "Searching for SystemGlobalEnvironment signature..." -ForegroundColor Cyan

# Signature: "48 8B 05 ? ? ? ? 48 8B 88 B0"
# This means: 48 8B 05 [any 4 bytes] 48 8B 88 B0
# In pattern: 48 8B 05 XX XX XX XX 48 8B 88 B0

$data = [System.IO.File]::ReadAllBytes($dllPath)
Write-Host "File size: $($data.Length) bytes" -ForegroundColor Green

# Search for the pattern with wildcards
# Pattern bytes (ignoring the wildcard positions 3,4,5,6)
$pattern = @(0x48, 0x8B, 0x05, -1, -1, -1, -1, 0x48, 0x8B, 0x88, 0xB0)

Write-Host "`nSearching for signature pattern..." -ForegroundColor Yellow
$matches = @()

for ($i = 0; $i -lt ($data.Length - 11); $i++) {
    $match = $true
    for ($j = 0; $j -lt $pattern.Length; $j++) {
        if ($pattern[$j] -ne -1) {
            # -1 means wildcard
            if ($data[$i + $j] -ne $pattern[$j]) {
                $match = $false
                break
            }
        }
    }
    
    if ($match) {
        $matches += $i
        
        # Extract the relative offset (the 4 wildcard bytes)
        $relativeOffset = [BitConverter]::ToInt32($data, $i + 3)
        $actualAddress = $i + 7 + $relativeOffset
        
        Write-Host "`nFound signature at: 0x$($i.ToString('X8'))" -ForegroundColor Green
        Write-Host "  Relative offset: 0x$($relativeOffset.ToString('X8'))" -ForegroundColor Cyan
        Write-Host "  Calculated address: 0x$($actualAddress.ToString('X8'))" -ForegroundColor Cyan
        
        # Show surrounding bytes
        $contextStart = [Math]::Max(0, $i - 32)
        $contextEnd = [Math]::Min($data.Length, $i + 64)
        $context = $data[$contextStart..$contextEnd]
        
        Write-Host "  Context (hex):"
        $hexString = ""
        for ($k = 0; $k -lt $context.Length; $k++) {
            if (($contextStart + $k) -eq $i) {
                $hexString += "["
            }
            $hexString += $context[$k].ToString("X2") + " "
            if (($contextStart + $k) -eq ($i + 10)) {
                $hexString += "] "
            }
        }
        Write-Host "    $hexString"
        
        if ($matches.Count -ge 10) {
            Write-Host "`n  (Showing first 10 matches only...)" -ForegroundColor Yellow
            break
        }
    }
}

Write-Host "`nTotal matches found: $($matches.Count)" -ForegroundColor Cyan

# Also search for common offset patterns near these locations
if ($matches.Count -gt 0) {
    Write-Host "`n=== Searching for offset patterns near signature ===" -ForegroundColor Yellow
    
    foreach ($matchPos in $matches[0..([Math]::Min(2, $matches.Count - 1))]) {
        $rangeStart = [Math]::Max(0, $matchPos - 512)
        $rangeEnd = [Math]::Min($data.Length, $matchPos + 512)
        $searchRange = $data[$rangeStart..$rangeEnd]
        
        # Look for health-related offset values
        $healthOffsets = @(0x58, 0x18, 0x10, 0x20, 0x78, 0xD0, 0x198)
        
        Write-Host "`nNear signature at 0x$($matchPos.ToString('X8')) (+/- 512 bytes):" -ForegroundColor Cyan
        
        foreach ($offset in $healthOffsets) {
            $offsetBytes = [BitConverter]::GetBytes([uint32]$offset)
            $count = 0
            for ($i = 0; $i -lt ($searchRange.Length - 4); $i++) {
                if ($searchRange[$i] -eq $offsetBytes[0] -and 
                    $searchRange[$i + 1] -eq $offsetBytes[1] -and 
                    $searchRange[$i + 2] -eq $offsetBytes[2] -and 
                    $searchRange[$i + 3] -eq $offsetBytes[3]) {
                    $count++
                }
            }
            if ($count -gt 0) {
                Write-Host "  Found 0x$($offset.ToString('X')) : $count times" -ForegroundColor Green
            }
        }
    }
}

Write-Host "`nDone!" -ForegroundColor Green
