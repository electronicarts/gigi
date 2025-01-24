///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

// Info about a technique server
STRUCT_BEGIN(BrowserServerInfo, "")
	STRUCT_FIELD(std::string, Name, "", "", 0)
	STRUCT_FIELD(std::string, Repo, "", "", 0)
	STRUCT_FIELD(std::string, Branch, "", "", 0)
	STRUCT_FIELD(std::string, TechniqueList, "", "", 0)
STRUCT_END()

// An entry in a technique server technique list
STRUCT_BEGIN(BrowserTechniqueSummary, "")
	STRUCT_FIELD(std::string, Repo, "", "The repo. Usually begins with https and ends with .git.", 0)
	STRUCT_FIELD(std::string, Commit, "", "The commit hash to look at in that repo.", 0)
	STRUCT_FIELD(std::string, DetailsFile, "", "The path of the json file containing BrowserTechniqueDetails of the technique. This location is recursively downloaded when the download button is pressed.", 0)
	STRUCT_FIELD(std::string, ListedDate, "", "A date in the form YYYY-MM-DD", 0)
STRUCT_END()

// An entire technique server technique list
STRUCT_BEGIN(BrowserTechniqueSummaries, "")
	STRUCT_DYNAMIC_ARRAY(BrowserTechniqueSummary, Techniques, "", 0)
STRUCT_END()

// The contents of the "meta" file.
// This lives along side the technique files, including license, screenshot, .gg file, shaders, assets, etc.
STRUCT_BEGIN(BrowserTechniqueDetails, "")
	STRUCT_FIELD(std::string, Title, "", "", 0)
	STRUCT_FIELD(std::string, Description, "", "", 0)
	STRUCT_FIELD(std::string, Author, "", "", 0)
	STRUCT_FIELD(std::string, Tags, "", "Separated by commas", 0)
	STRUCT_FIELD(std::string, Technique, "The .gg file to open. Relative to DetailsFile.", "", 0)
	STRUCT_FIELD(std::string, Website, "", "[Optional] A website to view and/or discuss your technique. For instance, the github page.", 0)
	STRUCT_FIELD(std::string, Screenshot, "", "The image shown that goes with your technique. Relative to DetailsFile.", 0)
	STRUCT_FIELD(std::string, License, "", "The file describing the license of your technique. Relative to DetailsFile.", 0)
	STRUCT_FIELD(std::string, GigiVersion, "", "The version of Gigi used to make this technique.", 0)
STRUCT_END()

// An entry in the local technique cache
STRUCT_BEGIN(BrowserCachedTechnique, "")
	STRUCT_FIELD(BrowserTechniqueSummary, Summary, {}, "", 0)
	STRUCT_FIELD(BrowserTechniqueDetails, Details, {}, "", 0)
	STRUCT_FIELD(bool, Downloaded, false, "", 0)
	STRUCT_FIELD(BrowserServerInfo, Origin, {}, "", 0)
STRUCT_END()

// The entire local technique cache
STRUCT_BEGIN(BrowserCachedTechniques, "")
	STRUCT_DYNAMIC_ARRAY(BrowserCachedTechnique, Techniques, "", 0)
STRUCT_END()