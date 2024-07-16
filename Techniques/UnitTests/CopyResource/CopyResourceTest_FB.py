import Host
import GigiArray
import numpy
import os

resources = [
	"Copy_Texture_To_Texture.Dest (After)",
	"Copy_Buffer_To_Buffer.Dest (After)",
]

# don't save gguser files during this script execution
Host.DisableGGUserSave(True)

def DoTest():
	TestPassed = True

	# make sure the output directory exists
	outDirName = "Techniques/UnitTests/_GoldImages/CopyResource/CopyResourceTest_FB/"
	os.makedirs(outDirName, exist_ok=True)

	# Load the technique
	if not Host.LoadGG("Techniques/UnitTests/CopyResource/CopyResourceTest_FB.gg"):
		return False

	# Specify the resources we want to readback
	for resource in resources:
		Host.SetWantReadback(resource)

	# Do one execution to ensure everything is initialized
	Host.RunTechnique()

	# Get the results and compare
	Host.RunTechnique()
	Host.WaitOnGPU()
	for i, resource in enumerate(resources):
		lastReadback, success = Host.Readback(resource)
		if success:
			lastReadbackNp = numpy.array(lastReadback)
			outFileName = outDirName + str(i) + ".npy"
			if os.path.exists(outFileName):
				img = numpy.load(outFileName)
				if not numpy.array_equal(img, lastReadbackNp):
					Host.Log("Error", outFileName + " did not match")
					TestPassed = False
			else:
				Host.Log("Error", outFileName + " didn't exist, creating")
				numpy.save(outFileName, lastReadbackNp)
				lastReadbackNp.tofile(outDirName + str(i) + ".bin", "")
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
