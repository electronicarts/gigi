@echo off

rem Run this with...
rem 1) the folder that holds the TnT folder (for code)
rem 2) the folder that holds the Raw folder (for assets)
rem example: .\install.bat d:\dev\devna\ d:\dev\devna\DataSets\ExampleData\

IF "%~1" == "" GOTO NoDir1
IF "%~2" == "" GOTO NoDir2

if not exist %1TnT\ GOTO NoTnT
if not exist %2Raw\ GOTO NoRaw

if not exist %1TnT\Code\Engine\World\Render\Public\Engine.World.Render\MultipleUVMesh\ mkdir %1TnT\Code\Engine\World\Render\Public\Engine.World.Render\MultipleUVMesh\
copy MultipleUVMeshRenderPassModule.h %1TnT\Code\Engine\World\Render\Public\Engine.World.Render\MultipleUVMesh\
copy MultipleUVMeshShaderPrograms.ddf %1TnT\Code\Engine\World\Render\Public\Engine.World.Render\MultipleUVMesh\

if not exist %1TnT\Code\Engine\World\Render\MultipleUVMesh\ mkdir %1TnT\Code\Engine\World\Render\MultipleUVMesh\
copy MultipleUVMeshRenderPassModule.cpp %1TnT\Code\Engine\World\Render\MultipleUVMesh\

if not exist %1TnT\Code\Engine\World\Pipeline\MultipleUVMesh\ mkdir %1TnT\Code\Engine\World\Pipeline\MultipleUVMesh\
copy MultipleUVMeshShaderProgramDefinitions.cpp %1TnT\Code\Engine\World\Pipeline\MultipleUVMesh\
copy MultipleUVMeshShaderProgramDefinitions.h %1TnT\Code\Engine\World\Pipeline\MultipleUVMesh\

if not exist %1TnT\Code\Engine\World\Render\sourcedata\source\MultipleUVMesh\ mkdir %1TnT\Code\Engine\World\Render\sourcedata\source\MultipleUVMesh\
robocopy .\shaders\ %1TnT\Code\Engine\World\Render\sourcedata\source\MultipleUVMesh\ /MIR

if not exist assets\ GOTO NoAssets
if Not exist %2Raw\MultipleUVMesh\ mkdir %2Raw\MultipleUVMesh\
copy assets\*.* %2Raw\MultipleUVMesh\
:NoAssets

GOTO EOF

:NoTnT
echo Error: Could not find the TnT folder at %1
echo Usage: install.bat ^<Code^> ^<Data^>
echo   where ^<Code^> cotains the TnT folder and ^<Data^> contains the Raw folder
echo example: .\install.bat d:\dev\devna\ d:\dev\devna\DataSets\ExampleData\
exit /b 1

:NoRaw
echo Error: Could not find the Raw folder at %2
echo Usage: install.bat ^<Code^> ^<Data^>
echo   where ^<Code^> cotains the TnT folder and ^<Data^> contains the Raw folder
echo example: .\install.bat d:\dev\devna\ d:\dev\devna\DataSets\ExampleData\
exit /b 2

:NoDir1
echo Error: No Code Directory Given!
echo Usage: install.bat ^<Code^> ^<Data^>
echo   where ^<Code^> cotains the TnT folder and ^<Data^> contains the Raw folder
echo example: .\install.bat d:\dev\devna\ d:\dev\devna\DataSets\ExampleData\
exit /b 3

:NoDir2
echo Error: No Data Directory Given!
echo Usage: install.bat ^<Code^> ^<Data^>
echo   where ^<Code^> cotains the TnT folder and ^<Data^> contains the Raw folder
echo example: .\install.bat d:\dev\devna\ d:\dev\devna\DataSets\ExampleData\
exit /b 4

:EOF
echo Success!