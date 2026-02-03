# Search for potential new health offsets in GameHunt.dll
# Focus on finding sequences that include 0x58 (confirmed HpOffset5)

$dllPath = "C:\Users\Admin\OneDrive\Documents\GitHub\HuntDMA\HuntDMA\GameHunt.dll"
$data = [System.IO.File]::ReadAllBytes($dllPath)

Write-Host "Searching for offset sequences containing 0x58..." -ForegroundColor Cyan
Write-Host "File size: $($data.Length) bytes`n" -ForegroundColor Green

# Pattern: looking for 0x58 and checking what 4-byte values precede it
$pattern58 = @(0x58, 0x00, 0x00, 0x00)  # 0x58 as uint32 little-endian

$candidates = @()

Write-Host "Finding all occurrences of 0x58 (as 4-byte value)..." -ForegroundColor Yellow

for ($i = 0; $i -lt ($data.Length - 20); $i++) {
    # Check if we have 0x58 00 00 00
    if ($data[$i] -eq 0x58 -and $data[$i + 1] -eq 0x00 -and $data[$i + 2] -eq 0x00 -and $data[$i + 3] -eq 0x00) {
        
        # Check if there's another offset-like value before it (within 16 bytes)
        $hasPrecedingOffset = $false
        
        # Look backwards for potential offset values
        if ($i -ge 4) {
            $prevValue = [BitConverter]::ToUInt32($data, $i - 4)
            # Check if previous value looks like an offset (0x10-0x200 range)
            if ($prevValue -ge 0x10 -and $prevValue -le 0x200) {
                $hasPrecedingOffset = $true
                
                # Extract surrounding values
                $values = @()
                for ($j = -16; $j -le 16; $j += 4) {
                    if (($i + $j) -ge 0 -and ($i + $j + 3) -lt $data.Length) {
                        $val = [BitConverter]::ToUInt32($data, $i + $j)
                        $values += [PSCustomObject]@{
                            Offset       = $j
                            Value        = "0x$($val.ToString('X'))"
                            DecimalValue = $val
                        }
                    }
                }
                
                $candidates += [PSCustomObject]@{
                    Position          = "0x$($i.ToString('X8'))"
                    PreviousValue     = "0x$($prevValue.ToString('X'))"
                    SurroundingValues = $values
                }
            }
        }
    }
}

Write-Host "Found $($candidates.Count) potential offset sequences with 0x58`n" -ForegroundColor Cyan

# Show most promising candidates
Write-Host "=== Top 20 Candidates ===" -ForegroundColor Yellow

foreach ($candidate in $candidates[0..([Math]::Min(19, $candidates.Count - 1))]) {
    Write-Host "`nPosition: $($candidate.Position), Previous: $($candidate.PreviousValue)" -ForegroundColor Green
    
    # Show the sequence
    $sequence = ""
    foreach ($val in $candidate.SurroundingValues | Where-Object { $_.Offset -ge -12 -and $_.Offset -le 12 }) {
        if ($val.Offset -eq 0) {
            $sequence += "[$($val.Value)] "
        }
        else {
            $sequence += "$($val.Value) "
        }
    }
    Write-Host "  Sequence: $sequence"
    
    # Check if contains other known offset values
    $hasKnownOffsets = @()
    foreach ($val in $candidate.SurroundingValues) {
        if ($val.DecimalValue -in @(0x18, 0x20, 0x78, 0xD0, 0x198, 0x10, 0x14)) {
            $hasKnownOffsets += $val.Value
        }
    }
    if ($hasKnownOffsets.Count -gt 0) {
        Write-Host "  ** Contains known offsets: $($hasKnownOffsets -join ', ')" -ForegroundColor Magenta
    }
}

# Also search for sequences: XX XX 00 00 YY YY 00 00 ZZ ZZ 00 00 WW WW 00 00 58 00 00 00
Write-Host "`n`n=== Searching for 4-offset sequences ending with 0x58 ===" -ForegroundColor Yellow

$sequenceCandidates = @()

for ($i = 16; $i -lt ($data.Length - 20); $i++) {
    if ($data[$i] -eq 0x58 -and $data[$i + 1] -eq 0x00 -and $data[$i + 2] -eq 0x00 -and $data[$i + 3] -eq 0x00) {
        
        # Extract 5 preceding uint32 values
        $offset1 = [BitConverter]::ToUInt32($data, $i - 16)
        $offset2 = [BitConverter]::ToUInt32($data, $i - 12)
        $offset3 = [BitConverter]::ToUInt32($data, $i - 8)
        $offset4 = [BitConverter]::ToUInt32($data, $i - 4)
        
        # Check if all values look like reasonable offsets (0x10-0x300 range)
        if ($offset1 -ge 0x10 -and $offset1 -le 0x300 -and
            $offset2 -ge 0x10 -and $offset2 -le 0x300 -and
            $offset3 -ge 0x10 -and $offset3 -le 0x300 -and
            $offset4 -ge 0x10 -and $offset4 -le 0x300) {
            
            $sequenceCandidates += [PSCustomObject]@{
                Position  = "0x$($i.ToString('X8'))"
                HpOffset1 = "0x$($offset1.ToString('X'))"
                HpOffset2 = "0x$($offset2.ToString('X'))"
                HpOffset3 = "0x$($offset3.ToString('X'))"
                HpOffset4 = "0x$($offset4.ToString('X'))"
                HpOffset5 = "0x58"
            }
        }
    }
}

Write-Host "`nFound $($sequenceCandidates.Count) potential 5-offset sequences" -ForegroundColor Cyan

if ($sequenceCandidates.Count -gt 0) {
    Write-Host "`n=== Potential Health Offset Chains ===" -ForegroundColor Green
    foreach ($seq in $sequenceCandidates[0..([Math]::Min(10, $sequenceCandidates.Count - 1))]) {
        Write-Host "`nPosition: $($seq.Position)"
        Write-Host "  HpOffset1: $($seq.HpOffset1)"
        Write-Host "  HpOffset2: $($seq.HpOffset2)"
        Write-Host "  HpOffset3: $($seq.HpOffset3)"
        Write-Host "  HpOffset4: $($seq.HpOffset4)"
        Write-Host "  HpOffset5: $($seq.HpOffset5) (confirmed)"
    }
}

Write-Host "`nDone!" -ForegroundColor Green
