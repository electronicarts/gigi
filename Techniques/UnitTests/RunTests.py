import Host
import sys
import os
import glob
import importlib

TestsPassed = []
TestsFailed = []

def RunTest(fileName, relFileName):

	# Get the module name
	moduleName = os.path.splitext(os.path.basename(fileName))[0]

	# Import the module
	sys.path.append(os.path.dirname(fileName))
	m = importlib.import_module(moduleName)
	sys.path.pop()

	# Do the test and report results
	if m.DoTest():
		TestsPassed.append(relFileName)
		return True
	else:
		TestsFailed.append(relFileName)
		return False

# don't save gguser files during this script execution
Host.DisableGGUserSave(True)

# find all .py files recursively
exitCode = 0
for fileName in glob.glob(os.getcwd() + "/Techniques/UnitTests/**/*.py", recursive = True):
	relFileName = os.path.relpath(fileName, os.getcwd() + "/Techniques/UnitTests/")
	if relFileName != "RunTests.py":
		Host.Warn("Running Test: " + relFileName)
		if not RunTest(fileName, relFileName):
			exitCode = 1

for test in TestsPassed:
	Host.Log("Info", test + " passed")

for test in TestsFailed:
	Host.Log("Error", test + " failed")

Host.Log("Info", str(len(TestsPassed)) + " Tests Passed, " + str(len(TestsFailed)) + " Tests Failed")

# Allow gguser file saves after this script execution, but unload the .gg file first
Host.LoadGG("")
Host.DisableGGUserSave(False)

# Exit
#Host.Exit(exitCode)
