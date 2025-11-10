///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

//clang-format off
#include "GigiCompilerLib/gigicompiler.h"

#include "Schemas/HTML.h"
#include "Schemas/JSONSchema.h"
//clang-format on

#ifdef _DEBUG
    #define BUILD_FLAVOR() "Debug"
#else
    #define BUILD_FLAVOR() "Release"
#endif

void WriteViewerPythonTypes(const char* fileName)
{
    FILE* file = nullptr;
    fopen_s(&file, fileName, "wb");
    if (!file)
        return;

    // Enum FromString / ToString functions
    fprintf(file, "========== Enum To / From String functions ==========\n\n");
    #include "external/df_serialize/_common.h"
    #define ENUM_BEGIN(_NAME, _DESCRIPTION) fprintf(file, "  " #_NAME "FromString()\n  " #_NAME "ToString()\n\n");
    #include "external/df_serialize/_fillunsetdefines.h"
    #include "Schemas/Schemas.h"

    // Enum values
    fprintf(file, "========== Enum Values ==========\n\n");
    #include "external/df_serialize/_common.h"
    #define ENUM_BEGIN(_NAME, _DESCRIPTION) \
    { \
        const char* enumName = #_NAME; \
        fprintf(file, "  %s:\n", enumName); \
        fprintf(file, "    Host.%s_FIRST = 0\n", enumName); \
        int enumValue = 0;

    #define ENUM_ITEM(_NAME, _DESCRIPTION) \
        fprintf(file, "    Host.%s_" #_NAME " = %i\n", enumName, enumValue); \
        enumValue++;

    #define ENUM_END() \
        fprintf(file, "    Host.%s_LAST = %i\n", enumName, enumValue - 1); \
        fprintf(file, "    Host.%s_COUNT = %i\n", enumName, enumValue); \
        fprintf(file, "\n"); \
    }
    #include "external/df_serialize/_fillunsetdefines.h"
    #include "Schemas/Schemas.h"

    fclose(file);
}

int main(int argc, char** argv)
{
    // Write out the most up to date help file and json schema
    WriteHTML("gigihelp.html");
    WriteJSONSchema("gigischema.json");
    WriteViewerPythonTypes("UserDocumentation/PythonTypes.txt");

    bool GENERATE_GRAPHVIZ_FLAG = false;
    std::string graphviz_param = "-graphviz";
    if (argc == 5 && graphviz_param.compare(argv[4]) == 0)
    {
        GENERATE_GRAPHVIZ_FLAG = true;
    }
    if ((argc != 4 && argc != 5) || (argc == 5 && !GENERATE_GRAPHVIZ_FLAG))
    {
        printf("Version " GIGI_VERSION_WITH_BUILD_NUMBER() " (" BUILD_FLAVOR() ")\n");
        printf("Usage: GigiCompiler.exe <platform> <json file> <output directory>\n\nExample: GigiCompiler.exe DX12_Module Techniques/boxblur.gg ./out/ [-graphviz]\n\n");
        printf("Backends Supported:\n");
        #include "external/df_serialize/_common.h"
        #define GIGI_BUILD_FLAVOR(BACKEND, FLAVOR, INTERNAL) if(!INTERNAL) { printf("  " #BACKEND "_" #FLAVOR "\n"); }
        #include "GigiCompilerLib/GigiBuildFlavorList.h"
        #undef GIGI_BUILD_FLAVOR
        printf("\n");
        printf("optional -graphviz enables generation of GraphViz graphs & images for gigi project visualization\n");
        printf("\n");
        return (int)GigiCompileResult::WrongParams;
    }


    // Get the build flavor
    GigiBuildFlavor buildFlavor;
    if (!StringToEnum(argv[1], buildFlavor))
    {
        GigiAssert(false, "Could not find build flavor '%s'", argv[1]);
        return (int)GigiCompileResult::NoBackend;
    }

    // Get the backend
    Backend backend;
    if (!GigiBuildFlavorBackend(buildFlavor, backend))
    {
        GigiAssert(false, "Could not get backend for build flavor '%s'", argv[1]);
        return (int)GigiCompileResult::NoBackend;
    }

    void (*PostLoad)(RenderGraph&) = nullptr;
    switch (backend)
    {
        #include "external/df_serialize/_common.h"
        #define ENUM_ITEM(x, y) case Backend::x: PostLoad = PostLoad_##x; break;
        // clang-format off
        #include "external/df_serialize/_fillunsetdefines.h"
        #include "Schemas/BackendList.h"
        // clang-format on
    }

    return (int)GigiCompile(buildFlavor, argv[2], argv[3], PostLoad, nullptr, GENERATE_GRAPHVIZ_FLAG);
}
