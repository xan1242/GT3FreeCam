#pragma once
#include "stdafx.h"
#include <vector>
#include <string>

#undef DrawText

#ifndef CONSOLEBUFFER_HPP
#define CONSOLEBUFFER_HPP

class ConsoleBuffer
{
public:
    ConsoleBuffer(short width, short height)
        : width(width), height(height)
    {
        buffer.resize(width * height);

        hConsole = CreateConsoleScreenBuffer(
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            CONSOLE_TEXTMODE_BUFFER,
            NULL);

        SetConsoleActiveScreenBuffer(hConsole);
    }

    HANDLE GetHandle() const {
        return hConsole;
    }

    void Clear(CHAR fill = ' ')
    {
        for (auto& c : buffer) {
            c.Char.AsciiChar = fill;
            c.Attributes = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE;
        }
    }

    int DrawText(short x, short y, const std::string& text, WORD attr = 7)
    {
        int numLines = 1;
        if (y < 0 || y >= height) return 0;

        short xcounter = 0;
        for (size_t i = 0; i < text.size(); ++i)
        {
            short tx = x + (short)xcounter;
            xcounter++;
            if (tx < 0 || tx >= width) continue;

            if (text[i] == '\n')
            {
                numLines++;
                y++;
                xcounter = 0;
                continue;
            }

            buffer[y * width + tx].Char.AsciiChar = text[i];
            buffer[y * width + tx].Attributes = attr;
        }

        return numLines;
    }

    void Render()
    {
        SMALL_RECT rect = { 0, 0, width - 1, height - 1 };
        COORD size = { width, height };
        COORD zero = { 0, 0 };

        WriteConsoleOutputA(
            hConsole,
            buffer.data(),
            size,
            zero,
            &rect);
    }

private:
    HANDLE hConsole;
    short width, height;
    std::vector<CHAR_INFO> buffer;
};


#endif