///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "external/sqlite/sqlite3.h"
#include <vector>
#include <string>
#include <unordered_set>
#include <filesystem>

// Note:
// SQLite is threadsafe in a serialized way (internal mutex) by default : https://www.sqlite.org/threadsafe.html
// SQLite supports transactions too if needed: https://www.sqlite.org/lang_transaction.html

class Database
{
public:
	static bool Open(const std::filesystem::path& dbFileName, std::unordered_set<std::string>& techniquesThatWereAlreadyDownloaded);
	static void Close();

public:
	template <typename LAMBDA>
	static bool ExecuteSQL(const char* text, const std::vector<std::string>& textParams, const LAMBDA& lambda)
	{
		// prepare the sql statement
		bool ret = true;
		sqlite3_stmt* statement = nullptr;
		const char* tail = nullptr;
		int result = sqlite3_prepare_v2(s_dataBase, text, (int)strlen(text), &statement, &tail);
		if (result != SQLITE_OK)
			return false;

		// Bind the text parameters (to properly escape them etc)
		{
			int paramIndex = 0; // it is 1 based
			for (const std::string& s : textParams)
			{
				paramIndex++;
				result = sqlite3_bind_text(statement, paramIndex, s.c_str(), (int)s.length(), nullptr);
				if (result != SQLITE_OK)
					return false;
			}
		}

		// Call the lambda for each result row
		while (1)
		{
			result = sqlite3_step(statement);

			if (result == SQLITE_ROW)
			{
				lambda(statement);
			}
			else
			{
				ret = (result == SQLITE_DONE);
				break;
			}
		}

		// finalize the statement and return whether we were sucessful
		result = sqlite3_finalize(statement);
		return ret && result == SQLITE_OK;
	}

	template <typename LAMBDA>
	static bool ExecuteSQL(const char* text, const LAMBDA& lambda)
	{
		std::vector<std::string> textParams;
		return ExecuteSQL(text, textParams, lambda);
	}

	static bool ExecuteSQL(const char* text)
	{
		return ExecuteSQL(text, [](sqlite3_stmt* statement) {});
	}

	static bool ExecuteSQL(const char* text, const std::vector<std::string>& textParams)
	{
		return ExecuteSQL(text, textParams, [](sqlite3_stmt* statement) {});
	}

private:
	static inline sqlite3* s_dataBase = nullptr;
};
