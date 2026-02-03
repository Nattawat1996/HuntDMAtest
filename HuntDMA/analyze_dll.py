#!/usr/bin/env python3
"""
Script to analyze GameHunt.dll for health offset patterns
"""

import struct
import re
from pathlib import Path

dll_path = r"C:\Users\Admin\OneDrive\Documents\GitHub\HuntDMA\HuntDMA\GameHunt.dll"

print(f"Analyzing {dll_path}...")

# Read the DLL file
with open(dll_path, 'rb') as f:
    data = f.read()

print(f"File size: {len(data)} bytes")

# Search for known offset patterns
known_offsets = [
    0x58,   # HpOffset5 (confirmed correct)
    0x18,   # HealthBar current_max_hp (confirmed correct)
    0x10,   # HealthBar current_hp
    0x14,   # HealthBar regenerable_max_hp
    0x198,  # Old HpOffset1
    0x20,   # Old HpOffset2 (might still be correct)
    0xD0,   # Old HpOffset3
    0x78,   # Old HpOffset4
]

print("\n=== Searching for offset patterns ===")

# Search for sequences that might represent offset definitions
# Looking for little-endian uint64 representations
for offset in known_offsets:
    # Search as 32-bit value (more likely)
    pattern_32 = struct.pack('<I', offset)
    count = data.count(pattern_32)
    if count > 0:
        print(f"Offset 0x{offset:X} (32-bit) found {count} times")
        
        # Find first few occurrences
        pos = 0
        for i in range(min(5, count)):
            pos = data.find(pattern_32, pos)
            if pos != -1:
                print(f"  - Position: 0x{pos:X}")
                pos += 1

# Search for strings that might indicate health
print("\n=== Searching for health-related strings ===")
health_keywords = [
    b'health',
    b'Health',
    b'HEALTH',
    b'hp',
    b'HP',
    b'hitpoint',
    b'HitPoint',
    b'damage',
    b'Damage',
]

for keyword in health_keywords:
    pos = data.find(keyword)
    if pos != -1:
        print(f"Found '{keyword.decode()}' at position 0x{pos:X}")
        # Show context
        context_start = max(0, pos - 20)
        context_end = min(len(data), pos + 50)
        context = data[context_start:context_end]
        # Filter printable characters
        printable = ''.join(chr(b) if 32 <= b < 127 else '.' for b in context)
        print(f"  Context: {printable}")

# Search for potential offset sequences (looking for pointer chains)
print("\n=== Searching for potential offset chains ===")
# Look for patterns like: [offset1][offset2][offset3][offset4] near 0x58
pattern_58 = struct.pack('<I', 0x58)
pos = 0
matches = []
while True:
    pos = data.find(pattern_58, pos)
    if pos == -1:
        break
    
    # Check if there are other potential offsets nearby (within 64 bytes)
    nearby_range = data[max(0, pos-64):min(len(data), pos+64)]
    
    # Look for other common offset patterns
    if any(struct.pack('<I', off) in nearby_range for off in [0x10, 0x18, 0x20, 0x78, 0xD0]):
        matches.append(pos)
        if len(matches) <= 10:  # Only show first 10
            print(f"Potential offset chain near 0x{pos:X}")
    
    pos += 1

print(f"\nFound {len(matches)} potential offset chain locations")

# Look for exported functions or symbols
print("\n=== Checking for PE structure info ===")
# Simple PE header check
if data[:2] == b'MZ':
    print("Valid PE file (MZ header found)")
    # Read PE offset
    pe_offset = struct.unpack('<I', data[0x3C:0x40])[0]
    if pe_offset < len(data) - 4 and data[pe_offset:pe_offset+2] == b'PE':
        print(f"PE header at offset 0x{pe_offset:X}")
    else:
        print("PE header not found at expected location")

print("\n=== Analysis complete ===")
