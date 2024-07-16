import Host
import GigiArray
import numpy
import os

resources = [
	"csmain.buff: buff (UAV - After)",
	"csmain._csmainCB: _csmainCB (CBV)"
]

# don't save gguser files during this script execution
Host.DisableGGUserSave(True)

def DoTest():
	TestPassed = True

	# make sure the output directory exists
	outDirName = "Techniques/UnitTests/_GoldImages/Buffers/StructuredBuffer/"
	os.makedirs(outDirName, exist_ok=True)

	# Load the technique
	if not Host.LoadGG("Techniques/UnitTests/Buffers/StructuredBuffer.gg"):
		return False

	# Make this test deterministic
	Host.SetFrameIndex(0)
	Host.SetFrameDeltaTime(1.0 / 30.0)

	# Do one execution to ensure everything is initialized
	Host.RunTechnique()

	# Say that we want readback of this buffer
	for resource in resources:
		Host.SetWantReadback(resource)

	# Run the technique
	Host.RunTechnique()
	Host.WaitOnGPU()

	# For each resource, Get the results and compare
	for i, resource in enumerate(resources):
		lastReadback, success = Host.Readback(resource)
		if success:
			lastReadbackNp = numpy.array(lastReadback)
			#Host.Log("Warn", resource)
			#Host.Log("Warn", str(lastReadbackNp.shape))
			#Host.Log("Warn", str(lastReadbackNp))
			#Host.Log("Warn", "")
			outFileName = outDirName + str(i) + ".npy"
			if os.path.exists(outFileName):
				img = numpy.load(outFileName)
				if not numpy.array_equal(img, lastReadbackNp):
					Host.Log("Error", outFileName + " did not match")
					#Host.Log("Error", str(img.shape))
					#Host.Log("Error", str(img))
					#Host.Log("Error", "")
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
