#include "Memory.h"

bool Memory::Attach(const wchar_t* processName) {
    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;

    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, processName) == 0) {
                processId = entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);

    if (processId == 0) return false;

    processHandle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (!processHandle) return false;

    clientBase = GetModuleBase(L"client.dll");
    engineBase = GetModuleBase(L"engine2.dll");

    return clientBase != 0;
}

void Memory::Detach() {
    if (processHandle) {
        CloseHandle(processHandle);
        processHandle = nullptr;
    }
    clientBase = 0;
    engineBase = 0;
    processId = 0;
}

uintptr_t Memory::GetModuleBase(const wchar_t* moduleName) {
    if (!processHandle) return 0;

    MODULEENTRY32W entry{};
    entry.dwSize = sizeof(MODULEENTRY32W);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    uintptr_t base = 0;
    if (Module32FirstW(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szModule, moduleName) == 0) {
                base = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
                break;
            }
        } while (Module32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return base;
}

bool Memory::ReadRaw(uintptr_t address, void* buffer, size_t size) {
    if (!processHandle || !address) return false;
    return ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(address), buffer, size, nullptr);
}
