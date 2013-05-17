#include <stdio.h>
#include <Windows.h>

#include "resource.h"

size_t PatternSearch(PBYTE haystack, size_t haystack_size, PBYTE pattern, size_t pattern_size) {
    // Makes string searching easier
    size_t pattern_last = pattern_size - 1;

    // Let us boyer-moore-horspool
    DWORD jump_table[0xFF+1];
    for (int i = 0; i <= 0xFF; i++) {
        jump_table[i] = pattern_size;
    }

    // Fill up the jump table w/ char's distance to end of needle
    for (size_t i = 0; i < pattern_last; i++) {
        jump_table[pattern[i]] = pattern_last - i;
    }

    // Compares starting from end of pattern & haystack[i]
    for (size_t i = pattern_last; i < haystack_size; i += jump_table[haystack[i]]) {
        // Nested loop iterates through partial matches
        for (size_t j = pattern_last; pattern[j] == haystack[i-pattern_last+j]; j--) {
            // 0 is the end because we iterate backwards, complete match
            if (j == 0) {
                return i-pattern_last;
            }
        }
    }

    return 0;
}

bool BackupDll(const char* file_location, PBYTE file_data, size_t file_size) {
    // Backup filename to filename.bak
    char dll_backup[1024] = {0};
    strcat(dll_backup, file_location);
    strcat(dll_backup, ".bak");
    FILE* backup_file = fopen(dll_backup, "wb");
    if (backup_file == nullptr) {
        return false;
    }
    fwrite(file_data, sizeof(BYTE), file_size, backup_file);
    fclose(backup_file);

    return true;
}

void BrowseForDll(HWND hwnd) {
	OPENFILENAMEA ofn;
	char file[1024];

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = file;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(file);
	ofn.lpstrFilter = "All (*.*)\0*.*\0Engine.dll\0ENGINE.DLL\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = "%USERPROFILE%";
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	GetOpenFileNameA(&ofn);

	SetDlgItemTextA(hwnd, IDC_FILE, file);

	return;
}

void PatchSuperRocketRagdolls(HWND hwnd) {
    char dll_location[1024] = {0};
    GetDlgItemTextA(hwnd, IDC_FILE, dll_location, 1024);

    // This pattern searches for the vector.y = vector.y     * vector.force
    // and changes it to             vector.y = vector.force * vector.force
    // FLD DWORD PTR DS:[EAX+4] ---> FLD DWORD PTR DS:[EAX+10]
    BYTE y_force_pattern[] = { 0xD9, 0x40, 0x04, 0xD8, 0xC9, 0xD9, 0x5D, 0xEC,
                               0xD8, 0x48, 0x08, 0x8B, 0x03, 0x8B, 0x90, 0x94 };
    size_t y_force_pattern_size = 16;
    BYTE y_force_replace[] = { 0xD9, 0x40, 0x10 };
    size_t y_force_replace_size = 3;

    // This pattern searches for the physics cap, a 32-bit float = 12.0
    // and changes it to                            32-bit float =  9.0
    // 00 00 40 41 ---> 00 00 10 41
    BYTE force_cap_pattern[] = { 0x00, 0x00, 0x40, 0x41, 0x6F, 0x12, 0x83, 0x3A,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2E, 0x40 };
    size_t force_cap_pattern_size = 16;
    BYTE force_cap_replace[] = { 0x00, 0x00, 0x10, 0x41 };
    size_t force_cap_replace_size = 4;

    FILE* file = fopen(dll_location, "rb+");

    // Make sure we opened it
    if (file == nullptr) {
        MessageBox(hwnd, L"Could not open Engine.dll", L"Error", MB_OK);
        return;
    }

    // Grab file size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Stick it into memory cus C++ has no good file search
    PBYTE file_data = new BYTE[file_size];
    fread(file_data, sizeof(BYTE), file_size, file);

    // Backup the dll
    if (!BackupDll(dll_location, file_data, file_size)) {
        MessageBox(hwnd, L"Failed to create backup of Engine.dll, aborting", L"Error", MB_OK);
        fclose(file);
        delete [] file_data;
        return;
    }

    // Find the file positions of our patterns
    size_t match = PatternSearch(file_data, file_size, y_force_pattern, y_force_pattern_size);
    if (!match) {
        MessageBox(hwnd, L"Could not locate physics height force code", L"Error", MB_OK);
        fclose(file);
        delete [] file_data;
        return;
    }
    fseek(file, match, SEEK_SET);

    // The replacing part of search & replace :)
    fwrite(y_force_replace, sizeof(BYTE), y_force_replace_size, file);

    // Repeat everything (it's ugly but a new struct type w/ dynamically allocated arrays seemed like overkill) 
    match = PatternSearch(file_data, file_size, force_cap_pattern, force_cap_pattern_size);
    if (!match) {
        MessageBox(hwnd, L"Could not locate physics max force code", L"Error", MB_OK);
        fclose(file);
        delete [] file_data;
        return;
    }
    fseek(file, match, SEEK_SET);

    // Last replace
    fwrite(force_cap_replace, sizeof(BYTE), force_cap_replace_size, file);

    MessageBox(hwnd, L"Engine.dll patched successfully", L"Success", MB_OK);

    // Cleanup
    fclose(file);
    delete [] file_data;
    return;
}

BOOL CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_INITDIALOG: {
            // Set icon
            HICON icon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDR_ICON));
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
                    PatchSuperRocketRagdolls(hwnd);
                    return TRUE;
            }
    }

    return FALSE;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmdline, int cmdshow) {
    return DialogBox(instance, MAKEINTRESOURCE(ID_DLG), nullptr, (DLGPROC)DialogProc);
}
