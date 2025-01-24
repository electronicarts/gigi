import Host
import GigiArray
import numpy
from PIL import Image
import os

resource = "ComputeShader.tex: Texture (UAV - After)"

DontTestTheseFormats = [
	Host.TextureFormat_Any,
	Host.TextureFormat_D32_Float_S8,
	Host.TextureFormat_D24_Unorm_S8,
	Host.TextureFormat_BC4_Unorm,
	Host.TextureFormat_BC4_Snorm,
	Host.TextureFormat_BC5_Unorm,
	Host.TextureFormat_BC5_Snorm,
	Host.TextureFormat_BC7_Unorm,
	Host.TextureFormat_BC7_Unorm,
	Host.TextureFormat_BC7_Unorm_sRGB,
	Host.TextureFormat_BC6_UF16,
	Host.TextureFormat_BC6_SF16,
]

# don't save gguser files during this script execution
Host.DisableGGUserSave(True)

def DoTest():
	TestPassed = True

	# make sure the output directory exists
	outDirName = "Techniques/UnitTests/_GoldImages/Textures/TextureFormats/"
	os.makedirs(outDirName, exist_ok=True)

	# Load the technique
	if not Host.LoadGG("Techniques/UnitTests/Textures/TextureFormats.gg"):
		return False

	# Specify the resource we want to readback
	Host.SetWantReadback(resource)

	# For each texture format type
	for textureFormatIndex in range(Host.TextureFormat_FIRST, Host.TextureFormat_COUNT):

		if (textureFormatIndex in DontTestTheseFormats):
			continue

		# Set the format type
		Host.SetImportedTextureFormat("Texture", textureFormatIndex)

		# Run once to make sure everything is initialize, then run again to make the technique execute, then wait on the GPU to be finished
		Host.RunTechnique(2)
		Host.WaitOnGPU()

		# Get results and compare
		lastReadback, success = Host.Readback(resource)
		if success:
			lastReadbackNp = numpy.array(lastReadback)
			outFileName = outDirName + Host.TextureFormatToString(textureFormatIndex) + ".npy"

			#Host.Log("Warn", str(lastReadbackNp.shape) + " " + Host.TextureFormatToString(textureFormatIndex) + " : " + str(lastReadbackNp[0,0,0,:]))

			if os.path.exists(outFileName):
				img = numpy.load(outFileName)
				if not numpy.array_equal(img, lastReadbackNp):
					Host.Log("Error", outFileName + " did not match")
					TestPassed = False
			else:
				Host.Log("Error", outFileName + " didn't exist, creating")
				numpy.save(outFileName, lastReadbackNp)
				lastReadbackNp.tofile(outDirName + Host.TextureFormatToString(textureFormatIndex) + ".bin", "")
				TestPassed = False
		else:
			Host.Log("Error", "Could not readback " + resource)
			TestPassed = False

	return TestPassed

# This is so the test can be ran by itself directly
if __name__ == "builtins":
	if DoTest():
		Host.Log("Info", "test Passed")
	else:
		Host.Log("Error", "Test Failed")
