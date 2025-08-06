DXC v1.8.2505 (May 2025)
downloaded from https://github.com/microsoft/DirectXShaderCompiler/releases
only x64 folder kept from lib and bin, the other platforms were deleted.

dxcompiler.dll and dxil.dll also were copied from bin/x64 into the root folder.

The folder in the DX12 code gen template also needs to be updated. These 2 folders should match:
external\DXC\
GigiCompilerLib\Backends\DX12\templates\Module\DXC\