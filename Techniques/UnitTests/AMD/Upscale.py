import importlib
import sys
import Host

sys.path.append('Techniques/UnitTests/')
m = importlib.import_module("TestLogic")

resources = [
	[ "RenderUI.Color: Output_SDR (UAV - After)", True ],
]

def DoTest():
	Host.SetFrameIndex(0)
	Host.SetFrameDeltaTime(0.0625)

	# Do one execution to ensure everything is initialized
	Host.RunTechnique()

	return m.RunATest("AMD/Upscale", resources)

# This is so the test can be ran by itself directly
if __name__ == "builtins":
	DoTest()
