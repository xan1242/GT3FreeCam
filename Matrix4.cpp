#include "Matrix4.hpp"

Matrix4Ex::Matrix4Ex()
{
	Reset();
}

void Matrix4Ex::Identity()
{
	memset(m, 0, sizeof(m));
	m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
}

void Matrix4Ex::Reset()
{
    Identity();
    yaw = 0.0f;
    pitch = 0.0f;
    roll = 0.0f;
}

void Matrix4Ex::Load(const float* addr)
{
    memcpy(m, addr, sizeof(m));
}

void Matrix4Ex::Store(float* addr) const
{
    memcpy(addr, m, sizeof(m));
}

void Matrix4Ex::Mul(const Matrix4Ex& rhs)
{
    Matrix4Ex out;

    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            out.m[r][c] =
                m[r][0] * rhs.m[0][c] +
                m[r][1] * rhs.m[1][c] +
                m[r][2] * rhs.m[2][c] +
                m[r][3] * rhs.m[3][c];
        }
    }

    memcpy(m, out.m, sizeof(out.m));
}

Matrix4Ex Matrix4Ex::RotX(float angle)
{
    Matrix4Ex r;
    float c = cosf(angle);
    float s = sinf(angle);

    r.m[1][1] = c;
    r.m[1][2] = -s;
    r.m[2][1] = s;
    r.m[2][2] = c;

    return r;
}

Matrix4Ex Matrix4Ex::RotY(float angle)
{
    Matrix4Ex r;
    float c = cosf(angle);
    float s = sinf(angle);

    r.m[0][0] = c;
    r.m[0][2] = s;
    r.m[2][0] = -s;
    r.m[2][2] = c;

    return r;
}

Matrix4Ex Matrix4Ex::RotZ(float angle)
{
    Matrix4Ex r;
    float c = cosf(angle);
    float s = sinf(angle);

    r.m[0][0] = c;
    r.m[0][1] = s;
    r.m[1][0] = -s;
    r.m[1][1] = c;

    return r;
}

void Matrix4Ex::RotateView(float yawDelta, float pitchDelta, float rollDelta)
{
    if (yawDelta)
    {
        yaw += yawDelta;
        Mul(RotX(-pitch));
        Mul(RotY(yawDelta));
        Mul(RotX(pitch));
        
    }

    if (pitchDelta)
    {
        pitch += pitchDelta;
        Mul(RotX(pitchDelta));
    }

    if (rollDelta)
    {
        roll += rollDelta;
        Mul(RotZ(rollDelta));
    }
}
