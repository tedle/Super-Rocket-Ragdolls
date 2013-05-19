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
	ofn.lpstrInitialDir = "%PROGRAMFILES(X86)%\\Steam\\steamapps\\common\\Grim Dawn";
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	GetOpenFileNameA(&ofn);

	SetDlgItemTextA(hwnd, IDC_FILE, file);

	return;
}

// Backs up file_location to file_location.bak, expects file already loaded in
// memory for optimization reasons
// Returns true on success, false otherwise
bool WriteDll(const char* file_location, PBYTE file_data, size_t file_size, bool backup=false) {
    FILE* file;
    char dll_backup[1024] = {0};
    strcat_s(dll_backup, file_location);
    if (backup) {
        strcat_s(dll_backup, ".bak");
        // Checks if there is already a backup of the dll, and aborts if so
        // because this implies Engine.dll is already modified and should not
        // be written to a backup
        fopen_s(&file, dll_backup, "r");
        if (file != nullptr) {
            fclose(file);
            return true;
        }
    }
    if (fopen_s(&file, dll_backup, "wb") != 0) {
        return false;
    }
    fwrite(file_data, sizeof(BYTE), file_size, file);
    fclose(file);

    return true;
}

// Patches Grim Dawn's Engine.dll to provide rocket ragdoll physics
void PatchRagdolls(HWND hwnd) {
    // Grab file location from text box
    char dll_location[1024] = {0};
    GetDlgItemTextA(hwnd, IDC_FILE, dll_location, 1024);

    PBYTE file_data;
    size_t file_size;

    // Load Engine.dll into memory
    if (!LoadFile(dll_location, &file_data, &file_size)) {
        MessageBox(hwnd, L"Failed to open Engine.dll", L"Error", MB_OK);
        return;
    }

    // Backup the dll
    if (!WriteDll(dll_location, file_data, file_size, true)) {
        MessageBox(hwnd, L"Failed to create backup of Engine.dll, aborting",
                   L"Error", MB_OK);
        delete [] file_data;
        return;
    }

    bool success = PatchSuperRocketRagdolls(file_data, file_size);

    // Oh no!
    if (!success) {
        MessageBox(hwnd, L"Failed to patch Engine.dll", L"Error", MB_OK);
        delete [] file_data;
        return;
    }

    // Output our patched file_data to the dll file
    if (!WriteDll(dll_location, file_data, file_size)) {
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
