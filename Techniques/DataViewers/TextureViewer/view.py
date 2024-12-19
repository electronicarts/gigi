import Host
import GigiArray
import sys
import os

#Host.Print("Argc: " + str(len(sys.argv)))
#Host.Print("Argv: " + str(sys.argv))

Host.LoadGG("TextureViewer.gg")

fileName = sys.argv[0]
extension = os.path.splitext(fileName)[1]

Host.SetImportedTextureFile("Input", fileName)

if extension == ".hdr" or extension == ".exr":
	Host.SetImportedTextureSourceIsSRGB("Input", False)
	Host.SetImportedTextureFormat("Input", Host.TextureFormat_RGBA32_Float)
	Host.SetVariable("sRGB", "false")
else:
	Host.SetImportedTextureSourceIsSRGB("Input", True)
	Host.SetImportedTextureFormat("Input", Host.TextureFormat_RGBA8_Unorm_sRGB)
	Host.SetVariable("sRGB", "true")
