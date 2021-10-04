#undef UNICODE
#include <Windows.h>
#include <valinet/pdb/pdb.h>
#include <valinet/utility/memmem.h>
#include <valinet/utility/takeown.h>

int main(int argc, char** argv)
{
    BOOL bRestore = FALSE;

    char szTaskkill[MAX_PATH];
    ZeroMemory(
        szTaskkill,
        (MAX_PATH) * sizeof(char)
    );
    szTaskkill[0] = '\"';
    GetSystemDirectoryA(
        szTaskkill + sizeof(char),
        MAX_PATH
    );
    strcat_s(
        szTaskkill,
        MAX_PATH,
        "\\taskkill.exe\" /f /im dwm.exe"
    );

    char szPath[_MAX_PATH];
    ZeroMemory(
        szPath,
        (_MAX_PATH) * sizeof(char)
    );
    GetModuleFileNameA(
        GetModuleHandle(NULL),
        szPath,
        _MAX_PATH
    );
    PathStripPathA(szPath);

    char szOriginalDWM[_MAX_PATH];
    ZeroMemory(
        szOriginalDWM,
        (_MAX_PATH) * sizeof(char)
    );
    GetSystemDirectoryA(
        szOriginalDWM,
        MAX_PATH
    );
    strcat_s(
        szOriginalDWM,
        MAX_PATH,
        "\\uDWM_win11drc.bak"
    );
    bRestore = fileExists(szOriginalDWM);

    char szModifiedDWM[_MAX_PATH];
    ZeroMemory(
        szModifiedDWM,
        (_MAX_PATH) * sizeof(char)
    );
    if (bRestore)
    {
        GetSystemDirectoryA(
            szModifiedDWM,
            MAX_PATH
        );    
        strcat_s(
            szModifiedDWM,
            MAX_PATH,
            "\\uDWMm.dll"
        );
    }
    else
    {
        GetModuleFileNameA(
            GetModuleHandle(NULL),
            szModifiedDWM,
            _MAX_PATH
        );
        PathRemoveFileSpecA(szModifiedDWM);
        strcat_s(
            szModifiedDWM,
            MAX_PATH,
            "\\uDWM.dll"
        );
    }

    char szDWM[MAX_PATH];
    ZeroMemory(
        szDWM,
        (MAX_PATH) * sizeof(char)
    );
    GetSystemDirectoryA(
        szDWM,
        MAX_PATH
    );
    strcat_s(
        szDWM,
        MAX_PATH,
        "\\uDWM.dll"
    );

    if (bRestore)
    {
        DeleteFileA(szModifiedDWM);
        if (!MoveFileA(szDWM, szModifiedDWM))
        {
            printf("Unable to restore DWM.\n");
            _getch();
            return 1;
        }
        if (!MoveFileA(szOriginalDWM, szDWM))
        {
            printf("Unable to restore DWM.\n");
            _getch();
            return 2;
        }
    }
    else
    {

        if (!CopyFileA(szDWM, szModifiedDWM, FALSE))
        {
            printf(
                "Temporary file copy failed. Make sure the application has write "
                "access to the folder it runs from.\n"
            );
            _getch();
            return 1;
        }
        if (VnDownloadSymbols(
            NULL,
            szModifiedDWM,
            szModifiedDWM,
            _MAX_PATH
        ))
        {
            printf(
                "Unable to download symbols. Make sure you have a working Internet "
                "connection.\n"
            );
            _getch();
            return 2;
        }
        DWORD addr[1] = { 0 };
        char* name[1] = { "CTopLevelWindow::GetEffectiveCornerStyle" };
        if (VnGetSymbols(
            szModifiedDWM,
            addr,
            name,
            1
        ))
        {
            printf("Unable to determine function address.\n");
            _getch();
            return 3;
        }
        printf("Function address is: 0x%x.\n", addr[0]);
        DeleteFile(szModifiedDWM);
        PathRemoveFileSpecA(szModifiedDWM);
        strcat_s(
            szModifiedDWM,
            MAX_PATH,
            "\\uDWM.dll"
        );
        HANDLE hFile = CreateFileA(
            szModifiedDWM,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            0
        );
        if (!hFile)
        {
            printf("Unable to open system file.\n");
            _getch();
            return 4;
        }
        HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
        if (hFileMapping == 0)
        {
            printf("Unable to create file mapping.\n");
            _getch();
            return 5;
        }
        char* lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (lpFileBase == 0)
        {
            printf("Unable to memory map system file.\n");
            _getch();
            return 6;
        }
        char szPayload[8] = { 0x48, 0xC7, 0xC0, 0x00, 0x00, 0x00, 0x00, 0xc3}; // mov rax, 0; ret
        memcpy(lpFileBase + addr[0], szPayload, sizeof(szPayload));
        UnmapViewOfFile(lpFileBase);
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        if (!VnTakeOwnership(szDWM))
        {
            printf("Unable to take ownership of system file.\n");
            _getch();
            return 8;
        }
        DeleteFileA(szOriginalDWM);
        if (!MoveFileA(szDWM, szOriginalDWM))
        {
            printf("Unable to backup system file.\n");
            _getch();
            return 9;
        }
        if (!CopyFileA(szModifiedDWM, szDWM, FALSE))
        {
            printf("Unable to replace system file.\n");
            _getch();
            return 10;
        }
        DeleteFileA(szModifiedDWM);
    }
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    BOOL b = CreateProcessA(
        NULL,
        szTaskkill,
        NULL,
        NULL,
        TRUE,
        CREATE_UNICODE_ENVIRONMENT,
        NULL,
        NULL,
        &si,
        &pi
    );
    WaitForSingleObject(pi.hProcess, INFINITE);
    Sleep(10000);
    if (bRestore)
    {
        DeleteFileA(szModifiedDWM);
    }
    printf("Operation successful.\n");
	return 0;
}