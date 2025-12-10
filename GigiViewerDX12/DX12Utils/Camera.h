///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>

class Camera
{
public:

    struct KeyStates
    {
        bool forward = false;
        bool back = false;
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
        bool fast = false;
        bool slow = false;
    };

	// key states are indexed by capitol letters 'A' and VK_ constants like VK_LEFT
	// Mouse state is (mouseX, mouseY, leftButton, rightButton)
	void Update(const KeyStates& keyStates, const float mouseState[4], const float mouseStateLastFrame[4], float frameTimeSeconds, float pos[3], float altitudeAzimuth[2], bool& cameraChanged);

	DirectX::XMMATRIX GetViewMatrix(const float pos[3], const float altitudeAzimuth[2]) const;
	DirectX::XMMATRIX GetProjMatrix(float fovDegrees, const float resolution[2], float nearZ, float farZ, bool reverseZ, bool reverseZInfiniteDepth, bool perspective) const;

	float m_flySpeed = 1.0f;
	bool m_leftHanded = false;
	float m_mouseSensitivity = 1.0f;

    float m_lastPos[3] = { 0.0f, 0.0f, 0.0f };
    float m_lastAltitudeAzimuth[2] = { 0.0f, 0.0f };
};
