// Needed for BYTE definition
#include <Windows.h>

// This pattern searches for the physics cap, a 32-bit float = 12.0
// 00 00 40 41
const BYTE force_cap_pattern[] = {0x00, 0x00, 0x40, 0x41,
                                  0x6F, 0x12, 0x83, 0x3A,
                                  0x00, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x2E, 0x40};
const bool force_cap_mask[]    = {true, true,false, true,
                                  true, true, true, true,
                                  true, true, true, true,
                                  true, true, true, true};

// This pattern searches for vector.y = vector.y * vector.force
// FLD DWORD PTR DS:[EAX+4]
const BYTE y_force_pattern[] = {0xD9, 0x40, 0x04, 0xD8,
                                0xC9, 0xD9, 0x5D, 0xEC,
                                0xD8, 0x48, 0x08, 0x8B,
                                0x03, 0x8B, 0x90, 0x94};
const bool y_force_mask[]    = {true, true,false, true,
                                true, true, true, true,
                                true, true, true, true,
                                true, true, true, true};