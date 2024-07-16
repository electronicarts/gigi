///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>

// Recent open file list, Data is stored in the Windows registry
// at the specifies location.
// A size limit is implemented in AddEntry().

class RecentFiles
{
	// most recent entries ones are in the front
	std::string m_registryAppName;

public:
	// access in UI code to present the data
	std::vector<std::string> m_Entries;

	// @param registryAppName e.g. "Software\\MyAppName"
	// the data is stored in "HKEY_CURRENT_USER\\Software\\MyAppName"
	RecentFiles(const char* registryAppName);

	// load from registry
	void LoadAllEntries();
	// save to registry
	void SaveAllEntries();

	// @param fileName must not be 0
	void AddEntry(const char* fileName);
};