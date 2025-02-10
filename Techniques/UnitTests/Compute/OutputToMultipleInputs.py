import Host
import GigiArray
import numpy
from PIL import Image
import os

resources = [
	[ "CopyTexture.Dest (After)", True ],
	[ "CopyBuffer.Dest (After)", False ],
]

# don't save gguser files during this script execution
Host.DisableGGUserSave(True)

def DoTest():
	TestPassed = True

	# make sure the output directory exists
	outDirName = "Techniques/UnitTests/_GoldImages/Compute/OutputToMultipleInputs/"
	os.makedirs(outDirName, exist_ok=True)

	# Load the technique
	if not Host.LoadGG("Techniques/UnitTests/Compute/OutputToMultipleInputs.gg"):
		return False

	# Specify the resources we want to readback
	for resource in resources:
		Host.SetWantReadback(resource[0])

	# Do one execution to ensure everything is initialized
	Host.RunTechnique()

	# Get the result
	Host.RunTechnique()
	Host.WaitOnGPU()

	# Get the results and compare
	Host.RunTechnique()
	Host.WaitOnGPU()
	for i, resource in enumerate(resources):
		lastReadback, success = Host.Readback(resource[0])
		if success:
			lastReadbackNp = numpy.array(lastReadback)
			if resource[1]:
				lastReadbackNp = lastReadbackNp.reshape((lastReadbackNp.shape[1], lastReadbackNp.shape[2], lastReadbackNp.shape[3]))
				outFileName = outDirName + str(i) + ".png"
				if os.path.exists(outFileName):
					img = numpy.asarray(Image.open(outFileName))
					if not numpy.array_equal(img, lastReadbackNp):
						Host.Log("Error", outFileName + " did not match")
						TestPassed = False
				else:
					Host.Log("Error", outFileName + " didn't exist, creating")
					Image.fromarray(lastReadbackNp, "RGBA").save(outFileName)
					TestPassed = False
			else:
				Host.Log("Info", str(lastReadbackNp))
				outFileName = outDirName + str(i) + ".npy"
				if os.path.exists(outFileName):
					img = numpy.load(outFileName)
					if not numpy.array_equal(img, lastReadbackNp):
						Host.Log("Error", outFileName + " did not match")
						TestPassed = False
				else:
					Host.Log("Error", outFileName + " didn't exist, creating")
					numpy.save(outFileName, lastReadbackNp)
					TestPassed = False
		else:
			Host.Log("Error", "Could not readback " + resource[0])
			TestPassed = False

	return TestPassed

# This is so the test can be ran by itself directly
if __name__ == "builtins":
	if DoTest():
		Host.Log("Info", "test Passed")
	else:
		Host.Log("Error", "Test Failed")
