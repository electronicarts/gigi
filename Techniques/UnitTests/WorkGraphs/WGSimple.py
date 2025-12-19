import importlib
import sys

sys.path.append('Techniques/UnitTests/')
m = importlib.import_module("TestLogic")

resources = [
	[ "WorkGraph1.Output: Output (UAV - After)", True ],
]

def DoTest():
	return m.RunATest("WorkGraphs/WGSimple", resources)

# This is so the test can be ran by itself directly
if __name__ == "builtins":
	DoTest()
