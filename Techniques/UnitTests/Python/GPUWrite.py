import Host
import GigiArray
import numpy
from PIL import Image
import os
import struct

resources = [
	["Texture2D.resource", "ComputeShader.tex2D: Texture2D (UAV - After)", 0],
	["Texture2DArray.resource", "ComputeShader.tex2DArray: Texture2DArray (UAV - After)", 1],
	["Texture3D.resource", "ComputeShader.tex3D: Texture3D (UAV - After)", 0],
	["FloatBuffer.resource", "ComputeShader.floatBuffer: FloatBuffer (UAV - After)", 0],
	["StructBuffer.resource", "ComputeShader.structBuffer: StructBuffer (UAV - After)", 0],
]

# don't save gguser files during this script execution
Host.DisableGGUserSave(True)

def DoTest():
	TestPassed = True

	# make sure the output directory exists
	outDirName = "Techniques/UnitTests/_GoldImages/Python/GPUWrite/"
	os.makedirs(outDirName, exist_ok=True)	

	# Load the technique
	if not Host.LoadGG("Techniques/UnitTests/Python/GPUWrite.gg"):
		return False	

	# Specify the resources we want to readback
	for resource in resources:
		Host.SetWantReadback(resource[1])

	# Do one execution to ensure everything is initialized
	Host.RunTechnique()

	# write the texture 2D
	data = numpy.full((32,32, 4), (32, 64, 128, 255)).astype(numpy.uint8)
	Host.WriteGPUResource(resources[0][0], data.tobytes(), resources[0][2])

	# write index 1 of the texture 2D array
	data = numpy.full((32,32, 4), (50, 51, 52, 53)).astype(numpy.uint8)
	Host.WriteGPUResource(resources[1][0], data.tobytes(), resources[1][2])

	# write the texture 3D
	data = numpy.full((32,32, 3, 4), (96, 64, 88, 108)).astype(numpy.uint8)
	Host.WriteGPUResource(resources[2][0], data.tobytes(), resources[2][2])

	# Write the float buffer
	data = numpy.full(8, (8, 7, 6, 5, 4, 3.1415, 2, 1)).astype(numpy.single)
	Host.WriteGPUResource(resources[3][0], data.tobytes(), resources[3][2])

	# Write the struct buffer
	data = struct.pack("if", 31337, 1.61803398875)
	Host.WriteGPUResource(resources[4][0], data, resources[4][2])

	# Run the technique, do the writes, wait for the GPU
	Host.RunTechnique()
	Host.WaitOnGPU()			

	# Read the data back, make sure it's correct
	for i, resource in enumerate(resources):
		lastReadback, success = Host.Readback(resource[1], resource[2])
		if success:
			lastReadbackNp = numpy.array(lastReadback)
			#Host.Warn(resource[1] + " " + str(lastReadbackNp.shape))
			#Host.Print(str(lastReadbackNp))
			#Host.Print("")
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
			Host.Log("Error", "Could not readback " + resource[1])
			TestPassed = False

	return TestPassed

# This is so the test can be ran by itself directly
if __name__ == "builtins":
	if DoTest():
		Host.Log("Info", "test Passed")
	else:
		Host.Log("Error", "Test Failed")
