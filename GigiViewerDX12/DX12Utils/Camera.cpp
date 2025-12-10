///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "Camera.h"

#include <algorithm>
#include <cmath>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static const float c_pi = 3.14159265359f;

void Camera::Update(const KeyStates& keyStates, const float mouseState[4], const float mouseStateLastFrame[4], float frameTimeSeconds, float pos[3], float altitudeAzimuth[2], bool& cameraChanged)
{
    // To account for frame time
    float movementMultiplier = frameTimeSeconds * 60.0f;

    // If the mouse is dragging, rotate the camera
    if (mouseState[2] && mouseStateLastFrame[2])
    {
        float dx = mouseStateLastFrame[0] - mouseState[0];
        float dy = mouseStateLastFrame[1] - mouseState[1];

        dx *= movementMultiplier;
        dy *= movementMultiplier;

        if (!m_leftHanded)
        {
            dx *= -1.0;
            dy *= -1.0;
        }

        altitudeAzimuth[0] += dy * m_mouseSensitivity;
        altitudeAzimuth[0] = std::max(std::min(altitudeAzimuth[0], c_pi * 0.49f), -c_pi * 0.49f);

        altitudeAzimuth[1] += dx * m_mouseSensitivity;
        if (altitudeAzimuth[1] < 0.0f)
            altitudeAzimuth[1] += 2.0f * c_pi;
        altitudeAzimuth[1] = std::fmodf(altitudeAzimuth[1], 2.0f * c_pi);

        cameraChanged |= (dx != 0.0f || dy != 0.0f);
    }

    // Handle WASD
    {

        DirectX::XMMATRIX viewMatrix = GetViewMatrix(pos, altitudeAzimuth);

        float left[3] = { viewMatrix.r[0].m128_f32[0] , viewMatrix.r[1].m128_f32[0] , viewMatrix.r[2].m128_f32[0] };
        float up[3] = { viewMatrix.r[0].m128_f32[1], viewMatrix.r[1].m128_f32[1], viewMatrix.r[2].m128_f32[1] };
        float fwd[3] = { viewMatrix.r[0].m128_f32[2], viewMatrix.r[1].m128_f32[2], viewMatrix.r[2].m128_f32[2] };

        float flySpeed = m_flySpeed * movementMultiplier;
        if (keyStates.fast)
            flySpeed *= 10;
        else if (keyStates.slow)
            flySpeed /= 10;

        if (!m_leftHanded)
        {
            fwd[0] *= -1.0f;
            fwd[1] *= -1.0f;
            fwd[2] *= -1.0f;
        }

        if (keyStates.forward)
        {
            pos[0] += fwd[0] * flySpeed;
            pos[1] += fwd[1] * flySpeed;
            pos[2] += fwd[2] * flySpeed;
            cameraChanged = true;
        }

        if (keyStates.back)
        {
            pos[0] -= fwd[0] * flySpeed;
            pos[1] -= fwd[1] * flySpeed;
            pos[2] -= fwd[2] * flySpeed;
            cameraChanged = true;
        }

        if (keyStates.left)
        {
            pos[0] -= left[0] * flySpeed;
            pos[1] -= left[1] * flySpeed;
            pos[2] -= left[2] * flySpeed;
            cameraChanged = true;
        }

        if (keyStates.right)
        {
            pos[0] += left[0] * flySpeed;
            pos[1] += left[1] * flySpeed;
            pos[2] += left[2] * flySpeed;
            cameraChanged = true;
        }

        if (keyStates.up)
        {
            pos[0] += up[0] * flySpeed;
            pos[1] += up[1] * flySpeed;
            pos[2] += up[2] * flySpeed;
            cameraChanged = true;
        }

        if (keyStates.down)
        {
            pos[0] -= up[0] * flySpeed;
            pos[1] -= up[1] * flySpeed;
            pos[2] -= up[2] * flySpeed;
            cameraChanged = true;
        }
    }

    memcpy(m_lastPos, pos, sizeof(float) * 3);
    memcpy(m_lastAltitudeAzimuth, altitudeAzimuth, sizeof(float) * 2);
}

DirectX::XMMATRIX Camera::GetViewMatrix(const float pos[3], const float altitudeAzimuth[2]) const
{
    float altitude = altitudeAzimuth[0];
    float azimuth = altitudeAzimuth[1];
    DirectX::XMMATRIX rot = DirectX::XMMatrixRotationY(azimuth) * DirectX::XMMatrixRotationX(altitude);
    return DirectX::XMMatrixTranslation(-pos[0], -pos[1], -pos[2]) * rot;
}

// Adapting XMMatrixPerspectiveFovLH
static inline DirectX::XMMATRIX XM_CALLCONV XMMatrixPerspectiveFovLH_ReverseZ_InfiniteDepth(float FovAngleY, float AspectRatio, float NearZ)
{
    assert(NearZ > 0.f);
    assert(!DirectX::XMScalarNearEqual(FovAngleY, 0.0f, 0.00001f * 2.0f));
    assert(!DirectX::XMScalarNearEqual(AspectRatio, 0.0f, 0.00001f));

    float    SinFov;
    float    CosFov;
    DirectX::XMScalarSinCos(&SinFov, &CosFov, 0.5f * FovAngleY);

    float Height = CosFov / SinFov;
    float Width = Height / AspectRatio;

    DirectX::XMMATRIX M;
    M.r[0].m128_f32[0] = Width;
    M.r[0].m128_f32[1] = 0.0f;
    M.r[0].m128_f32[2] = 0.0f;
    M.r[0].m128_f32[3] = 0.0f;

    M.r[1].m128_f32[0] = 0.0f;
    M.r[1].m128_f32[1] = Height;
    M.r[1].m128_f32[2] = 0.0f;
    M.r[1].m128_f32[3] = 0.0f;

    M.r[2].m128_f32[0] = 0.0f;
    M.r[2].m128_f32[1] = 0.0f;
    M.r[2].m128_f32[2] = 0.0f;
    M.r[2].m128_f32[3] = 1.0f;

    M.r[3].m128_f32[0] = 0.0f;
    M.r[3].m128_f32[1] = 0.0f;
    M.r[3].m128_f32[2] = NearZ;
    M.r[3].m128_f32[3] = 0.0f;
    return M;
}

// Adapting XMMatrixPerspectiveFovRH
static inline DirectX::XMMATRIX XM_CALLCONV XMMatrixPerspectiveFovRH_ReverseZ_InfiniteDepth(float FovAngleY, float AspectRatio, float NearZ)
{
    assert(NearZ > 0.f);
    assert(!DirectX::XMScalarNearEqual(FovAngleY, 0.0f, 0.00001f * 2.0f));
    assert(!DirectX::XMScalarNearEqual(AspectRatio, 0.0f, 0.00001f));

    float    SinFov;
    float    CosFov;
    DirectX::XMScalarSinCos(&SinFov, &CosFov, 0.5f * FovAngleY);

    float Height = CosFov / SinFov;
    float Width = Height / AspectRatio;

    DirectX::XMMATRIX M;
    M.r[0].m128_f32[0] = Width;
    M.r[0].m128_f32[1] = 0.0f;
    M.r[0].m128_f32[2] = 0.0f;
    M.r[0].m128_f32[3] = 0.0f;

    M.r[1].m128_f32[0] = 0.0f;
    M.r[1].m128_f32[1] = Height;
    M.r[1].m128_f32[2] = 0.0f;
    M.r[1].m128_f32[3] = 0.0f;

    M.r[2].m128_f32[0] = 0.0f;
    M.r[2].m128_f32[1] = 0.0f;
    M.r[2].m128_f32[2] = 0.0f;
    M.r[2].m128_f32[3] = -1.0f;

    M.r[3].m128_f32[0] = 0.0f;
    M.r[3].m128_f32[1] = 0.0f;
    M.r[3].m128_f32[2] = NearZ;
    M.r[3].m128_f32[3] = 0.0f;
    return M;
}

DirectX::XMMATRIX Camera::GetProjMatrix(float fovDegrees, const float resolution[2], float nearZ, float farZ, bool reverseZ, bool reverseZInfiniteDepth, bool perspective) const
{
    float fov = fovDegrees * c_pi / 180.0f;

    nearZ = std::max(nearZ, 0.001f);
    farZ = std::max(farZ, 0.001f);
    fov = std::max(fov, 0.001f);

    if (nearZ == farZ)
        farZ = nearZ + 0.01f;

    if (reverseZ && !reverseZInfiniteDepth)
        std::swap(nearZ, farZ);

    if (perspective)
    {
        if (reverseZInfiniteDepth)
        {
            if (m_leftHanded)
                return XMMatrixPerspectiveFovLH_ReverseZ_InfiniteDepth(fov, resolution[0] / resolution[1], nearZ);
            else
                return XMMatrixPerspectiveFovRH_ReverseZ_InfiniteDepth(fov, resolution[0] / resolution[1], nearZ);
        }
        else
        {
            if (m_leftHanded)
                return DirectX::XMMatrixPerspectiveFovLH(fov, resolution[0] / resolution[1], nearZ, farZ);
            else
                return DirectX::XMMatrixPerspectiveFovRH(fov, resolution[0] / resolution[1], nearZ, farZ);
        }
    }
    else
    {
        if (m_leftHanded)
            return DirectX::XMMatrixOrthographicLH(resolution[0], resolution[1], nearZ, farZ);
        else
            return DirectX::XMMatrixOrthographicRH(resolution[0], resolution[1], nearZ, farZ);
    }
}