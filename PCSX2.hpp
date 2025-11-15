#include "stdafx.h"
#include "ProcMem.hpp"
#include <filesystem>

namespace PCSX2
{
	inline uintptr_t PCSX2Base;
	inline HANDLE PCSX2ProcHandle;

	namespace VM
	{
		inline uintptr_t PS2Base;

		template <typename T>
		inline T Read(uintptr_t address)
		{
			return mem::Read<T>(PCSX2ProcHandle, PS2Base + address);
		}

		template <typename T>
		inline bool Write(uintptr_t address, const T& value)
		{
			return mem::Write<T>(PCSX2ProcHandle, PS2Base + address, value);
		}

		inline bool ReadRaw(uintptr_t address, void* buffer, SIZE_T size)
		{
			return mem::ReadRaw(PCSX2ProcHandle, PS2Base + address, buffer, size);
		}

		inline bool WriteRaw(uintptr_t address, const void* buffer, SIZE_T size)
		{
			return mem::WriteRaw(PCSX2ProcHandle, PS2Base + address, buffer, size);
		}
	}

	enum LaunchError : int
	{
		LaunchError_UnknownError = -1,
		LaunchError_NoError,
		LaunchError_CreateProcessFail,
		LaunchError_BaseAddressFail,
		LaunchError_Count,
	};

	bool IsInFocus();
	bool IsOpen();
	void CloseHandles();
	DWORD GetPID();
	bool IsReady();
	int Launch(std::filesystem::path exePath);
}