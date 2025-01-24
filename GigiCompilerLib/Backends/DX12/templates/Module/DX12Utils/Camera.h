/*$(CopyrightHeader)*/#pragma once

#include <stdint.h>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>

class Camera
{
public:
	// key states are indexed by capitol letters 'A' and VK_ constants like VK_LEFT
	// Mouse state is (mouseX, mouseY, leftButton, rightButton)
	void Update(const uint8_t g_keyStates[256], const float mouseState[4], const float mouseStateLastFrame[4], float frameTimeSeconds, float pos[3], float altitudeAzimuth[2], bool& cameraChanged);

	DirectX::XMMATRIX GetViewMatrix(const float pos[3], const float altitudeAzimuth[2]) const;
	DirectX::XMMATRIX GetProjMatrix(float fovDegrees, const float resolution[2], float nearZ, float farZ, bool reverseZ, bool perspective) const;

	float m_flySpeed = 1.0f;
	bool m_leftHanded = false;
	float m_mouseSensitivity = 1.0f;
};
