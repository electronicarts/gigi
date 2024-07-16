Run MakeSolution.bat to create the solution for this code using cmake

This code plugs the module generated code into the win32/dx12 imgui example code.
Changes to main.cpp have been marked with comments "Gigi Modification Begin" and "Gigi Modification End"

Look for comments starting with "// TODO: " to see where to put one time initialization to
give input variable gives and imported resources, and similar.

Check out the unit tests in _GeneratedCode/UnitTests/DX12/main.cpp to see examples of how
to load vertex buffers from disk, create TLAS for raytracing, and similar.
