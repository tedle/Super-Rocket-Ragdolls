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
bool PatchSuperRocketRagdolls(PBYTE file_data, const size_t file_size) {
    // Search for the physics cap, a 32-bit float
    // and change it to              32-bit float =  9.0
    // 00 00 40 41 ---> 00 00 10 41
    BYTE force_cap_replace[] = {0x00, 0x00, 0x10, 0x41};

    // Search for skyward momentum vector.y = vector.y          * vector.force
    // and change it to            vector.y = (1/3)vector.force * vector.force
    // FLD DWORD PTR DS:[EAX+4] ---> FLD DWORD PTR DS:[EAX+10]
    BYTE y_force_replace[] = {0xD9, 0x40, 0x10};

    // Find the file position of the first pattern
    size_t match = PatternSearch(file_data, file_size, y_force_pattern,
                                 sizeof(y_force_pattern), y_force_mask);
    if (!match) {
        return false;
    }

    // The replacing part of search & replace :)
    memcpy(file_data+match, y_force_replace, sizeof(y_force_replace));

    // Find the file position of the second pattern
    match = PatternSearch(file_data, file_size, force_cap_pattern,
                          sizeof(force_cap_pattern), force_cap_mask);
    if (!match) {
        return false;
    }

    // Last replace
    memcpy(file_data+match, force_cap_replace, sizeof(force_cap_replace));

    return true;
}