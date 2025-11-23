//
// Gran Turismo 3 Free Camera
// A PCSX2 wrapper/trainer that modifies the memory for a free camera
//
// This is made as a quick 'n' dirty tool. Do not expect quality code.
//

//
// COMPATIBILITY:
// Currently only compatible with NTSC-U v1.00 SCUS-97102
//

//
// TODO:
// - print out the projection matrix -- not _really_ done, it's left commented out for now... try at your own risk :P
// - dynamic UI toggle when free cam is turned on
// - figure out if there's a way to get better math, maybe by the means of a world matrix so we can reset the transforms? -- partially done, we can reset the rotations...
// - configuration file...
// - sigscanning and support for other versions/variants
// - ...and other todos scattered around the sources...
//

#include "stdafx.h"
#include <commdlg.h>
#include <string>
#include <thread>
#include <filesystem>
#include "PCSX2.hpp"
#include "Matrix4.hpp"
#include "FreeCam.hpp"
#include "ConsoleBuffer.hpp"

bool bDestroyedFlag = false;

Matrix4Ex matFreeCam;
Matrix4Ex matWorld;
Matrix4Ex matProj;
char FormatBuffer[128];

std::filesystem::path GetExecutablePathInteractive()
{
    OPENFILENAMEW ofn = { 0 };
    wchar_t filePath[MAX_PATH] = { 0 };

    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = L"PCSX2 Executable (*.exe)\0*.exe\0All Files (*.*)\0*.*\0";
    ofn.lpstrTitle = L"Please select your PCSX2 executable...";
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameW(&ofn))
        return std::filesystem::path(filePath);

    return "";
}

void BlockUntilGameLoaded()
{
    // #TODO add sigscanning and remove fixed stuff for different game variations...
    constexpr uint32_t sigIns1 = 0x3C040035;
    constexpr uint32_t sigIns2 = 0x24840B50;

    uint32_t ins1 = 0;
    uint32_t ins2 = 0;

    while (((ins1 != sigIns1) || (ins2 != sigIns2)) && PCSX2::IsOpen())
    {
        ins1 = PCSX2::VM::Read<uint32_t>(0x102C6C);
        ins2 = PCSX2::VM::Read<uint32_t>(0x102C74);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void HandleToggleInput()
{
    if (!PCSX2::IsInFocus())
        return;

    if (FreeCam::Controls::GetToggleKeyState())
    {
        FreeCam::SetState(!FreeCam::GetState());
        if (!FreeCam::GetState())
        {
            matFreeCam.Reset();
        }
    }
}

void HandleFreeCam(Matrix4Ex& mat, uint32_t PS2Addr)
{
    if (!PCSX2::IsInFocus())
        return;

    FreeCam::ProcessControlPacket(FreeCam::Controls::GetKeyboardValues(), mat);
    FreeCam::ProcessControlPacket(FreeCam::Controls::GetControllerValues(), mat);

    // #NOTE: the rotations are broken because of the nature of this tool
    // Camera matricies are processed on VU0, and because of that, things get a little caddy-wumpus
    FreeCam::SetCameraMatrix(mat, PS2Addr);
}

int main(int argc, char* argv[])
{
    std::filesystem::path exePath;
    std::cout << AppTitle << '\n';

    if (argc > 1) {
        exePath = argv[1];
    }

    if (exePath.empty() || !std::filesystem::exists(exePath))
    {
        std::wcout << L"Please select your PCSX2 executable..." << L'\n';
        exePath = GetExecutablePathInteractive();
        if (exePath.empty()) {
            std::cerr << "No file selected.\n";
            return 1;
        }
    }

    int errCode = PCSX2::Launch(exePath);
    if (errCode != PCSX2::LaunchError_NoError)
    {
        switch (errCode)
        {
            case PCSX2::LaunchError_BaseAddressFail:
            {
                std::cerr << "Failed to get the base address. Please try again.\n";
                break;
            }
            case PCSX2::LaunchError_CreateProcessFail:
            {
                std::cerr << "Failed to create the PCSX2 process.\n";
                break;
            }
        }
        PCSX2::CloseHandles();
        return 1;
    }

    std::cout << "Process launched. PID = " << PCSX2::GetPID() << '\n';

    std::cout << "Waiting for GT3 to load...\n";
    BlockUntilGameLoaded();

    if (!PCSX2::IsOpen())
    {
        std::cout << "PCSX2 closed, closing...\n";
        PCSX2::CloseHandles();
        return 0;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Gran Turismo 3 booted, patching the game now...\n";
    
    FreeCam::Patches::Install();

    const short W = 80;
    const short H = 25;
    constexpr int StartingLine = 2;
    ConsoleBuffer screen(W, H);


    while (PCSX2::IsOpen())
    {
        screen.Clear(' ');
        screen.DrawText(0, 0, AppTitle);
        int LineCursor = StartingLine;

        if (FreeCam::Patches::CheckNeedsInstalling())
        {
            LineCursor += screen.DrawText(0, LineCursor, "Patching the game again...");
            FreeCam::Patches::Install();
        }

        uint32_t matAddr = FreeCam::GetMatrixAddr();
        if (!matAddr)
        {
            LineCursor += screen.DrawText(0, LineCursor, "Matrix unavailable or not ingame... Waiting...");
            screen.Render();

            if (!bDestroyedFlag)
            {
                FreeCam::SetState(false);
                bDestroyedFlag = true;
                matFreeCam.Reset();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
            continue;
        }

        bDestroyedFlag = false;

        FreeCam::Controls::UpdateState();
        HandleToggleInput();
        FreeCam::GetCameraMatrix(matFreeCam, matAddr);

        const char* statetxt = FreeCam::GetState() ? "ON" : "OFF";

        sprintf_s(FormatBuffer, "State: %s", statetxt);
        LineCursor += screen.DrawText(0, LineCursor, FormatBuffer);
        sprintf_s(FormatBuffer, "View translation: %.4f, %.4f, %.4f", matFreeCam.m[3][0], matFreeCam.m[3][1], matFreeCam.m[3][2]);
        LineCursor += screen.DrawText(0, LineCursor, FormatBuffer);

        uint32_t worldMatAddr = FreeCam::GetWorldMatrixAddr();
        FreeCam::GetCameraMatrix(matWorld, worldMatAddr);
        sprintf_s(FormatBuffer, "World Position: %.4f, %.4f, %.4f", matWorld.m[1][0], matWorld.m[1][1], matWorld.m[1][2]);
        LineCursor += screen.DrawText(0, LineCursor, FormatBuffer) + 1;

        //LineCursor += screen.DrawText(0, LineCursor, "-=-=-= PROJ MATRIX =-=-=-");
        //
        //uint32_t projMatAddr = FreeCam::GetProjMatrixAddr();
        //FreeCam::GetCameraMatrix(matProj, projMatAddr);
        //matProj.Print(FormatBuffer);
        //LineCursor += screen.DrawText(0, LineCursor, FormatBuffer) + 1;

        LineCursor += screen.DrawText(0, ++LineCursor, ControlsText);

        if (FreeCam::GetState())
            HandleFreeCam(matFreeCam, matAddr);

        //LineCursor += screen.DrawText(0, LineCursor, "-=-=-= VIEW MATRIX =-=-=-");
        //
        //matFreeCam.Print(FormatBuffer);
        //LineCursor += screen.DrawText(0, LineCursor, FormatBuffer) + 1;


        screen.Render();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "PCSX2 closed, closing...\n";

    PCSX2::CloseHandles();

    return 0;
}
