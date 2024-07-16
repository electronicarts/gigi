import Host
import GigiArray
import numpy
import os

# don't save gguser files during this script execution
Host.DisableGGUserSave(True)

def DoTest():
	TestPassed = True

	# make sure the output directory exists
	outDirName = "Techniques/UnitTests/_GoldImages/Python/profiling/"
	os.makedirs(outDirName, exist_ok=True)	

	# Load the technique
	if not Host.LoadGG("Techniques/UnitTests/Python/profiling.gg"):
		return False

	# Turn on profiling mode
	Host.SetProfilingMode(True)

	# Turn on profiling
	Host.ForceEnableProfiling(True)

	# Do one execution to ensure everything is initialized
	# Run it once again and then wait on the GPU to make sure our first gpu data point is valid and doesn't contain the initialization step
	Host.RunTechnique(2)
	Host.WaitOnGPU()

	# Render a few frames and get the profiling data
	for i in range(10):
		Host.RunTechnique()
		data = Host.GetProfilingData()
		Host.Print("[" + str(i) + "] " + str(data["Total"][1]))

	# Turn off profiling mode
	Host.SetProfilingMode(False)

	return TestPassed

# This is so the test can be ran by itself directly
if __name__ == "builtins":
	if DoTest():
		Host.Log("Info", "test Passed")
	else:
		Host.Log("Error", "Test Failed")
