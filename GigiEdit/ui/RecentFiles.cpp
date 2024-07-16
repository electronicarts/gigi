///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "RecentFiles.h"
#include <assert.h>
#include <windows.h>

// @return "" if failed
static std::string getRegKeyValue(HKEY hkey, const char* key)
{
	assert(key);

	DWORD type = 0;
	DWORD size = 0;
	LSTATUS res = RegQueryValueExA(hkey, (LPCSTR)key, nullptr, &type, nullptr, &size);
	if (res != ERROR_SUCCESS || type != REG_SZ)
		return {};

	std::string ret;

	ret.resize(size - 1);

	res = RegQueryValueExA(hkey, (LPCSTR)key, nullptr, &type, (LPBYTE)ret.data(), &size);
	if (res != ERROR_SUCCESS)
		return {};

	return ret;
}

RecentFiles::RecentFiles(const char* registryAppName)
	: m_registryAppName(registryAppName)
{
}

void RecentFiles::LoadAllEntries()
{
	m_Entries.clear();

	HKEY hkey = {};

	LSTATUS res = RegOpenKeyExA(HKEY_CURRENT_USER, (LPCSTR)m_registryAppName.c_str(), 0, KEY_READ, &hkey);
	if (res != ERROR_SUCCESS)
		return;

	for (uint32_t i = 0;;)
	{
		char key[80];
		sprintf_s(key, sizeof(key), "MRU_%d", i++);

		std::string value = getRegKeyValue(hkey, key);
		if (value.empty())
			break;

		m_Entries.push_back(value);
	}

	RegCloseKey(hkey);
}

void RecentFiles::SaveAllEntries()
{
	HKEY hkey = {};

	DWORD dwDisposition;
	LSTATUS res = RegCreateKeyExA(HKEY_CURRENT_USER, (LPCSTR)m_registryAppName.c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hkey, &dwDisposition);
	assert(res == ERROR_SUCCESS);
	if (res != ERROR_SUCCESS)
		return;

	res = RegOpenKeyExA(HKEY_CURRENT_USER, (LPCSTR)m_registryAppName.c_str(), 0, KEY_WRITE, &hkey);
	assert(res == ERROR_SUCCESS);
	if (res != ERROR_SUCCESS)
		return;

	uint32_t i = 0;
	for (const auto& el : m_Entries)
	{
		char key[80];
		sprintf_s(key, sizeof(key), "MRU_%d", i++);

		const char* value = el.c_str();
		res = RegSetValueExA(hkey, (LPCSTR)key, 0, REG_SZ, (const BYTE*)value, (DWORD)strlen(value) + 1);
		assert(res == ERROR_SUCCESS);
		if (res != ERROR_SUCCESS)
			break;
	}

	RegCloseKey(hkey);
}

void RecentFiles::AddEntry(const char* fileName)
{
	assert(fileName);

	// always erase first
	auto it = std::find(m_Entries.begin(), m_Entries.end(), std::string(fileName));
	if (it != m_Entries.end())
		m_Entries.erase(it);

	// and add at front to reshuffle based on last usage
	m_Entries.insert(m_Entries.begin(), fileName);
	// Save right away so if app crashes / gets killed / quits, we have the data stored.
	SaveAllEntries();

	// this could be exposed
	const size_t maxSize = 10;

	// limit size assuming the important data is in the front
	if (m_Entries.size() > maxSize)
	{
		m_Entries.resize(maxSize);
	}
}