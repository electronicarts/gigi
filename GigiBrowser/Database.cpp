///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "Database.h"

#include "Version.h"

bool Database::Open(const std::filesystem::path& dbFileName, std::unordered_set<std::string>& techniquesThatWereAlreadyDownloaded)
{
	// Make sure the database path exists
	std::filesystem::path dbFileDir = dbFileName;
	dbFileDir.replace_filename("");
	std::filesystem::create_directories(dbFileDir);

	// Open or create the database
	int result = sqlite3_open(dbFileName.string().c_str(), &s_dataBase);
	if (result != SQLITE_OK)
		return false;

	// Create the version table if it doesn't yet exist
	if (!ExecuteSQL("create table if not exists Version(version text)"))
		return false;

	// Read the version to see if it's still valid
	std::string oldVersion;
	bool versionOK = false;
	if (!ExecuteSQL("select version from Version",
		[&versionOK, &oldVersion](sqlite3_stmt* statement)
		{
			const unsigned char* text = sqlite3_column_text(statement, 0);
			oldVersion = (const char*)text;
			versionOK = !_stricmp((const char*)text, BROWSER_DB_VERSION());
		}
	))
	{
		return false;
	}

	// If the version is wrong, we need to update it, and also recreate all the tables we care about
	if (!versionOK)
	{
		// Get the list of techniques that are marked as downloaded, so we can restore the Downloaded flag later
		if (!oldVersion.empty())
		{
			if (!ExecuteSQL("select SummaryHash from Techniques where Downloaded = 1",
				[&techniquesThatWereAlreadyDownloaded](sqlite3_stmt* statement)
				{
					techniquesThatWereAlreadyDownloaded.insert((const char*)sqlite3_column_text(statement, 0));
				}
			))
			{
				return false;
			}
		}

		static const char* CreateTechniquesTable =
			"create table Techniques("
			"SummaryHash text primary key,"
			"Summary_Repo text, Summary_Commit text, Summary_DetailsFile text, Summary_ListedDate text,"
			"Details_Title text, Details_Description text, Details_Author text, Details_Tags text, Details_Technique text, Details_Website text, Details_Screenshot text, Details_License text, Details_GigiVersion text,"
			"Downloaded integer,"
			"Origin_Name text, Origin_Repo text, Origin_Branch text, Origin_TechniqueList text)"
			;

		if (
			!ExecuteSQL("delete from Version") ||
			!ExecuteSQL("insert into Version (version) values (\"" BROWSER_DB_VERSION() "\")") ||
			!ExecuteSQL("drop table if exists Techniques") ||
			!ExecuteSQL(CreateTechniquesTable)
			)
		{
			return false;
		}
	}

	return true;
}

void Database::Close()
{
	sqlite3_close(s_dataBase);
	s_dataBase = nullptr;
}
