///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

ENUM_BEGIN(BackendTemplateFileType, "")
	ENUM_ITEM(Output, "This goes into the output package")
	ENUM_ITEM(InternalShader, "This is a shader used during the code gen process, but should not be part of the output package")
ENUM_END()

STRUCT_BEGIN(BackendTemplateFileProperties, "File Properties")
	STRUCT_FIELD(std::string, fileName, "", "The name of the file this is the properties for.", 0)
	STRUCT_FIELD(std::string, renameTo, "", "If not blank, what to rename the file to. May use the /*$(Name)*/ Gigi token.", 0)
	STRUCT_FIELD(bool, isDirectory, false, "If this is a directory, set this to true to make this apply to all files in the directory recursively.", 0)
	STRUCT_FIELD(bool, onlyIncludeIfRaytracing, false, "If a file should only be copied when the render graph uses raytracing, set this to true.", 0)
	STRUCT_FIELD(bool, onlyIncludeIfDX12AgilitySDKRequired, false, "If a file should only be copied when the dx12 agility sdk is required, set this to true", 0)
	STRUCT_FIELD(BackendTemplateFileType, type, BackendTemplateFileType::Output, "The type of the file determines what happens to it during code generation", 0)
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
