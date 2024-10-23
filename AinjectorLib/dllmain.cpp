// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <Windows.h>
#include <TlHelp32.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
extern "C" __declspec(dllexport) bool Inject(DWORD processID, const char* DllPath);

bool Inject(DWORD processID, const char* DllPath) {
    if (processID == 0 || !DllPath)
        return false;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (hProcess == NULL) {
        return false; // OpenProcess失败

        void* allocated = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!allocated) {
            CloseHandle(hProcess);
            return false; // VirtualAllocEx失败
        }

        if (!WriteProcessMemory(hProcess, allocated, DllPath, strlen(DllPath) + 1, NULL)) {
            VirtualFreeEx(hProcess, allocated, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return false; // WriteProcessMemory失败
        }

        HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, allocated, 0, NULL);
        if (hThread == NULL) {
            VirtualFreeEx(hProcess, allocated, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return false; // CreateRemoteThread失败
        }

        CloseHandle(hThread);
        CloseHandle(hProcess);
        return true;
    }
}
