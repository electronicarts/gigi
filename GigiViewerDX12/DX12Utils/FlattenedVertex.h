///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

typedef std::array<float, 1> Vec1;
typedef std::array<float, 2> Vec2;
typedef std::array<float, 3> Vec3;
typedef std::array<float, 4> Vec4;

struct FlattenedVertex
{
	Vec3 position;
	Vec3 normal;
	Vec4 tangent;
	Vec4 albedo;
	Vec2 uvs[4]; // FBX supports up to 4 uvs
	int materialID = -1;
	int shapeIndex = -1; // mesh index in FBX. shape index in OBJ.
};

template <size_t N>
std::array<float, N> operator - (const std::array<float, N>& A, const std::array<float, N>& B)
{
	std::array<float, N> ret;
	for (size_t i = 0; i < N; ++i)
		ret[i] = A[i] - B[i];
	return ret;
}

template <size_t N>
std::array<float, N> operator * (const std::array<float, N>& A, float B)
{
	std::array<float, N> ret;
	for (size_t i = 0; i < N; ++i)
		ret[i] = A[i] * B;
	return ret;
}

template <size_t N>
std::array<float, N> operator += (std::array<float, N>& A, const std::array<float, N>& B)
{
	for (size_t i = 0; i < N; ++i)
		A[i] += B[i];
	return A;
}

template <size_t N>
float Dot(const std::array<float, N>& A, const std::array<float, N>& B)
{
	float ret = 0.0f;
	for (size_t i = 0; i < N; ++i)
		ret += A[i] * B[i];
	return ret;
}

template <size_t N>
std::array<float, N> Normalize(const std::array<float, N>& A)
{
	float len = std::sqrt(Dot(A, A));
	std::array<float, N> ret;
	for (size_t i = 0; i < N; ++i)
		ret[i] = A[i] / len;
	return ret;
}

inline Vec3 Cross(const Vec3& A, const Vec3& B)
{
	return Vec3
	{
		A[1] * B[2] - A[2] * B[1],
		-(A[0] * B[2] - A[2] * B[0]),
		A[0] * B[1] - A[1] * B[0]
	};
}
