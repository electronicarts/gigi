import Host
import GigiArray
import sys
import os

#Host.Print("Argc: " + str(len(sys.argv)))
#Host.Print("Argv: " + str(sys.argv))

def DoIt():
	Host.LoadGG("TextureViewer_dds.gg")

	fileName = sys.argv[0]

	Host.SetImportedTextureFile("Input2D", fileName)
	Host.SetImportedTextureFile("Input3D", fileName)

	# Try as BC7
	Host.SetImportedTextureSourceIsSRGB("Input2D", True)
	Host.SetImportedTextureSourceIsSRGB("Input3D", True)
	Host.SetImportedTextureFormat("Input2D", Host.TextureFormat_BC7_Unorm_sRGB)
	Host.SetImportedTextureFormat("Input3D", Host.TextureFormat_BC7_Unorm_sRGB)
	Host.RunTechnique(2)
	Host.WaitOnGPU()

	if Host.IsResourceCreated("Input2D"):
		Host.SetViewedResource("Input2D.resource - Initial State")
		return

	if Host.IsResourceCreated("Input3D"):
		Host.SetViewedResource("Input3D.resource - Initial State")
		return

	# Try as BC6
	Host.SetImportedTextureSourceIsSRGB("Input2D", False)
	Host.SetImportedTextureSourceIsSRGB("Input3D", False)
	Host.SetImportedTextureFormat("Input2D", Host.TextureFormat_BC6_UF16)
	Host.SetImportedTextureFormat("Input3D", Host.TextureFormat_BC6_UF16)
	Host.RunTechnique(2)
	Host.WaitOnGPU()

	if Host.IsResourceCreated("Input2D"):
		Host.SetViewedResource("Input2D.resource - Initial State")
		return

	if Host.IsResourceCreated("Input3D"):
		Host.SetViewedResource("Input3D.resource - Initial State")
		return

	# Try as BC4
	Host.SetImportedTextureSourceIsSRGB("Input2D", False)
	Host.SetImportedTextureSourceIsSRGB("Input3D", False)
	Host.SetImportedTextureFormat("Input2D", Host.TextureFormat_BC4_Unorm)
	Host.SetImportedTextureFormat("Input3D", Host.TextureFormat_BC4_Unorm)
	Host.RunTechnique(2)
	Host.WaitOnGPU()

	if Host.IsResourceCreated("Input2D"):
		Host.SetViewedResource("Input2D.resource - Initial State")
		return

	if Host.IsResourceCreated("Input3D"):
		Host.SetViewedResource("Input3D.resource - Initial State")
		return

	# Try as BC5
	Host.SetImportedTextureSourceIsSRGB("Input2D", False)
	Host.SetImportedTextureSourceIsSRGB("Input3D", False)
	Host.SetImportedTextureFormat("Input2D", Host.TextureFormat_BC5_Unorm)
	Host.SetImportedTextureFormat("Input3D", Host.TextureFormat_BC5_Unorm)
	Host.RunTechnique(2)
	Host.WaitOnGPU()

	if Host.IsResourceCreated("Input2D"):
		Host.SetViewedResource("Input2D.resource - Initial State")
		return

	if Host.IsResourceCreated("Input3D"):
		Host.SetViewedResource("Input3D.resource - Initial State")
		return

DoIt()
