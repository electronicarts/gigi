@echo off

rem Run this with the root folder of the unreal engine - the folder where GenerateProjectFiles.bat lives

rem example: .\install.bat D:\dev\github\UnrealEngine\

IF "%~1" == "" GOTO NoDir1

copy public\VariableAliasesTechnique.h %1Engine\Source\Runtime\Renderer\Public\

copy private\VariableAliasesTechnique.cpp %1Engine\Source\Runtime\Renderer\Private\

if not exist %1Engine\Shaders\Private\VariableAliases\ mkdir %1Engine\Shaders\Private\VariableAliases\
copy shaders\*.* %1Engine\Shaders\Private\VariableAliases\

GOTO EOF

:NoDir1
echo Error: No Unreal Engine Directory Given!
echo Usage: install.bat ^<Root^>
echo   where ^<Root^> cotains GenerateProjectFiles.bat
echo example: .\install.bat D:\dev\github\UnrealEngine\
exit /b 1

:EOF
echo Success!