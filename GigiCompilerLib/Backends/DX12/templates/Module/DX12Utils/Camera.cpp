/*$(CopyrightHeader)*/#include "Camera.h"

#include <algorithm>
#include <cmath>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static const float c_pi = 3.14159265359f;

void Camera::Update(const uint8_t g_keyStates[256], const float mouseState[4], const float mouseStateLastFrame[4], float frameTimeSeconds, float pos[3], float altitudeAzimuth[2], bool& cameraChanged)
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
        altitudeAzimuth[0] = std::max(std::min(altitudeAzimuth[0], c_pi * 0.95f), -c_pi * 0.95f);

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
        if (g_keyStates[VK_SHIFT])
            flySpeed *= 10;
        else if (g_keyStates[VK_CONTROL])
            flySpeed /= 10;

        if (!m_leftHanded)
        {
            fwd[0] *= -1.0f;
            fwd[1] *= -1.0f;
            fwd[2] *= -1.0f;
        }

        if (g_keyStates['W'] || g_keyStates[VK_UP])
        {
            pos[0] += fwd[0] * flySpeed;
            pos[1] += fwd[1] * flySpeed;
            pos[2] += fwd[2] * flySpeed;
            cameraChanged = true;
        }

        if (g_keyStates['S'] || g_keyStates[VK_DOWN])
        {
            pos[0] -= fwd[0] * flySpeed;
            pos[1] -= fwd[1] * flySpeed;
            pos[2] -= fwd[2] * flySpeed;
            cameraChanged = true;
        }

        if (g_keyStates['A'] || g_keyStates[VK_LEFT])
        {
            pos[0] -= left[0] * flySpeed;
            pos[1] -= left[1] * flySpeed;
            pos[2] -= left[2] * flySpeed;
            cameraChanged = true;
        }

        if (g_keyStates['D'] || g_keyStates[VK_RIGHT])
        {
            pos[0] += left[0] * flySpeed;
            pos[1] += left[1] * flySpeed;
            pos[2] += left[2] * flySpeed;
            cameraChanged = true;
        }

        if (g_keyStates['E'] || g_keyStates[VK_PRIOR])
        {
            pos[0] += up[0] * flySpeed;
            pos[1] += up[1] * flySpeed;
            pos[2] += up[2] * flySpeed;
            cameraChanged = true;
        }

        if (g_keyStates['Q'] || g_keyStates[VK_NEXT])
        {
            pos[0] -= up[0] * flySpeed;
            pos[1] -= up[1] * flySpeed;
            pos[2] -= up[2] * flySpeed;
            cameraChanged = true;
        }
    }
}

DirectX::XMMATRIX Camera::GetViewMatrix(const float pos[3], const float altitudeAzimuth[2]) const
{
    float altitude = altitudeAzimuth[0];
    float azimuth = altitudeAzimuth[1];
    DirectX::XMMATRIX rot = DirectX::XMMatrixRotationY(azimuth) * DirectX::XMMatrixRotationX(altitude);
    return DirectX::XMMatrixTranslation(-pos[0], -pos[1], -pos[2]) * rot;
}

DirectX::XMMATRIX Camera::GetProjMatrix(float fovDegrees, const float resolution[2], float nearZ, float farZ, bool reverseZ, bool perspective) const
{
    float fov = fovDegrees * c_pi / 180.0f;

    nearZ = std::max(nearZ, 0.001f);
    farZ = std::max(farZ, 0.001f);
    fov = std::max(fov, 0.001f);

    if (nearZ == farZ)
        farZ = nearZ + 0.01f;

    if (reverseZ)
        std::swap(nearZ, farZ);

    if (perspective)
    {
        if (m_leftHanded)
            return DirectX::XMMatrixPerspectiveFovLH(fov, resolution[0] / resolution[1], nearZ, farZ);
        else
            return DirectX::XMMatrixPerspectiveFovRH(fov, resolution[0] / resolution[1], nearZ, farZ);
    }
    else
    {
        if (m_leftHanded)
            return DirectX::XMMatrixOrthographicLH(resolution[0], resolution[1], nearZ, farZ);
        else
            return DirectX::XMMatrixOrthographicRH(resolution[0], resolution[1], nearZ, farZ);
    }
}