// main.cpp -------------------------------------------------------------------
// Contains program's entry point as well as all functions used to interact
// with the Win32 API
// ----------------------------------------------------------------------------

// Need some WinXP features
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define NTDDI_VERSION 0x05010300

#include <stdio.h>
#include <Windows.h>

#include "patch.h"
#include "resource.h"

// Creates a browse for file dialog using Win32 API
void BrowseForDll(HWND hwnd) {
    OPENFILENAMEA ofn = {0};
    char file[1024];

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = file;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(file);
    ofn.lpstrFilter = "All (*.*)\0*.*\0Engine.dll\0ENGINE.DLL\0";
    ofn.nFilterIndex = 2;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = "%PROGRAMFILES(X86)%\\Steam\\"
                          "steamapps\\common\\Grim Dawn";
    ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

    GetOpenFileNameA(&ofn);

    SetDlgItemTextA(hwnd, IDC_FILE, file);

    return;
}

// Patches Grim Dawn's Engine.dll to provide rocket ragdoll physics
void PatchRagdolls(HWND hwnd) {
    // Grab file location from text box
    char dll_location[1024] = {0};
    GetDlgItemTextA(hwnd, IDC_FILE, dll_location, 1024);

    PBYTE file_data;
    size_t file_size;
    PatchType patch;

    // Check our selected physics behaviour before opening any files
    int combo_selection = SendMessage(GetDlgItem(hwnd, IDC_PATCHTYPE),
                                      CB_GETCURSEL, 0, 0);
    if (combo_selection == CB_ERR) {
        MessageBox(hwnd, L"Please select a physics behaviour", L"Error", MB_OK);
        return;
    }
    patch = static_cast<PatchType>(combo_selection);

    // Load Engine.dll into memory
    if (!LoadFile(dll_location, &file_data, &file_size)) {
        MessageBox(hwnd, L"Failed to open Engine.dll", L"Error", MB_OK);
        return;
    }

    // Backup the dll
    if (!SaveFile(dll_location, file_data, file_size, true)) {
        MessageBox(hwnd, L"Failed to create backup of Engine.dll, aborting",
                   L"Error", MB_OK);
        delete [] file_data;
        return;
    }

    // Patch the file loaded into memory with new physics settings
    bool success = PatchMemory(file_data, file_size, patch);

    // Oh no!
    if (!success) {
        MessageBox(hwnd, L"Failed to patch Engine.dll", L"Error", MB_OK);
        delete [] file_data;
        return;
    }

    // Output our patched file_data to the dll file
    if (!SaveFile(dll_location, file_data, file_size)) {
        MessageBox(hwnd, L"Failed to open Engine.dll", L"Error", MB_OK);
        delete [] file_data;
        return;
    }

    // Yay!
    MessageBox(hwnd, L"Engine.dll patched successfully", L"Success", MB_OK);

    // Cleanup
    delete [] file_data;
    return;
}

// Dialog window callbacks
// Returns TRUE when message was handled
BOOL CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_INITDIALOG: {
            // Set dialog icon
            HICON icon = LoadIcon(GetModuleHandle(0),
                                  MAKEINTRESOURCE(IDR_ICON));
            SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
            // Set the text box cue banner
            // 0x1501 is EM_SETCUEBANNER, this is undeclared for some reason...?
            SendMessage(GetDlgItem(hwnd, IDC_FILE), 0x1501, 0,
                        (LPARAM)L"Location of Engine.dll");

            // Populate combobox
            HWND combo = GetDlgItem(hwnd, IDC_PATCHTYPE);
            SendMessage(combo, CB_INSERTSTRING, kSuperRocketRagdolls,
                        (LPARAM)L"Super Rocket Ragdolls");
            SendMessage(combo, CB_INSERTSTRING, kSuperFlyingBodies,
                        (LPARAM)L"Super Flying Bodies");
            SendMessage(combo, CB_INSERTSTRING, kLesserFlyingBodies,
                        (LPARAM)L"Lesser Flying Bodies");
            SendMessage(combo, CB_INSERTSTRING, kOriginal,
                        (LPARAM)L"Regular Ragdolls");
            // Select default "Super Rocket Ragdolls"
            SendMessage(combo, CB_SETCURSEL, 0, 0);
            return TRUE;
        }

        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wparam)) {
                case IDC_BROWSE:
                    BrowseForDll(hwnd);
                    return TRUE;

                case IDC_PATCH:
                    PatchRagdolls(hwnd);
                    return TRUE;
            }
    }

    return FALSE;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmdline,
                   int cmdshow) {
    return DialogBox(instance, MAKEINTRESOURCE(ID_DLG), nullptr,
                     (DLGPROC)DialogProc);
}
