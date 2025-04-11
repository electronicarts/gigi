///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "gigiinterpreter.h"
#include "ParseCSV.h"

void VariableStorage::SetFromString(const char* text, size_t count, int* value)
{
	ParseCSV::ForEachValue(text, false,
		[&] (int tokenIndex, const char* token)
		{
			if (tokenIndex < count)
				sscanf_s(token, "%i", &value[tokenIndex]);
			return true;
		}
	);
}

void VariableStorage::SetFromString(const char* text, size_t count, unsigned int* value)
{
	ParseCSV::ForEachValue(text, false,
		[&](int tokenIndex, const char* token)
		{
			if (tokenIndex < count)
				sscanf_s(token, "%u", &value[tokenIndex]);
			return true;
		}
	);
}

void VariableStorage::SetFromString(const char* text, size_t count, float* value)
{
	ParseCSV::ForEachValue(text, false,
		[&](int tokenIndex, const char* token)
		{
			if (tokenIndex < count)
				sscanf_s(token, "%f", &value[tokenIndex]);
			return true;
		}
	);
}

void VariableStorage::SetFromString(const char* text, size_t count, bool* value)
{
	ParseCSV::ForEachValue(text, false,
		[&](int tokenIndex, const char* token)
		{
			if (tokenIndex < count)
			{
				if (!_stricmp(token, "true") || !_stricmp(token, "1"))
					value[tokenIndex] = true;
				else if (!_stricmp(token, "false") || !_stricmp(token, "0"))
					value[tokenIndex] = false;
				else
					return false;
			}
			return true;
		}
	);
}

void VariableStorage::SetFromString(const char* text, size_t count, uint16_t* value)
{
	ParseCSV::ForEachValue(text, false,
		[&](int tokenIndex, const char* token)
		{
			if (tokenIndex < count)
			{
				unsigned int temp;
				sscanf_s(token, "%u", &temp);
				value[tokenIndex] = temp;
			}
			return true;
		}
	);
}

void VariableStorage::SetFromString(const char* text, size_t count, int64_t* value)
{
	ParseCSV::ForEachValue(text, false,
		[&](int tokenIndex, const char* token)
		{
			if (tokenIndex < count)
			{
				int64_t temp;
				sscanf_s(token, "%lli", &temp);
				value[tokenIndex] = temp;
			}
			return true;
		}
	);
}

void VariableStorage::SetFromString(const char* text, size_t count, uint64_t* value)
{
	ParseCSV::ForEachValue(text, false,
		[&](int tokenIndex, const char* token)
		{
			if (tokenIndex < count)
			{
				uint64_t temp;
				sscanf_s(token, "%llu", &temp);
				value[tokenIndex] = temp;
			}
			return true;
		}
	);
}

std::string VariableStorage::GetAsString(size_t count, int* value)
{
	std::string ret;

	char buffer[256];
	for (size_t i = 0; i < count; ++i)
	{
		sprintf_s(buffer, "%s%i", ((i > 0) ? "," : ""), value[i]);
		ret += std::string(buffer);
	}

	return ret;
}

std::string VariableStorage::GetAsString(size_t count, unsigned int* value)
{
	std::string ret;

	char buffer[256];
	for (size_t i = 0; i < count; ++i)
	{
		sprintf_s(buffer, "%s%u", ((i > 0) ? "," : ""), value[i]);
		ret += std::string(buffer);
	}

	return ret;
}

std::string VariableStorage::GetAsString(size_t count, float* value)
{
	std::string ret;

	char buffer[256];
	for (size_t i = 0; i < count; ++i)
	{
		sprintf_s(buffer, "%s%f", ((i > 0) ? "," : ""), value[i]);
		ret += std::string(buffer);
	}

	return ret;
}

std::string VariableStorage::GetAsString(size_t count, bool* value)
{
	std::string ret;

	char buffer[256];
	for (size_t i = 0; i < count; ++i)
	{
		sprintf_s(buffer, "%s%s", ((i > 0) ? "," : ""), (value[i] ? "true" : "false"));
		ret += std::string(buffer);
	}

	return ret;
}

std::string VariableStorage::GetAsString(size_t count, uint16_t* value)
{
	std::string ret;

	char buffer[256];
	for (size_t i = 0; i < count; ++i)
	{
		sprintf_s(buffer, "%s%u", ((i > 0) ? "," : ""), (unsigned int)value[i]);
		ret += std::string(buffer);
	}

	return ret;
}

std::string VariableStorage::GetAsString(size_t count, int64_t* value)
{
	std::string ret;

	char buffer[256];
	for (size_t i = 0; i < count; ++i)
	{
		sprintf_s(buffer, "%s%lli", ((i > 0) ? "," : ""), (int64_t)value[i]);
		ret += std::string(buffer);
	}

	return ret;
}

std::string VariableStorage::GetAsString(size_t count, uint64_t* value)
{
	std::string ret;

	char buffer[256];
	for (size_t i = 0; i < count; ++i)
	{
		sprintf_s(buffer, "%s%llu", ((i > 0) ? "," : ""), (uint64_t)value[i]);
		ret += std::string(buffer);
	}

	return ret;
}

