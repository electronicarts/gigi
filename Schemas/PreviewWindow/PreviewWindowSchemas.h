///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

//========================================================
// Bidirectional Messages
//========================================================

STRUCT_BEGIN(PreviewMsg_Ping, "The client and server send this periodically to know the other is still alive. It fails to send if the connection closes.")
STRUCT_END()

//========================================================
// Client To Server Messages
//========================================================

STRUCT_BEGIN(PreviewMsgCS_Version, "The client tells the server what Gigi version it is using")
	STRUCT_FIELD(std::string, version, "", "Gigi Version", 0)
STRUCT_END()

ENUM_BEGIN(PreviewMsgCS_Log_Level, "")
    ENUM_ITEM(Info, "")
    ENUM_ITEM(Warn, "")
    ENUM_ITEM(Error, "")
ENUM_END()

STRUCT_BEGIN(PreviewMsgCS_Log, "The client tells the server what Gigi version it is using")
	STRUCT_FIELD(PreviewMsgCS_Log_Level, level, PreviewMsgCS_Log_Level::Info, "The type of message", 0)
	STRUCT_FIELD(std::string, msg, "", "The log message", 0)
STRUCT_END()

#include "PreviewMsgClientToServerVariant.h"

//========================================================
// Server To Client Messages
//========================================================

STRUCT_BEGIN(PreviewMsgSC_VersionResponse, "The server tells the client whether or not the version is ok.")
	STRUCT_FIELD(bool, versionOK, false, "Whether the gigi version is ok or not.", 0)
STRUCT_END()

STRUCT_BEGIN(PreviewMsgSC_LoadGGFile, "The client tells the server to load a gg file")
	STRUCT_FIELD(std::string, fileName, "", "The path and name of the .gg file to load", 0)
	STRUCT_FIELD(bool, preserveState, true, "If true, camera state etc should be preserved", 0)
STRUCT_END()

#include "PreviewMsgServerToClientVariant.h"

//========================================================
// .gguser File Schema
//========================================================

STRUCT_BEGIN(GGUserFile_Audio, "")

    // Audio Settings
    STRUCT_FIELD(unsigned int, sampleRate, 44100, "The sample rate.", 0)
    STRUCT_FIELD(bool, stereo, true, "Stereo or mono.", 0)
    STRUCT_FIELD(std::string, Var_Stereo, "AudioStereo", "A boolean variable for whether audio is mono or stereo. Stereo has two floats per sample, mono has 1. Holds the value you set above. Meant to be readable by shaders.", 0)
    STRUCT_FIELD(std::string, Var_SampleRate, "AudioSampleRate", "A uint variable holding the sample rate. Holds the value you set above. Meant to be readable by shaders.", 0)

    // Audio Output
    STRUCT_FIELD(BufferNodeReference, outputBuffer, {}, "Must be an imported buffer. Gigi controls the size, and the format is always float.", 0)
    STRUCT_FIELD(float, outputVolume, 1.0f, "Output volume.", 0)
    STRUCT_FIELD(unsigned int, outputBufferLengthMs, 300, "The length of the audio output buffer in milliseconds. Smaller buffers have less latency but are more prone to starvation when there is bad perf.", 0)
    STRUCT_FIELD(std::string, Var_AudioOutSampleWindowCount, "AudioOutSampleWindowCount", "The uint variable that says how many audio samples should be generated for output. Stereo has 2 floats per sample, mono has 1. Meant to be readable by shaders", 0)

    // Audio Input
    STRUCT_FIELD(BufferNodeReference, inputBuffer, {}, "Must be an imported buffer. Gigi controls the size, and the format is always float.", 0)
    STRUCT_FIELD(float, inputVolume, 1.0f, "Input volume.", 0)
    STRUCT_FIELD(unsigned int, inputBufferLengthMs, 300, "The length of the audio input buffer in milliseconds.", 0)
    STRUCT_FIELD(std::string, Var_AudioInSampleWindowCount, "AudioInSampleWindowCount", "The uint variable that says how many audio samples should be consumed from input. Stereo has 2 floats per sample, mono has 1. Meant to be readable by shaders", 0)

STRUCT_END()

STRUCT_BEGIN(GGUserFile_WebCam, "")
    STRUCT_FIELD(TextureNodeReference, outputTexture, {}, "Must be an imported texture. Gigi controls the size and the format.", 0)
STRUCT_END()

STRUCT_BEGIN(GGUserFile_AMD_FidelityFXSDK_FrameInterpolation, "")
    STRUCT_FIELD(bool, enabled, { false }, "", 0)
    STRUCT_FIELD(unsigned int, sleepMS, 0, "The number of milliseconds to sleep every frame. Useful for artificially slowing down the rendering to see frame interpolation in action.", 0)

    STRUCT_FIELD(std::string, version, "default", "The version used", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(int, fsrUIRenderMode, 2, "UI Render Mode: 0=Standard, 1=Swapchain, 2=Callback, 3=Hudless", 0)

    // FfxApiCreateContextFramegenerationFlags
    STRUCT_FIELD(bool, ENABLE_ASYNC_WORKLOAD_SUPPORT, { true }, "", 0)
    STRUCT_FIELD(bool, ENABLE_MOTION_VECTORS_JITTER_CANCELLATION, { false }, "A bit indicating that the motion vectors have the jittering pattern applied to them.", 0)
    STRUCT_FIELD(bool, ENABLE_HIGH_DYNAMIC_RANGE, { false }, "A bit indicating if the input color data provided to all inputs is using a high-dynamic range.", 0)
    STRUCT_FIELD(bool, ENABLE_DEBUG_CHECKING, { true }, "A bit indicating that the runtime should check some API values and report issues.", 0)

    // FfxApiDispatchFramegenerationFlags
    STRUCT_FIELD(bool, DRAW_DEBUG_TEAR_LINES, { false }, "A bit indicating that the debug tear lines will be drawn to the generated output.", 0)
    STRUCT_FIELD(bool, DRAW_DEBUG_RESET_INDICATORS, { false }, "A bit indicating that the debug reset indicators will be drawn to the generated output.", 0)
    STRUCT_FIELD(bool, DRAW_DEBUG_VIEW, { false }, "A bit indicating that the generated output resource will contain debug views with relevant information.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(bool, DRAW_DEBUG_PACING_LINES, { false }, "A bit indicating that the debug pacing lines will be drawn to the generated output.", 0)

    STRUCT_FIELD(std::string, depth, "", "The depth buffer data", SCHEMA_FLAG_NO_UI)
    STRUCT_FIELD(std::string, motionVectors, "", "The motion vector data", SCHEMA_FLAG_NO_UI)
    STRUCT_FIELD(std::string, uiTexture, "", "The UI texture data", SCHEMA_FLAG_NO_UI)

    STRUCT_FIELD(bool, allowAsyncWorkloads, { true }, "Sets the state of async workloads. Set to true to enable generation work on async compute.", 0)
    STRUCT_FIELD(bool, onlyPresentGenerated, { false }, "Set to true to only present generated frames.", 0)
    STRUCT_FIELD(bool, constrainToRectangle, { false }, "If true, constrains frame generation to the texture being viewed.", 0)
STRUCT_END()

ENUM_BEGIN(GGUserFile_TLASBuildFlags, "D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE etc")
	ENUM_ITEM(None, "")
	ENUM_ITEM(AllowUpdate, "")
	ENUM_ITEM(AllowCompaction, "")
	ENUM_ITEM(PreferFastTrace, "")
	ENUM_ITEM(PreferFastBuild, "")
	ENUM_ITEM(MinimizeMemory, "")
ENUM_END()

ENUM_BEGIN(GGUserFile_BLASCullMode, "controls D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE and D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE")
	ENUM_ITEM(CullNone, "")
	ENUM_ITEM(FrontIsClockwise, "")
	ENUM_ITEM(FrontIsCounterClockwise, "")
ENUM_END()

STRUCT_BEGIN(GGUserFile_ImportedTexture, "The details of an imported texture")
	STRUCT_FIELD(std::string, fileName, "", "The image file loaded", 0)
	STRUCT_FIELD(bool, fileIsSRGB, true, "Whether the file is an sRGB file or not", 0)
	STRUCT_FIELD(bool, makeMips, false, "Whether to make mips or no", 0)
	STRUCT_STATIC_ARRAY(int, size, 3, {0 COMMA 0 COMMA 1}, "The size of the image to create", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
	STRUCT_STATIC_ARRAY(float, color, 4, { 1 COMMA 1 COMMA 1 COMMA 1}, "The color of the image to create, or the tint of the loaded file", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
	STRUCT_FIELD(TextureFormat, format, TextureFormat::RGBA8_Unorm_sRGB, "The format of the texture to create", 0)

	STRUCT_STATIC_ARRAY(int, binaryDims, 3, {0 COMMA 0 COMMA 1}, "The size of the image in the binary file", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
	STRUCT_FIELD(TextureFormat, binaryFormat, TextureFormat::RGBA8_Unorm_sRGB, "The format of the binary file", 0)
STRUCT_END()

STRUCT_BEGIN(GGUserFile_ImportedBuffer, "The details of an imported buffer")
	STRUCT_FIELD(std::string, fileName, "", "The file loaded", 0)
	STRUCT_FIELD(bool, CSVHeaderRow, true, "If reading a CSV, and this is true, it will skip everything up to the first newline, to ignore a header row.", 0)
	STRUCT_FIELD(int, structIndex, -1, "the index of the struct if a structured buffer", 0)
	STRUCT_FIELD(DataFieldType, type, DataFieldType::Count, "The data field type, if not a structured buffer", 0)
	STRUCT_FIELD(int, count, 1, "how many items are stored", 0)
	STRUCT_FIELD(GGUserFile_TLASBuildFlags, RT_BuildFlags, GGUserFile_TLASBuildFlags::PreferFastTrace, "D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE etc", 0)

	STRUCT_FIELD(bool, BLASOpaque, false, "DXR BLAS Option", 0)
	STRUCT_FIELD(bool, BLASNoDuplicateAnyhitInvocations, false, "DXR BLAS Option", 0)
	STRUCT_FIELD(GGUserFile_BLASCullMode, BLASCullMode, GGUserFile_BLASCullMode::CullNone, "BLAS triangle culling settings", 0)
	STRUCT_FIELD(bool, IsAABBs, false, "Set to true if ray tracing AABBs with intersection shaders. Format is Min XYZ, Max XYZ.", 0)

    STRUCT_STATIC_ARRAY(float, GeometryTransform, 16, { 1.0f COMMA 0.0f COMMA 0.0f COMMA 0.0f COMMA 0.0f COMMA 1.0f COMMA 0.0f COMMA 0.0f COMMA 0.0f COMMA 0.0f COMMA 1.0f COMMA 0.0f COMMA 0.0f COMMA 0.0f COMMA 0.0f COMMA 1.0f }, "A 4x4 matrix to transform pos, normal, tangent.", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)

	STRUCT_FIELD(CooperativeVectorData, cvData, {}, "Data needed for cooperative vectors support for imported buffer resources", 0)
STRUCT_END()

STRUCT_BEGIN(GGUserFile_ImportedResource, "The details of an imported resource")
	STRUCT_FIELD(std::string, nodeName, "", "The name of the node this imported texture data is for", 0)
	STRUCT_FIELD(bool, resetEveryFrame, true, "Whether this texture should be reset every frame, or if the technique is allowed to change it over multiple frames", 0)

	STRUCT_FIELD(bool, isATexture, true, "Is it a texture or a buffer", 0)
	STRUCT_FIELD(GGUserFile_ImportedTexture, texture, {}, "Texture data", 0)
	STRUCT_FIELD(GGUserFile_ImportedBuffer, buffer, {}, "Buffer data", 0)
STRUCT_END()

ENUM_BEGIN(GGUserFile_CameraJitterType, "The sequence of the jittered projection matrix")
	ENUM_ITEM(None, "No Jitter")
	ENUM_ITEM(UniformWhite, "Uniform white noise")
	ENUM_ITEM(Halton23, "Halton(2,3)")
ENUM_END()

STRUCT_BEGIN(GGUserFile_Camera, "")
	STRUCT_FIELD(bool, perspective, true, "Perspective if true, orthographic if false.", 0)
	STRUCT_FIELD(bool, leftHanded, true, "Left handed if true, right handed if false.", 0)
	STRUCT_FIELD(bool, reverseZ, true, "If true, reverses the depth values for more precision. https://developer.nvidia.com/content/depth-precision-visualized", 0)
    STRUCT_FIELD(bool, reverseZInfiniteDepth, false, "If true, reverses the depth values and has far plane at infinite z for more precision. https://developer.nvidia.com/content/depth-precision-visualized", 0)
	STRUCT_FIELD(float, nearPlane, 0.1f, "The distance to the near plane.", 0)
	STRUCT_FIELD(float, farPlane, 1000.0f, "The distance to the far plane. Set to zero for infinite Z.", 0)
	STRUCT_FIELD(float, FOV, 45.0f, "Vertical field of view, in degrees", 0)
	STRUCT_FIELD(float, flySpeed, 0.1f, "How fast the WASD keys move in the world", 0)
	STRUCT_FIELD(float, mouseSensitivity, 0.01f, "How fast the mouse rotates the camera", 0)

	STRUCT_FIELD(GGUserFile_CameraJitterType, jitterType, GGUserFile_CameraJitterType::Halton23, "The sequence of the jittered projection matrix", 0)
	STRUCT_FIELD(int, jitterLength, 16, "The length of the sequence used to jitter the jittered projection matrix. 0 means infinite.", 0)

	STRUCT_STATIC_ARRAY(float, startingCameraPos, 3, { 0.0f COMMA 0.0f COMMA -10.0f }, "", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
	STRUCT_STATIC_ARRAY(float, startingCameraAltitudeAzimuth, 2, { 0.0f COMMA 0.0f }, "", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)

	STRUCT_STATIC_ARRAY(float, cameraPos, 3, {0.0f COMMA 0.0f COMMA -10.0f}, "", SCHEMA_FLAG_NO_SERIALIZE)
	STRUCT_STATIC_ARRAY(float, cameraAltitudeAzimuth, 2, {0.0f COMMA 0.0f}, "", SCHEMA_FLAG_NO_SERIALIZE)
	STRUCT_FIELD(bool, cameraChanged, false, "", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(GGUserFile_SystemVars, "")
	STRUCT_FIELD(std::string, iResolution_varName, "iResolution", "", 0)
	STRUCT_FIELD(std::string, iResolution_textureName, "", "", 0)
	STRUCT_FIELD(std::string, iTime_varName, "iTime", "", 0)
	STRUCT_FIELD(std::string, iTimeDelta_varName, "iTimeDelta", "", 0)
	STRUCT_FIELD(std::string, iFrameRate_varName, "iFrameRate", "", 0)
	STRUCT_FIELD(std::string, iFrame_varName, "iFrame", "", 0)
	STRUCT_FIELD(std::string, iMouse_varName, "iMouse", "", 0)
	STRUCT_FIELD(std::string, MouseState_varName, "MouseState", "", 0)
	STRUCT_FIELD(std::string, MouseStateLastFrame_varName, "MouseStateLastFrame", "", 0)
	STRUCT_FIELD(std::string, WindowSize_varName, "WindowSize", "", 0)

	STRUCT_FIELD(GGUserFile_Camera, camera, {}, "The camera settings. Can pluralize if one isn't enough.", 0)
	STRUCT_FIELD(std::string, ProjMtx_textureName, "", "The projection matrix needs a resolution to calculate for. Choose a texture node and that will be used as a resolution.", 0)
	STRUCT_FIELD(std::string, ViewMtx_varName, "ViewMtx", "", 0)
	STRUCT_FIELD(std::string, InvViewMtx_varName, "InvViewMtx", "", 0)
	STRUCT_FIELD(std::string, ProjMtx_varName, "ProjMtx", "", 0)
	STRUCT_FIELD(std::string, InvProjMtx_varName, "InvProjMtx", "", 0)
	STRUCT_FIELD(std::string, ViewProjMtx_varName, "ViewProjMtx", "", 0)
	STRUCT_FIELD(std::string, InvViewProjMtx_varName, "InvViewProjMtx", "", 0)
	STRUCT_FIELD(std::string, JitteredProjMtx_varName, "JitteredProjMtx", "ProjMtx with jitter.", 0)
	STRUCT_FIELD(std::string, InvJitteredProjMtx_varName, "InvJitteredProjMtx", "Inverted ProjMtx with jitter.", 0)
	STRUCT_FIELD(std::string, JitteredViewProjMtx_varName, "JitteredViewProjMtx", "ViewProjMtx with jitter.", 0)
	STRUCT_FIELD(std::string, InvJitteredViewProjMtx_varName, "InvJitteredViewProjMtx", "Inverted ViewProjMtx with jitter.", 0)
	STRUCT_FIELD(std::string, CameraPos_varName, "CameraPos", "Camera position in world space", 0)
    STRUCT_FIELD(std::string, CameraUp_varName, "CameraUp", "Normalized camera up vector in world space", 0)
    STRUCT_FIELD(std::string, CameraLeft_varName, "CameraLeft", "Normalized camera left vector in world space", 0)
    STRUCT_FIELD(std::string, CameraForward_varName, "CameraForward", "Normalized camera forward vector in world space", 0)
	STRUCT_FIELD(std::string, CameraAltitudeAzimuth_varName, "CameraAltitudeAzimuth", "", 0)
	STRUCT_FIELD(std::string, CameraChanged_varName, "CameraChanged", "", 0)
	STRUCT_FIELD(std::string, CameraJitter_varName, "CameraJitter", "", 0)
    STRUCT_FIELD(std::string, CameraJitterRaw_varName, "CameraJitterRaw", "", 0)
	STRUCT_FIELD(std::string, CameraFOV_varName, "CameraFOV", "", 0)
	STRUCT_FIELD(std::string, CameraNearPlane_varName, "CameraNearPlane", "", 0)
	STRUCT_FIELD(std::string, CameraFarPlane_varName, "CameraFarPlane", "", 0)
	STRUCT_FIELD(std::string, ShadingRateImageTileSize_varName, "ShadingRateImageTileSize", "", 0)
	STRUCT_FIELD(std::string, KeyState_bufferName, "", "256 entries for the keys this frame, and 256 entries for the keys last frame. 512 entries total.", 0)
    STRUCT_FIELD(float, motionVectorDepthThreshold, 0.1f, "Maximum allowed difference between depth values when comparing pixels for motion vector inlier selection.", 0)
STRUCT_END()

STRUCT_BEGIN(GGUserFile_SavedVariable, "Saved Variable Values")
	STRUCT_FIELD(std::string, name, "", "", 0)
	STRUCT_FIELD(std::string, value, "", "", 0)
STRUCT_END()

STRUCT_BEGIN(GGUserFileV1, "The contents of a .gguser file")
	STRUCT_FIELD(std::string, version, "1.0", "The version of the .gguser file", SCHEMA_FLAG_SERIALIZE_DFLT)
	STRUCT_FIELD(GGUserFile_SystemVars, systemVars, {}, "", 0)
	STRUCT_FIELD(int, resourceViewType, 0, "The type of resource being viewed", 0)
	STRUCT_FIELD(int, resourceViewNodeIndex, -1, "The index of the node bieng viewed", 0)
	STRUCT_FIELD(int, resourceViewResourceIndex, -1, "The index of that resource within that node being used", 0)
	STRUCT_FIELD(int, syncInterval, 1, "IDXGISwapChain::Present() parameter: Synchronize presentation after the nth vertical blank.", 0)
	STRUCT_DYNAMIC_ARRAY(GGUserFile_ImportedResource, importedResources, "", 0)
	STRUCT_DYNAMIC_ARRAY(GGUserFile_SavedVariable, savedVariables, "", 0)
STRUCT_END()

STRUCT_BEGIN(GGUserFileV2Snapshot, "The snapshot of a GGUserFileV2")
	STRUCT_FIELD(std::string, name, "", "The snapshot name", 0)
	STRUCT_FIELD(int, resourceViewType, 0, "The type of resource being viewed", 0)
	STRUCT_FIELD(int, resourceViewNodeIndex, -1, "The index of the node bieng viewed", 0)
	STRUCT_FIELD(int, resourceViewResourceIndex, -1, "The index of that resource within that node being used", 0)

	STRUCT_FIELD(bool, loadVars, true, "Whether variables will be loaded from this snapshot", 0)
	STRUCT_FIELD(bool, loadCamera, true, "Whether the camera will be loaded from this snapshot", 0)
	STRUCT_FIELD(bool, loadResources, true, "Whether imported resources will be loaded from this snapshot", 0)
	STRUCT_FIELD(bool, loadView, true, "Whether the resource viewed will be loaded from this snapshot", 0)

	STRUCT_STATIC_ARRAY(float, cameraPos, 3, { 0.0f COMMA 0.0f COMMA - 10.0f }, "Used by snapshots to capture the camera position", 0)
	STRUCT_STATIC_ARRAY(float, cameraAltitudeAzimuth, 2, { 0.0f COMMA 0.0f }, "Used by snapshots to capture the camera orientation", 0)
	STRUCT_DYNAMIC_ARRAY(GGUserFile_ImportedResource, importedResources, "", 0)
	STRUCT_DYNAMIC_ARRAY(GGUserFile_SavedVariable, savedVariables, "", 0)
STRUCT_END()

STRUCT_BEGIN(GGUserFileV2, "The contents of a .gguser file")
	STRUCT_FIELD(std::string, version, "2.0", "The version of the .gguser file", SCHEMA_FLAG_SERIALIZE_DFLT)
	STRUCT_FIELD(int, syncInterval, 1, "IDXGISwapChain::Present() parameter: Synchronize presentation after the nth vertical blank.", 0)
	STRUCT_FIELD(GGUserFile_SystemVars, systemVars, {}, "", 0)
	STRUCT_FIELD(GGUserFileV2Snapshot, snapshot, {}, "", 0)
	STRUCT_DYNAMIC_ARRAY(GGUserFileV2Snapshot, snapshots, "", 0)

    STRUCT_FIELD(GGUserFile_AMD_FidelityFXSDK_FrameInterpolation, AMDFrameInterpolation, {}, "", 0)
    STRUCT_FIELD(GGUserFile_Audio, Audio, {}, "", 0)
    STRUCT_FIELD(GGUserFile_WebCam, WebCam, {}, "", 0)
STRUCT_END()

STRUCT_BEGIN(GGUserFileVersionOnly, "Only the version of the .gguser file")
	STRUCT_FIELD(std::string, version, "1.0", "The version of the .gguser file", SCHEMA_FLAG_SERIALIZE_DFLT)
STRUCT_END()

STRUCT_BEGIN(GGViewerConfig, "The config file for the viewer settings file ViewerConfig.json")
    STRUCT_FIELD(std::string, version, "1.0", "The version of the file", SCHEMA_FLAG_SERIALIZE_DFLT)
    STRUCT_FIELD(std::string, keyCameraForward, "W", "Camera forward key", 0)
    STRUCT_FIELD(std::string, keyCameraLeft, "A", "Camera left key", 0)
    STRUCT_FIELD(std::string, keyCameraBackward, "S", "Camera back key", 0)
    STRUCT_FIELD(std::string, keyCameraRight, "D", "Camera right key", 0)
    STRUCT_FIELD(std::string, keyCameraUp, "E", "Camera right key", 0)
    STRUCT_FIELD(std::string, keyCameraDown, "Q", "Camera down key", 0)
    STRUCT_FIELD(std::string, keyCameraFast, "Shift", "Camera fast key", 0)
    STRUCT_FIELD(std::string, keyCameraSlow, "Control", "Camera slow key", 0)
    STRUCT_FIELD(bool, betterShaderError, false, "Double click shader errors with an attached debugger.", 0)
STRUCT_END()

#define GGUserFileLatest GGUserFileV2
