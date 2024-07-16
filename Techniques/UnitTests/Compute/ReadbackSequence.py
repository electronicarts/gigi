import Host
import GigiArray
import numpy
from PIL import Image
import os

# don't save gguser files during this script execution
Host.DisableGGUserSave(True)

def DoTest():
	TestPassed = True

	# make sure the output directory exists
	outDirName = "Techniques/UnitTests/_GoldImages/Compute/ReadbackSequence/"
	os.makedirs(outDirName, exist_ok=True)

	# Load the technique
	if not Host.LoadGG("Techniques/UnitTests/Compute/ReadbackSequence.gg"):
		return False

	# Specify the resources we want to readback
	Host.SetWantReadback("Node_1.output: Output (UAV - After)")

	# Do one execution to ensure everything is initialized
	Host.RunTechnique()

	# Now do our readback, starting at frame index 0
	# Write the file out if it doesn't exist, else verify that the 
	Host.SetFrameIndex(0)
	for i in range(10):
		Host.RunTechnique()
		Host.WaitOnGPU()
		lastReadback, success = Host.Readback("Node_1.output: Output (UAV - After)")
		if success:
			lastReadbackNp = numpy.array(lastReadback)
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
			Host.Log("Error", str(i) + ": Could not readback")
			TestPassed = False
	return TestPassed

# This is so the test can be ran by itself directly
if __name__ == "builtins":
	if DoTest():
		Host.Log("Info", "test Passed")
	else:
		Host.Log("Error", "Test Failed")
