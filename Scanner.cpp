#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>

// Signature: 48 8B 05 ? ? ? ? 48 8B 88 B0
// Mask:      x  x  x  ? ? ? ? x  x  x  x

bool Compare(const uint8_t* pData, const char* bMask, const char* szMask)
{
    for (; *szMask; ++szMask, ++pData, ++bMask)
        if (*szMask == 'x' && *pData != *bMask)
            return false;
    return (*szMask) == 0;
}

uint64_t FindPattern(const std::vector<uint8_t>& data, const char* bMask, const char* szMask)
{
    size_t len = data.size();
    size_t maskLen = strlen(szMask);
    for (size_t i = 0; i < len - maskLen; i++)
    {
        if (Compare(&data[i], bMask, szMask))
            return i;
    }
    return 0;
}

int main()
{
    const char* filename = "example/GameHunt.dll";
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    
    if (!file.is_open())
    {
        std::cerr << "Failed to open " << filename << std::endl;
        return 1;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read((char*)buffer.data(), size))
    {
        std::cerr << "Failed to read file" << std::endl;
        return 1;
    }

    std::cout << "File loaded. Size: " << size << " bytes." << std::endl;

    // Pattern: 48 8B 05 ? ? ? ? 48 8B 88 B0
    // Bytes:   \x48\x8B\x05\x00\x00\x00\x00\x48\x8B\x88\xB0
    // Mask:    xxx????xxxx
    char pattern[] = "\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\x88\xB0";
    char mask[] = "xxx????xxxx";

    uint64_t offset = FindPattern(buffer, pattern, mask);

    if (offset != 0)
    {
        std::cout << "FOUND SIGNATURE AT OFFSET: 0x" << std::hex << std::uppercase << offset << std::endl;
        
        // Calculate RIP relative
        // Instruction: 48 8B 05 [Rel32]
        // Offset points to START of instruction (48)
        // Rel32 is at Offset + 3
        
        if (offset + 7 <= size)
        {
            int32_t relative = *(int32_t*)&buffer[offset + 3];
            std::cout << "Relative Offset (Le): 0x" << relative << std::endl;
            
            // Target = InstructionEnd + Relative
            // Instruction length = 7 bytes (48 8B 05 XX XX XX XX)
            uint64_t instructionEnd = offset + 7;
            uint64_t targetRVA = instructionEnd + relative;
            
             std::cout << "Resolved Target RVA: 0x" << std::hex << targetRVA << std::endl;
        }
    }
    else
    {
        std::cout << "Signature NOT found." << std::endl;
    }

    return 0;
}
