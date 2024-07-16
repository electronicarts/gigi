///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GigiCompilerLib/Utils.h"
#include <vector>
#include <string>
#include <stdint.h>

struct GigiArray
{
	std::vector<char> data;
	std::vector<int64_t> dims;
	std::vector<size_t> strides;
	std::string formatString;  // https://docs.python.org/3/library/struct.html#module-struct
	size_t itemSize = 0;
};

class PythonInterface
{
public:
	struct ProfilingData
	{
		std::string label;
		float cpums = 0.0f;
		float gpums = 0.0f;
	};

	virtual bool LoadGG(const char* fileName) = 0;
	virtual void RequestExit(int exitCode) = 0;
	virtual void SetHideUI(bool set) = 0;
	virtual void SetVSync(bool set) = 0;
	virtual void SetSyncInterval(int syncInterval) = 0;
	virtual void SetStablePowerState(bool set) = 0;
	virtual void SetProfilingMode(bool set) = 0;
	virtual bool SetVariable(const char* varName, const char* varValue) = 0;
	virtual bool GetVariable(const char* varName, std::string& value) = 0;
	virtual void SetDisableGGUserSave(bool set) = 0;
	virtual void SetWantReadback(const char* viewableResourceName, bool wantsReadback, int arrayIndex, int mipIndex) = 0;
	virtual bool Readback(const char* viewableResourceName, GigiArray& data) = 0;
	virtual void RunTechnique(int runCount) = 0;
	virtual void SetFrameIndex(int frameIndex) = 0;
	virtual void WaitOnGPU() = 0;
	virtual void Pause(bool pause) = 0;
	virtual void PixCaptureNextFrames(const char* fileName, int frameCount) = 0;
	virtual void SetImportedBufferCSVHeaderRow(const char* bufferName, bool CSVHeaderRow) = 0;
	virtual void SetImportedBufferCount(const char* bufferName, int count) = 0;
	virtual void SetImportedBufferFile(const char* bufferName, const char* fileName) = 0;
	virtual void SetImportedBufferStruct(const char* bufferName, const char* structName) = 0;
	virtual void SetImportedBufferType(const char* bufferName, DataFieldType type) = 0;
	virtual void SetImportedTextureFile(const char* textureName, const char* fileName) = 0;
	virtual void SetImportedTextureSourceIsSRGB(const char* textureName, bool sourceIsSRGB) = 0;
	virtual void SetImportedTextureMakeMips(const char* textureName, bool makeMips) = 0;
	virtual void SetImportedTextureFormat(const char* textureName, int textureFormat) = 0;
	virtual void SetImportedTextureColor(const char* textureName, float R, float G, float B, float A) = 0;
	virtual void SetImportedTextureSize(const char* textureName, int x, int y, int z) = 0;
	virtual void SetImportedTextureBinaryType(const char* textureName, GGUserFile_ImportedTexture_BinaryType type) = 0;
	virtual void SetImportedTextureBinarySize(const char* textureName, int x, int y, int z) = 0;
	virtual void SetImportedTextureBinaryChannels(const char* textureName, int channels) = 0;
	virtual void SetFrameDeltaTime(float seconds) = 0;
	virtual void SetCameraPos(float X, float Y, float Z) = 0;
	virtual void SetCameraAltitudeAzimuth(float altitude, float azimuth) = 0;
	virtual void GetCameraPos(float &X, float &Y, float &Z) = 0;
	virtual void GetCameraAltitudeAzimuth(float &altitude, float &azimuth) = 0;
	virtual void WriteGPUResource(const char* viewableResourceName, int subresourceIndex, const char* data, size_t size) = 0;
	virtual void ForceEnableProfiling(bool forceEnable) = 0;
	virtual std::vector<ProfilingData> GetProfilingData() = 0;

	virtual void Log(LogLevel level, const char* msg, ...) = 0;
	virtual void OnExecuteFinished() = 0;

	virtual int GGEnumValue(const char* enumName, const char* enumLabel) = 0;
	virtual std::string GGEnumLabel(const char* enumName, int value) = 0;
	virtual int GGEnumCount(const char* enumName) = 0;

	virtual std::string GetGPUString() = 0;

	virtual std::string GetScriptLocation() = 0;

	// The location and name of the python script
	std::string m_scriptLocation;
};

void PythonInit(PythonInterface* i, int argc, char** argv, int firstPythonArgv);
void PythonShutdown();

bool PythonExecute(const char* fileName);
