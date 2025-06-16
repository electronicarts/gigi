import Host
import GigiArray
import numpy
from PIL import Image
import os

resources = [
	[ "Right_WriteColor.Color: Output (UAV - After)", True ],
]

# don't save gguser files during this script execution
Host.DisableGGUserSave(True)

def DoTest():
	TestPassed = True

	# make sure the output directory exists
	#outDirName = "Techniques/UnitTests/_GoldImages/SubGraph/SetVarNode/"
	#os.makedirs(outDirName, exist_ok=True)

	# Load the technique
	if not Host.LoadGG("Techniques/UnitTests/SubGraph/SetVarNode.gg"):
		return False

	# Specify the resources we want to readback
	for resource in resources:
		Host.SetWantReadback(resource[0])

	# Do one execution to ensure everything is initialized
	Host.RunTechnique()

	# Do another to get the setvars
	Host.RunTechnique()

	if Host.GetVariable("Sub_BufferSize") != "5":
		Host.Log("Error", "Sub_BufferSize was wrong")
		TestPassed = False

	if Host.GetVariable("Sub_TextureSize") != "512,256,1":
		Host.Log("Error", "Sub_TextureSize was wrong")
		TestPassed = False

	return TestPassed

# This is so the test can be ran by itself directly
if __name__ == "builtins":
	if DoTest():
		Host.Log("Info", "test Passed")
	else:
		Host.Log("Error", "Test Failed")
