#pragma once
#include <windows.h>

namespace mem
{
    template <typename T>
    inline T Read(HANDLE hProcess, uintptr_t address)
    {
        T buffer{};
        SIZE_T bytesRead;
        ReadProcessMemory(hProcess, (LPCVOID)address, &buffer, sizeof(T), &bytesRead);
        return buffer;
    }

    template <typename T>
    inline bool Write(HANDLE hProcess, uintptr_t address, const T& value)
    {
        SIZE_T bytesWritten;
        return WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(T), &bytesWritten);
    }

    // Optional: Read array or raw buffer
    inline bool ReadRaw(HANDLE hProcess, uintptr_t address, void* buffer, SIZE_T size)
    {
        SIZE_T bytesRead;
        return ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, &bytesRead);
    }

    inline bool WriteRaw(HANDLE hProcess, uintptr_t address, const void* buffer, SIZE_T size)
    {
        SIZE_T bytesWritten;
        return WriteProcessMemory(hProcess, (LPVOID)address, buffer, size, &bytesWritten);
    }
}
