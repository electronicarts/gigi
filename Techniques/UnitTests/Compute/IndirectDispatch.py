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
	outDirName = "Techniques/UnitTests/_GoldImages/Compute/IndirectDispatch/"
	os.makedirs(outDirName, exist_ok=True)

	# Load the technique
	if not Host.LoadGG("Techniques/UnitTests/Compute/IndirectDispatch.gg"):
		return False

	# Specify the resources we want to readback
	Host.SetWantReadback("Do_Indirect_Dispatch_2.Render_Target: Render_Target (UAV - After)")

	# Do one execution to ensure everything is initialized
	Host.RunTechnique()

	# Get the result
	Host.RunTechnique()
	Host.WaitOnGPU()
	lastReadback, success = Host.Readback("Do_Indirect_Dispatch_2.Render_Target: Render_Target (UAV - After)")

	# Compare etc
	if success:
		lastReadbackNp = numpy.array(lastReadback)
		lastReadbackNp = lastReadbackNp.reshape((lastReadbackNp.shape[1], lastReadbackNp.shape[2], lastReadbackNp.shape[3]))
		outFileName = outDirName + "out.png"
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
		Host.Log("Error", "Could not readback")
		TestPassed = False

	return TestPassed

# This is so the test can be ran by itself directly
if __name__ == "builtins":
	if DoTest():
		Host.Log("Info", "test Passed")
	else:
		Host.Log("Error", "Test Failed")
