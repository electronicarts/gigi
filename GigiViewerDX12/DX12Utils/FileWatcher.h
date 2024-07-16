///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <unordered_map>
#include <filesystem>
#include <string>
#include <chrono>

template <typename TENTRYDATA>
class FileWatcher
{
public:
	FileWatcher(float checkIntervalSeconds)
	{
		m_lastTick = std::chrono::steady_clock::now();
		m_checkIntervalSeconds = checkIntervalSeconds;
	}

	void Add(const char* fileName_, const TENTRYDATA& data)
	{
		// normalize the string by making it canonical and making it lower case
		std::string s = std::filesystem::weakly_canonical(fileName_).string();
		std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
		const char* fileName = s.c_str();

		std::error_code ec;
		std::filesystem::file_time_type time = std::filesystem::last_write_time(fileName, ec);
		if (ec)
			return;

		EntryData entryData;
		entryData.time = time;
		entryData.data = data;

		m_trackedFiles[fileName] = entryData;
	}

	void Clear()
	{
		m_trackedFiles.clear();
	}

	template <typename LAMBDA>
	bool Tick(const LAMBDA& lambda)
	{
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::duration<float>>(now - m_lastTick).count() < m_checkIntervalSeconds)
			return false;
		m_lastTick = now;

		bool ret = false;
		for (auto& it : m_trackedFiles)
		{
			std::error_code ec;
			std::filesystem::file_time_type time = std::filesystem::last_write_time(it.first, ec);
			if (ec)
			{
				lambda(it.first, it.second.data);
				ret = true;
				m_trackedFiles.erase(it.first);
			}
			else
			{
				if (time != it.second.time)
				{
					lambda(it.first, it.second.data);
					ret = true;
					m_trackedFiles[it.first].time = time;
				}
			}
		}
		return ret;
	}

	struct EntryData
	{
		TENTRYDATA data;
		std::filesystem::file_time_type time;
	};

	std::unordered_map<std::string, EntryData> getTrackedFiles()
	{
		return m_trackedFiles;
	}

private:

	std::unordered_map<std::string, EntryData> m_trackedFiles;
	float m_checkIntervalSeconds = 0.0f;
	std::chrono::steady_clock::time_point m_lastTick;
};