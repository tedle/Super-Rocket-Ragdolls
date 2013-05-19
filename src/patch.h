// patch.h---------------------------------------------------------------------
// Defines various functions used for interacting with Grim Dawn's .DLL files
// General usage involves using LoadFile to place a .DLL into memory and then
// PatchMemory to apply changes to it, and finally SaveFile to replace the
// vanilla .DLL file
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <Windows.h>

// Used to tell PatchMemory what kind of physics modifications to make
enum PatchType {
    kOriginal            = 0,
    kSuperRocketRagdolls = 1,
    kSuperFlyingBodies   = 2,
    kLesserFlyingBodies  = 3
};

// Loads file into memory
// Uses file_data and file_size as output args
// Returns true on success, false on failure
bool LoadFile(const char* filename, PBYTE* file_data, size_t* file_size);

// Saves file from memory
// Use of backup flag will append ".bak" to filename
// Backup will not be written if one already exists
// Returns true on success, false on failure
bool SaveFile(const char* file_location, PBYTE file_data, size_t file_size,
              bool backup=false);

// Searches for needle in haystack
// If needle_mask[i] is false, needle[i] is not evaluated
// Returns position of first match
size_t PatternSearch(const BYTE* haystack, const size_t haystack_size,
                     const BYTE* needle, const size_t needle_size,
                     const bool* needle_mask);

// Scans file in memory for the code we need to modify to make
// new physics behaviours happen
// Returns true on success, false otherwise
bool PatchMemory(PBYTE file_data, const size_t file_size, PatchType type);
