#pragma once
#include "PCSX2.hpp"
#include <tlhelp32.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <filesystem>
#include "ProcMem.hpp"

#ifndef PCSX2_HPP
#define PCSX2_HPP

namespace PCSX2
{
    PROCESS_INFORMATION pi = { 0 };

    DWORD GetPID()
    {
        return pi.dwProcessId;
    }

    static uintptr_t GetBaseAddress(DWORD pid)
    {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
        if (snapshot == INVALID_HANDLE_VALUE)
            return 0;

        MODULEENTRY32 me;
        me.dwSize = sizeof(me);

        uintptr_t baseAddr = 0;
        if (Module32First(snapshot, &me)) {
            baseAddr = (uintptr_t)me.modBaseAddr;
        }

        CloseHandle(snapshot);
        return baseAddr;
    }

    bool IsInFocus()
    {
        HWND fg = GetForegroundWindow();
        if (!fg) return false;

        DWORD pid = 0;
        GetWindowThreadProcessId(fg, &pid);
        return pid == pi.dwProcessId;
    }

    bool IsOpen()
    {
        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        return exitCode == STILL_ACTIVE;
    }

    bool IsReady()
    {
        return (PCSX2Base != 0) && (PCSX2ProcHandle != nullptr);
    }

    void CloseHandles()
    {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }

	int Launch(std::filesystem::path exePath)
	{
        STARTUPINFOA si = { 0 };
        si.cb = sizeof(si);

        if (!CreateProcessA(
            exePath.string().c_str(),
            nullptr,
            nullptr, nullptr,
            FALSE,
            0,
            nullptr, nullptr,
            &si, &pi))
        {
            std::cerr << "CreateProcess failed.\n";
            return LaunchError_CreateProcessFail;
        }

        // Give it a moment to load modules (usually instant, but just to be safe)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        PCSX2ProcHandle = pi.hProcess;
        PCSX2Base = GetBaseAddress(pi.dwProcessId);
        if (PCSX2Base)
        {
            VM::PS2Base = (PCSX2Base & 0xFFFFF0000000) + 0x40000000;
        }
        else
        {
            return LaunchError_BaseAddressFail;
        }

        return LaunchError_NoError;
	}
}

#endif