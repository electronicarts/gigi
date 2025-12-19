import importlib
import sys

sys.path.append('Techniques/UnitTests/')
m = importlib.import_module("TestLogic")

resources = [
	[ "WGComputed.Output: Output (UAV - After)", False ],
]

def DoTest():
	return m.RunATest("WorkGraphs/WGRecords", resources)

# This is so the test can be ran by itself directly
if __name__ == "builtins":
	DoTest()
