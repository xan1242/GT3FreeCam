#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _USE_MATH_DEFINES
#include <windows.h>
#include <iostream>

inline constexpr const char* AppTitle = "-=-=-=-= Gran Turismo 3 Free Camera =-=-=-=-";

inline constexpr const char* ControlsText = "\
-=-= KEYBOARD CONTROLS =-=-\n\
Bksp to toggle | WASD to move | IJKL to look around\n\
Spc/Ctrl to Up/Down | LShift/F for Turbo/SuperTurbo\n\
U/O to tilt camera counter-clockwise/clockwise\n\
R to reset all rotations\n\n\
-=-= GAMEPAD CONTROLS =-=-\n\
NOTE: Port 2 ONLY!\n\
Back/View to toggle | LS to move | RS/Dpad to look\n\
LT/RT to Up/Down | LB/RB for Turbo/SuperTurbo\n\
LT+Start/RT+Start to tilt | B+Start to Reset rot. \
";
