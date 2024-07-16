rem ///////////////////////////////////////////////////////////////////////////////
rem //         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
rem //        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
rem ///////////////////////////////////////////////////////////////////////////////

echo [Gigi] Build RELEASE

"C:\Program Files\Microsoft Visual Studio\2022\Professional\Msbuild\Current\Bin\MSBuild.exe" gigi.sln /property:Configuration=Release
