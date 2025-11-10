@rem ///////////////////////////////////////////////////////////////////////////////
@rem //         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
@rem //        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
@rem ///////////////////////////////////////////////////////////////////////////////

"./GigiViewerDX12/python/Python310/python.exe" MakeCode_UnitTests_DX12.py
cd _GeneratedCode/UnitTests/DX12/
rem rmdir private /s /q
rem rmdir public /s /q
call MakeSolution.bat
