// patch.cpp-------------------------------------------------------------------
// Uses various file IO to load game code into memory for modification. This
// is for optimization reasons since it lets us be more modular in the changes
// we make as well as allow us to hold off on committing any changes to the
// .DLL file until we're sure the full patch will be successful
// ----------------------------------------------------------------------------

#include "patch.h"
#include "pattern.h"

bool LoadFile(const char* file_name, PBYTE* file_data, size_t* file_size) {

    FILE* file;
    if (fopen_s(&file, file_name, "rb") != 0) {
        return false;
    }

    // Grab file size
    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // File will be loaded into memory, don't wanna explode user's RAM over
    // a file that should only be 3MB~ in size
    if (*file_size > 1024 * 1024 * 20) {
        fclose(file);
        return false;
    }

    // Stick file contents into memory cus stdlib has no good file search
    *file_data = new BYTE[*file_size];
    fread(*file_data, sizeof(BYTE), *file_size, file);

    // Cleanup
    fclose(file);
    return true;
}

// Expects file already loaded into memory for optimization reasons
bool SaveFile(const char* file_location, PBYTE file_data, size_t file_size,
              bool backup) {
    FILE* file;
    char dll[1024] = {0};
    strcat_s(dll, file_location);
    if (backup) {
        strcat_s(dll, ".bak");
        // Checks if there is already a backup of the dll, and aborts if so
        // because this implies Engine.dll is already modified and should not
        // be written to a backup
        fopen_s(&file, dll, "r");
        if (file != nullptr) {
            fclose(file);
            return true;
        }
    }
    if (fopen_s(&file, dll, "wb") != 0) {
        return false;
    }
    fwrite(file_data, sizeof(BYTE), file_size, file);
    fclose(file);

    return true;
}

// Does a pattern search of binary data using Boyer-Moore-Horspool algorithm
size_t PatternSearch(const BYTE* haystack, const size_t haystack_size,
                     const BYTE* needle, const size_t needle_size,
                     const bool* needle_mask) {
    // Makes string searching easier
    size_t needle_last = needle_size - 1;

    // Zeroing out jump table, without the 0s
    DWORD jump_table[0xFF+1];
    for (int i = 0; i <= 0xFF; i++) {
        jump_table[i] = needle_size;
    }

    // Fill up the jump table w/ char's distance to end of needle
    for (size_t i = 0; i < needle_last; i++) {
        jump_table[needle[i]] = needle_last - i;
    }

    // Compares starting from end of pattern & haystack[i]
    for (size_t i = needle_last; i < haystack_size;
         i += jump_table[haystack[i]]) {
        // Nested loop iterates through potential matches
        // Breaks only when needle[j] does not match AND needle_mask[j] is true
        for (size_t j = needle_last;
             needle[j] == haystack[i-needle_last+j] || !needle_mask[j]; j--) {
            // 0 is the end because we iterate backwards, complete match
            if (j == 0) {
                return i-needle_last;
            }
        }
    }

    return 0;
}

// Searches for predetermined patterns discovered through debugging
// in the file_data (of Engine.dll), and replaces them with new values
// to provide the changed physics behaviour
// The patterns and replacements are all defined in pattern.h
bool PatchMemory(PBYTE file_data, const size_t file_size, PatchType type) {
    // Find the file position of the y force pattern
    size_t up_match = PatternSearch(file_data, file_size, kUpForce,
                                   sizeof(kUpForce), kUpForceMask);
    // Find the file position of the physics cap pattern
    size_t cap_match = PatternSearch(file_data, file_size, kForceCap,
                                     sizeof(kForceCap), kForceCapMask);
    if (!up_match || !cap_match) {
        return false;
    }

    // memcpy is called with search patterns instead of predefined replacements
    // when it makes sense to ensure memory blocks are at their default values
    switch (type) {
        case kSuperRocketRagdolls:
            memcpy(file_data+up_match, kUpForceRocket,
                   sizeof(kUpForceRocket));
            memcpy(file_data+cap_match, kForceCapRocket,
                   sizeof(kForceCapRocket));
            break;
        case kSuperFlyingBodies:
            memcpy(file_data+up_match, kUpForce,
                   sizeof(kUpForce));
            memcpy(file_data+cap_match, kForceCapSuperFlying,
                   sizeof(kForceCapSuperFlying));
            break;
        case kLesserFlyingBodies:
            memcpy(file_data+up_match, kUpForce,
                   sizeof(kUpForce));
            memcpy(file_data+cap_match, kForceCapLesserFlying,
                   sizeof(kForceCapLesserFlying));
            break;
        case kOriginal:
            memcpy(file_data+up_match, kUpForce,
                   sizeof(kUpForce));
            memcpy(file_data+cap_match, kForceCap,
                   sizeof(kForceCap));
            break;
        default:
            return false;
            break;
    }
    return true;
}