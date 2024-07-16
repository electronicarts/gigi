///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

STRUCT_BEGIN(BackendTemplateFileProperties, "File Properties")
	STRUCT_FIELD(std::string, fileName, "", "The name of the file this is the properties for.", 0)
	STRUCT_FIELD(std::string, renameTo, "", "If not blank, what to rename the file to. May use the /*$(Name)*/ Gigi token.", 0)
	STRUCT_FIELD(bool, onlyIncludeIfRaytracing, false, "If a file should only be copied when the render graph uses raytracing, set this to true.", 0)
STRUCT_END()

STRUCT_BEGIN(BackendTemplateNodeTemplate, "Template text for specific tags, for specific nodes")
	STRUCT_FIELD(std::string, nodeType, "", "The type of node this template is for.", 0)
	STRUCT_FIELD(std::string, tag, "", "The tag that this template is for.", 0)
	STRUCT_FIELD(std::string, text, "", "The text body of the template", 0)
STRUCT_END()

STRUCT_BEGIN(BackendTemplateConfig, "Backend Template Config")
	STRUCT_DYNAMIC_ARRAY(BackendTemplateFileProperties, fileProperties, "Properties for files in a backend template directory", 0)
	STRUCT_DYNAMIC_ARRAY(std::string, ignoreDirectories, "Do not process a directory as template files", 0)
	STRUCT_DYNAMIC_ARRAY(std::string, nodeTemplateFiles, "The files to process into node templates", 0)
	STRUCT_DYNAMIC_ARRAY(BackendTemplateNodeTemplate, nodeTemplates, "calculated at runtime, from nodeTemplateFiles", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()
