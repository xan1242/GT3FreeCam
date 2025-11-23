#pragma once
#include "stdafx.h"
#include <string>
#include <cmath>
#include <cstring>
#include <algorithm>

#ifndef MATRIX4_HPP
#define MATRIX4_HPP

class Matrix4
{
public:
    float m[4][4]; // row-major
};

class Matrix4Ex : public Matrix4
{
public:
    float yaw = 0.0f; 
    float pitch = 0.0f;
    float roll = 0.0f;

    Matrix4Ex();
    void Identity();
    void Reset();

    // Load matrix from game memory
    void Load(const float* addr);

    // Write matrix back to game memory
    void Store(float* addr) const;

    // Right-multiply: this = this * rhs
    void Mul(const Matrix4Ex& rhs);

    // Rotation around X axis (pitch)
    static Matrix4Ex RotX(float angle);

    // Rotation around Y axis (yaw)
    static Matrix4Ex RotY(float angle);

    // Rotation around Z axis (roll)
    static Matrix4Ex RotZ(float angle);

    // Apply yaw/pitch to the view matrix
    void RotateView(float yawDelta, float pitchDelta, float rollDelta);

    void Print(char* buffer);
};

#endif