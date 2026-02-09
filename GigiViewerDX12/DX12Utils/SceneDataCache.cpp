///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2026 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "SceneDataCache.h"

#include <filesystem>

SceneData& SceneDataCache::Get(FileCache& fileCache, const char* fileName_)
{
    // normalize the string by making it canonical and making it lower case
    std::string s = std::filesystem::weakly_canonical(fileName_).string();
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    const char* fileName = s.c_str();

    if (m_cache.count(fileName) != 0)
        return m_cache[fileName];

    SceneData& sceneData = m_cache[fileName];

    FileCache::File fileData = fileCache.Get(fileName);
    if (!fileData.Valid())
    {
        sceneData.valid = false;
        return sceneData;
    }

    bool success = false;
    std::string extension = std::filesystem::path(s).extension().string();
    if (extension == ".fbx")
        success = LoadFBX(fileData, sceneData);
    if (extension == ".obj")
        success = LoadOBJ(fileData, sceneData);
    if (extension == ".gltf" || extension == ".glb")
        success = LoadGLTF(fileData, sceneData);

    sceneData.filename = fileName;
    sceneData.valid = success;
    return sceneData;
}