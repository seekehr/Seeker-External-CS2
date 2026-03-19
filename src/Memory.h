#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <string>
#include <vector>

class Memory {
public:
    HANDLE processHandle = nullptr;
    uintptr_t clientBase = 0;
    uintptr_t engineBase = 0;
    DWORD processId = 0;

    bool Attach(const wchar_t* processName = L"cs2.exe");
    void Detach();

    uintptr_t GetModuleBase(const wchar_t* moduleName);
    uintptr_t GetClientBase() const { return clientBase; }
    uintptr_t GetEngineBase() const { return engineBase; }

    template<typename T>
    bool Read(uintptr_t address, T& buffer) {
        if (!processHandle || !address) return false;
        return ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(address), &buffer, sizeof(T), nullptr);
    }

    template<typename T>
    bool Write(uintptr_t address, const T& value) {
        if (!processHandle || !address) return false;
        return WriteProcessMemory(processHandle, reinterpret_cast<LPVOID>(address), &value, sizeof(T), nullptr);
    }

    bool ReadRaw(uintptr_t address, void* buffer, size_t size);
    bool IsValid() const { return processHandle != nullptr && processHandle != INVALID_HANDLE_VALUE; }
};

inline Memory g_Memory;
