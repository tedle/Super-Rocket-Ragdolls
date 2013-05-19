#include <stdio.h>
#include <Windows.h>

// Loads file into memory
// Uses file_data and file_size as output args
// Returns true on success, false on failure
bool LoadFile(const char* filename, PBYTE* file_data, size_t* file_size);

// Searches for needle in haystack
// If needle_mask[i] is false, needle[i] is not evaluated
// Returns position of first match
size_t PatternSearch(const PBYTE haystack, const size_t haystack_size,
                     const PBYTE needle, const size_t needle_size,
                     const bool* needle_mask);

// Scans file in memory for the code we need to modify to make
// Super Rocket Ragdolls happen
// Returns true on success, false otherwise
bool PatchSuperRocketRagdolls(PBYTE file_data, const size_t file_size);