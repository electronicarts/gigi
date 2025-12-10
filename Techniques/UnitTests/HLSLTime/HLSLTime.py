import Host
import GigiArray
import numpy
import os

resources = [
	"Post2.State: State (UAV - After)"
]

# don't save gguser files during this script execution
Host.DisableGGUserSave(True)

def DoTest():
	TestPassed = True

	# make sure the output directory exists
	outDirName = "Techniques/UnitTests/_GoldImages/HLSLTime/"
	os.makedirs(outDirName, exist_ok=True)

	# Load the technique
	if not Host.LoadGG("Techniques/UnitTests/HLSLTime/HLSLTime.gg"):
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
			TestPassed = True # todo: improve
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
