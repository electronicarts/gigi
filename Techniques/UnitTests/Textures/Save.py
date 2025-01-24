import Host
import os
import shutil
import filecmp

resources = [
	"MakeImages.ColorSDR: ColorSDR (UAV - After)",
	"MakeImages.ColorHDR: ColorHDR (UAV - After)",
]

# don't save gguser files during this script execution
Host.DisableGGUserSave(True)

def compare_binary_files(file1, file2):
	return filecmp.cmp(file1, file2, shallow=False)

def TestFilesEqual(fileNameA, fileNameB):
	if not os.path.exists(fileNameA):
		Host.Log("Error", fileNameA + " did not exist")
		return False

	if not os.path.exists(fileNameB):
		Host.Log("Error", fileNameB + " did not exist")
		shutil.copyfile(fileNameA, fileNameB)
		os.remove(fileNameA)
		return False

	if compare_binary_files(fileNameA, fileNameB):
		os.remove(fileNameA)
		return True

	Host.Log("Error", fileNameA + " did not match " + fileNameB)
	os.remove(fileNameA)
	return False

def DoTest():
	TestPassed = True

	# make sure the output directory exists
	outDirName = "Techniques/UnitTests/_GoldImages/Textures/Save/"
	os.makedirs(outDirName, exist_ok=True)

	# Load the technique
	if not Host.LoadGG("Techniques/UnitTests/Textures/Save.gg"):
		return False

	# Specify the resources we want to readback
	for resource in resources:
		Host.SetWantReadback(resource)

	# Do one execution to ensure everything is initialized
	Host.RunTechnique()

	# Get the results
	Host.RunTechnique()
	Host.WaitOnGPU()

	Host.Print("Saving SDR")

	# Save SDR image
	if not Host.SaveAsPNG(outDirName + "_SDR.png", resources[0]):
		Host.Log("Error", "SDR SaveAsPNG returned false")
		TestPassed = False

	if not Host.SaveAsDDS_BC4(outDirName + "_SDR_BC4.dds", resources[0]):
		Host.Log("Error", "SDR SaveAsDDS_BC4 returned false")
		TestPassed = False

	if not Host.SaveAsDDS_BC5(outDirName + "_SDR_BC5.dds", resources[0]):
		Host.Log("Error", "SDR SaveAsDDS_BC5 returned false")
		TestPassed = False

	if not Host.SaveAsDDS_BC7(outDirName + "_SDR_BC7.dds", resources[0]):
		Host.Log("Error", "SDR SaveAsDDS_BC7 returned false")
		TestPassed = False

	if not Host.SaveAsCSV(outDirName + "_SDR.csv", resources[0]):
		Host.Log("Error", "SDR SaveAsCSV returned false")
		TestPassed = False

	if not Host.SaveAsBinary(outDirName + "_SDR.bin", resources[0]):
		Host.Log("Error", "SDR SaveAsBinary returned false")
		TestPassed = False

	Host.Print("Saving HDR")

	# Save HDR image
	if not Host.SaveAsEXR(outDirName + "_HDR.exr", resources[1]):
		Host.Log("Error", "HDR SaveAsEXR returned false")
		TestPassed = False

	if not Host.SaveAsHDR(outDirName + "_HDR.hdr", resources[1]):
		Host.Log("Error", "HDR SaveAsHDR returned false")
		TestPassed = False

	if not Host.SaveAsDDS_BC6(outDirName + "_HDR_BC6.dds", resources[1]):
		Host.Log("Error", "HDR SaveAsDDS_BC6 returned false")
		TestPassed = False

	if not Host.SaveAsCSV(outDirName + "_HDR.csv", resources[1]):
		Host.Log("Error", "HDR SaveAsCSV returned false")
		TestPassed = False

	if not Host.SaveAsBinary(outDirName + "_HDR.bin", resources[1]):
		Host.Log("Error", "HDR SaveAsBinary returned false")
		TestPassed = False

	Host.Print("Testing")

	# Verify everything is ok
	filesToTest = [
		"SDR.index0.mip0.png",
		"SDR_BC4.dds",
		"SDR_BC5.dds",
		"SDR_BC7.dds",
		"SDR.index0.mip0.csv",
		"SDR.bin",
		"HDR.index0.mip0.exr",
		"HDR.index0.mip0.hdr",
		"HDR_BC6.dds",
		"HDR.index0.mip0.csv",
		"HDR.bin",
	]

	for file in filesToTest:
		if(not TestFilesEqual(outDirName + "_" + file, outDirName + file)):
			TestPassed = False

	Host.Print("Testing Done")

	return TestPassed

# This is so the test can be ran by itself directly
if __name__ == "builtins":
	if DoTest():
		Host.Log("Info", "test Passed")
	else:
		Host.Log("Error", "Test Failed")
