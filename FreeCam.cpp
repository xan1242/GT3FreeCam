#include "stdafx.h"
#include "FreeCam.hpp"
#include "PCSX2.hpp"
#include "Matrix4.hpp"

#include <Xinput.h>
#pragma comment(lib, "XInput.lib")

namespace FreeCam
{
    bool bEnabled;

    // #TODO reconfiguration via a config file...
	namespace Controls
	{
        bool bToggleOldState;

		constexpr uint32_t vkToggle = VK_BACK;
		constexpr uint32_t vkTurbo = VK_LSHIFT;
		constexpr uint32_t vkSuperTurbo = 'F';
		constexpr uint32_t vkForwards = 'W';
		constexpr uint32_t vkBackwards = 'S';
		constexpr uint32_t vkLeft = 'A';
		constexpr uint32_t vkRight = 'D';
		constexpr uint32_t vkUp = VK_SPACE;
		constexpr uint32_t vkDown = VK_LCONTROL;

		constexpr uint32_t vkLookUp = 'I';
		constexpr uint32_t vkLookDown = 'K';
		constexpr uint32_t vkLookLeft = 'J';
		constexpr uint32_t vkLookRight = 'L';

        constexpr uint32_t vkTiltClockwise = 'U';
        constexpr uint32_t vkTiltCounterclockwise = 'O';

		// Controller controls are the same as NFS debug camera
		XINPUT_STATE xiState;
        constexpr float xiStickDeadzonePct = 0.20f;
		constexpr SHORT xiDeadzoneLS = static_cast<SHORT>(xiStickDeadzonePct * std::numeric_limits<SHORT>::max());
		constexpr SHORT xiDeadzoneRS = static_cast<SHORT>(xiStickDeadzonePct * std::numeric_limits<SHORT>::max());
		constexpr int XInputPort = 1; // 2nd controller port
		bool bControllerPresent;

		constexpr float TurboSpeed = 5.0f;
		constexpr float SuperTurboSpeed = 10.0f;

        //
        // KEYBOARD CONTROLS
        //

        static bool GetAsyncKeyDown(int vKey)
        {
            return GetAsyncKeyState(vKey) & 0x8000;
        }

        ControlPacket GetKeyboardValues()
        {
            ControlPacket out = { 0 };
            float movementVal = 1.0f;

            if (GetAsyncKeyDown(vkTurbo))
            {
                movementVal *= TurboSpeed;
            }
            else if (GetAsyncKeyDown(vkSuperTurbo))
            {
                movementVal *= SuperTurboSpeed;
            }


            if (GetAsyncKeyDown(vkForwards))
            {
                out.movementValZ += movementVal;
            }

            if (GetAsyncKeyDown(vkBackwards))
            {
                out.movementValZ -= movementVal;
            }

            if (GetAsyncKeyDown(vkLeft))
            {
                out.movementValX -= movementVal;
            }

            if (GetAsyncKeyDown(vkRight))
            {
                out.movementValX += movementVal;
            }

            if (GetAsyncKeyDown(vkUp))
            {
                out.movementValY += movementVal;
            }

            if (GetAsyncKeyDown(vkDown))
            {
                out.movementValY -= movementVal;
            }

            if (GetAsyncKeyDown(vkLookUp))
            {
                out.pitchVal += 0.03f;
            }

            if (GetAsyncKeyDown(vkLookDown))
            {
                out.pitchVal -= 0.03f;
            }

            if (GetAsyncKeyDown(vkLookLeft))
            {
                out.yawVal += 0.03f;
            }

            if (GetAsyncKeyDown(vkLookRight))
            {
                out.yawVal -= 0.03f;
            }

            if (GetAsyncKeyDown(vkTiltClockwise))
            {
                out.rollVal -= 0.01f;
            }

            if (GetAsyncKeyDown(vkTiltCounterclockwise))
            {
                out.rollVal += 0.01f;
            }

            return out;
        }

        //
        // XINPUT CONTROLS
        //

        static SHORT calculateAxisValue(SHORT rawValue, SHORT deadzone)
        {
            // Calculate the deadzone-adjusted value
            if (std::abs(rawValue) < deadzone)
            {
                return 0;  // Value within deadzone, return 0
            }
            else
            {
                // Calculate the adjusted value outside the deadzone
                SHORT sign = (rawValue > 0) ? 1 : -1;
                float ratio = (std::abs(rawValue) - deadzone) / ((float)std::numeric_limits<SHORT>::max() - deadzone);
                SHORT adjustedValue = static_cast<SHORT>(ratio * (float)std::numeric_limits<SHORT>::max()) * sign;
                return adjustedValue;
            }
        }

        bool GetControllerPresence()
        {
            return bControllerPresent;
        }

        ControlPacket GetControllerValues()
        {
            ControlPacket out = { 0 };
            if (!GetControllerPresence())
                return out;
            
            out.movementValX = static_cast<float>(xiState.Gamepad.sThumbLX) / static_cast<float>(std::numeric_limits<SHORT>::max());
            out.movementValZ = static_cast<float>(xiState.Gamepad.sThumbLY) / static_cast<float>(std::numeric_limits<SHORT>::max());

            if (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_START)
            {
                out.rollVal += (static_cast<float>(xiState.Gamepad.bRightTrigger) / 255.0f) * 0.01f;
                out.rollVal -= (static_cast<float>(xiState.Gamepad.bLeftTrigger) / 255.0f) * 0.01f;
            }
            else
            {
                out.movementValY = (static_cast<float>(xiState.Gamepad.bRightTrigger) / 255.0f) - (static_cast<float>(xiState.Gamepad.bLeftTrigger) / 255.0f);
            }

            out.yawVal = static_cast<float>(xiState.Gamepad.sThumbRX) / static_cast<float>(std::numeric_limits<SHORT>::max()) * -0.04f;
            out.pitchVal = static_cast<float>(xiState.Gamepad.sThumbRY) / static_cast<float>(std::numeric_limits<SHORT>::max()) * 0.04f;

            if (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_A)
            {
                out.movementValZ = 1.0f;
            }
            else if (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
            {
                out.movementValZ = -1.0f;
            }

            if (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_B)
            {
                out.movementValX = 1.0f;
            }
            else if (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_X)
            {
                out.movementValX = -1.0f;
            }

            if (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
            {
                out.yawVal = 0.03f;
            }
            else if (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
            {
                out.yawVal = -0.03f;
            }

            if (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)
            {
                out.pitchVal = 0.03f;
            }
            else if (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
            {
                out.pitchVal = -0.03f;
            }

            if (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
            {
                out.movementValX *= TurboSpeed;
                out.movementValY *= TurboSpeed;
                out.movementValZ *= TurboSpeed;
            }
            else if (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
            {
                out.movementValX *= SuperTurboSpeed;
                out.movementValY *= SuperTurboSpeed;
                out.movementValZ *= SuperTurboSpeed;
            }

            return out;
        }

        bool GetToggleKeyState()
        {
            bool bToggleState = (GetAsyncKeyState(vkToggle) & 0x8000) || (xiState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);
            if (bToggleState != bToggleOldState)
            {
                if (bToggleState)
                {
                    bToggleOldState = bToggleState;
                    return true;
                }
            }
            bToggleOldState = bToggleState;
            return false;
        }

        void UpdateState()
        {
            ZeroMemory(&xiState, sizeof(XINPUT_STATE));
            bControllerPresent = XInputGetState(XInputPort, &xiState) == ERROR_SUCCESS;
            if (!bControllerPresent)
                return;

            xiState.Gamepad.sThumbLX = calculateAxisValue(xiState.Gamepad.sThumbLX, xiDeadzoneLS);
            xiState.Gamepad.sThumbLY = calculateAxisValue(xiState.Gamepad.sThumbLY, xiDeadzoneLS);
            xiState.Gamepad.sThumbRX = calculateAxisValue(xiState.Gamepad.sThumbRX, xiDeadzoneRS);
            xiState.Gamepad.sThumbRY = calculateAxisValue(xiState.Gamepad.sThumbRY, xiDeadzoneRS);
        }
	}

    namespace Addresses
    {
        // #TODO add a sigscanner to support other regions/variations
        constexpr uint32_t pOnBoardCameraHook = 0x3376A8;
        constexpr uint32_t ppOnBoardCamera = 0x337488;
        constexpr uint32_t pOnBoardCamera_Entrypoint = 0x2004B8;

        constexpr uint32_t pRaceCourseDestructorHook = 0x337A38;
        constexpr uint32_t pRaceCourseDestructor_Entrypoint = 0x32DD5C;

        constexpr uint32_t pToggleDisableUpdateCodeHook = 0x337AA0;
        constexpr uint32_t pToggleDisableUpdateCode_Entrypoint = 0x1FFFD8;
        constexpr uint32_t pToggleDisableUpdate = 0x33748C;

        //
        // void Scan();
        //
    }

    // #TODO: introduce a MIPS injector instead and generate instructions in runtime... should be a lot more flexible that way.
    // Also maybe make a pnach dumper so that PCSX2 can apply it instead...
	namespace Patches
	{
        bool CheckNeedsInstalling()
        {
            constexpr uint32_t defaultValue = 0x67393251;
            uint32_t val = PCSX2::VM::Read<uint32_t>(Addresses::ppOnBoardCamera);
            return val == defaultValue;
        }

		bool Install()
		{
			if (!PCSX2::IsReady())
				return false;

            uintptr_t CodeBase = Addresses::pOnBoardCameraHook;
            uintptr_t DestructorBase = Addresses::pRaceCourseDestructorHook;
            uintptr_t ToggleCodeBase = Addresses::pToggleDisableUpdateCodeHook;

            // code for setting the OnBoardCamera's matrix pointer
            constexpr const uint32_t instructions[] =
            {
                0x3C020033,    // lui v0, 0x33 (top half of ppOnBoardCamera)
                0x24427488,    // addiu v0, v0, 0x7488 (bottom half of ppOnBoardCamera)
                0xAC440000,    // sw a0, 0(v0)
                0x08095F65,    // j 0x257D94
                0x00000000     // nop
            };

            // destructor when exiting from a race -- zeroes out the pointer
            constexpr const uint32_t instructions_destructor[] =
            {
                0x3C020033,    // lui v0, 0x33
                0x24427488,    // addiu v0, v0, 0x7488
                0xAC400000,    // sw zero, 0(v0)
                0x08063E24,    // j 0x18F890
                0x00000000     // nop
            };

            // toggle reader code
            constexpr const uint32_t instructions_toggler[] =
            {
                0x3C020033,    // lui v0, 0x33        
                0x2442748C,    // addiu v0, v0, 0x748C
                0x8C420000,    // lw v0, 0(v0)        
                0x14400003,    // bnez v0, epoint     
                0x00000000,    // nop                 
                0x08095F0D,    // j 0x257C34          
                0x00000000,    // nop                 
                0x03E00008,    // epoint: jr ra       
                0x00000000,    // nop                            
            };

            PCSX2::VM::Write<uint32_t>(Addresses::ppOnBoardCamera, 0);
            PCSX2::VM::Write<uint32_t>(Addresses::pToggleDisableUpdate, 0);

            for (int i = 0; i < _countof(instructions); i++)
            {
                PCSX2::VM::Write<uint32_t>(CodeBase, instructions[i]);
                CodeBase += sizeof(uint32_t);
            }

            for (int i = 0; i < _countof(instructions_destructor); i++)
            {
                PCSX2::VM::Write<uint32_t>(DestructorBase, instructions_destructor[i]);
                DestructorBase += sizeof(uint32_t);
            }

            for (int i = 0; i < _countof(instructions_toggler); i++)
            {
                PCSX2::VM::Write<uint32_t>(ToggleCodeBase, instructions_toggler[i]);
                ToggleCodeBase += sizeof(uint32_t);
            }

            // entrypoints
            PCSX2::VM::Write<uint32_t>(Addresses::pOnBoardCamera_Entrypoint, 0x0C0CDDAA); // jal OnBoardCameraHook
            PCSX2::VM::Write<uint32_t>(Addresses::pRaceCourseDestructor_Entrypoint, Addresses::pRaceCourseDestructorHook);
            PCSX2::VM::Write<uint32_t>(Addresses::pToggleDisableUpdateCode_Entrypoint, 0x0C0CDEA8); // jal ToggleDisableUpdateCodeHook

            return true;
		}
	}

    void SetCameraUpdateState(bool state)
    {
        if (state)
            PCSX2::VM::Write<uint32_t>(Addresses::pToggleDisableUpdate, 0);
        else
            PCSX2::VM::Write<uint32_t>(Addresses::pToggleDisableUpdate, 1);
    }

    bool GetCameraUpdateState()
    {
        uint32_t val = PCSX2::VM::Read<uint32_t>(Addresses::pToggleDisableUpdate);
        return val == 0;
    }

    void SetState(bool state)
    {
        SetCameraUpdateState(!state);
        bEnabled = !GetCameraUpdateState();
    }

    bool GetState()
    {
        return bEnabled;
    }

    uint32_t GetMatrixAddr()
    {
        constexpr uint32_t defaultValue = 0x67393251;
        uint32_t retVal = PCSX2::VM::Read<uint32_t>(Addresses::ppOnBoardCamera);

        if (retVal == defaultValue)
            retVal = 0;

        return retVal;
    }

    bool GetCameraMatrix(Matrix4& out, uint32_t inPS2Addr)
    {

        uint32_t PS2Addr = inPS2Addr;
        if (!PS2Addr)
        {
            PS2Addr = GetMatrixAddr();
            if (!PS2Addr)
                return false;
        }

        out = PCSX2::VM::Read<Matrix4>(PS2Addr);
        return true;
    }

    void SetCameraMatrix(const Matrix4& in, uint32_t inPS2Addr)
    {
        uint32_t PS2Addr = inPS2Addr;
        if (!PS2Addr)
        {
            PS2Addr = GetMatrixAddr();
            if (!PS2Addr)
                return;
        }

        PCSX2::VM::Write<Matrix4>(PS2Addr, in);
    }

    void ProcessControlPacket(const Controls::ControlPacket& pkt, Matrix4Ex& mat)
    {
        mat.m[3][0] -= pkt.movementValX;
        mat.m[3][2] += pkt.movementValZ;
        mat.m[3][1] -= pkt.movementValY;

        mat.RotateView(pkt.yawVal, pkt.pitchVal, pkt.rollVal);
    }
}