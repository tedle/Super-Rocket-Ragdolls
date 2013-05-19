// pattern.h-------------------------------------------------------------------
// Contains various search & replace byte code patterns for modifying the
// the Grim Dawn's .DLL files
// ----------------------------------------------------------------------------

// Needed for BYTE definition
#include <Windows.h>

namespace {
// ---------------
// Search Patterns
// ---------------

// This pattern searches for the physics cap, a 32-bit float = 12.0
// 00 00 40 41
const BYTE kForceCap[] =        {0x00, 0x00, 0x40, 0x41,
                                 0x6F, 0x12, 0x83, 0x3A,
                                 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x2E, 0x40};
const bool kForceCapMask[]    = {true, true,false,false,
                                 true, true, true, true,
                                 true, true, true, true,
                                 true, true, true, true};

// This pattern searches for vector.y = vector.y * vector.force
// FLD DWORD PTR DS:[EAX+4]
const BYTE kUpForce[] =        {0xD9, 0x40, 0x04, 0xD8,
                                0xC9, 0xD9, 0x5D, 0xEC,
                                0xD8, 0x48, 0x08, 0x8B,
                                0x03, 0x8B, 0x90, 0x94};
const bool kUpForceMask[]    = {true, true,false, true,
                                true, true, true, true,
                                true, true, true, true,
                                true, true, true, true};

// -------------------
// Replacement Patches
// -------------------

// Replace vector.y = vector.y          * vector.force
// with    vector.y = (1/3)vector.force * vector.force
// FLD DWORD PTR DS:[EAX+4] ---> FLD DWORD PTR DS:[EAX+10]
const BYTE kUpForceRocket[] = {0xD9, 0x40, 0x10};

// Replace physics cap, a 32-bit float
// with                   32-bit float = 9.0
// 00 00 40 41 ---> 00 00 10 41
const BYTE kForceCapRocket[] = {0x00, 0x00, 0x10, 0x41};

// Replace physics cap, a 32-bit float
// with                   32-bit float = 24.0
// 00 00 40 41 ---> 00 00 C0 41
const BYTE kForceCapSuperFlying[] = {0x00, 0x00, 0xC0, 0x41};

// Replace physics cap, a 32-bit float
// with                   32-bit float = 1.0
// 00 00 40 41 ---> 00 00 80 3F
const BYTE kForceCapLesserFlying[] = {0x00, 0x00, 0x80, 0x3F};
}