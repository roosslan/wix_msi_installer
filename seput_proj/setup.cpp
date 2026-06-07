/* build g++ setup.cpp -static-libgcc -static-libstdc++ */
#include <iostream>
#include <windows.h>
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>
#include <string.h>
#include <shlwapi.h>
#include <filesystem>
#include "shlobj.h"

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "msi.lib")


namespace fs = std::filesystem;

void execute(LPWSTR arguments) {
    // additional information
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    // set the size of the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // start the program up
    CreateProcessW (
        nullptr,                // the path
        arguments,              // Command line
        nullptr,                // Process handle not inheritable
        nullptr,                // Thread handle not inheritable
        FALSE,                  // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE,     // Opens file in a separate console
        nullptr,                // Use parent's environment block
        nullptr,                // Use parent's starting directory
        &si,                    // Pointer to STARTUPINFO structure
        &pi                     // Pointer to PROCESS_INFORMATION structure
    );

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

std::wstring get_msi_filename() {
    for (auto &p : fs::directory_iterator("."))
    {
        if (p.path().extension() == ".msi")
            return p.path().filename().wstring();
    }
    return nullptr;
}

void kill_process(const char *filename) {
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
    PROCESSENTRY32 pEntry;
    pEntry.dwSize = sizeof (pEntry);
    BOOL hRes = Process32First(hSnapShot, &pEntry);
    while (hRes)
    {
        if (strcmp(pEntry.szExeFile, filename) == 0)
        {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
                                          (DWORD) pEntry.th32ProcessID);
            if (hProcess != NULL) {
                TerminateProcess(hProcess, 9);
                CloseHandle(hProcess);
            }
        }
        hRes = Process32Next(hSnapShot, &pEntry);
    }
    CloseHandle(hSnapShot);
}

int main(int argc, char* argv[]) {
    kill_process("bgHelper.exe");
    kill_process("worksets_ctrl.exe");
    kill_process("DbChecker.exe");
    kill_process("PluginVersionUpdater.exe");
    kill_process("rfaWindow.exe");
    kill_process("warnWindow.exe");
    kill_process("ifc_exporter.exe");

    std::string install_directory = std::getenv("APPDATA");
    install_directory += "\\software_installation";

    fs::remove_all(install_directory);
    std::wstring msi_path = L"msiexec /i" + get_msi_filename() + L" SETUP_GUID=\"cabba1a2-dead-babe-b00b-ca11ab1e10ad\"";

    execute(msi_path.data());   /* .data() converts wstring to LPWSTR */

    if (GetDriveType(TEXT("L:\\")) == DRIVE_REMOTE) {
        // "L:\PathTo"
    };

    return 0;
}