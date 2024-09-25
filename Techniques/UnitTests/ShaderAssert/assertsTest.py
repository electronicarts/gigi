import Host

# don't save gguser files during this script execution
Host.DisableGGUserSave(True)

class AssertInfo:
	def __init__(self, assertId = -1):
		if assertId >= 0:
			self.fmtId = Host.GetShaderAssertFormatStrId(assertId)
			self.msg = Host.GetShaderAssertMsg(assertId)
			self.displayName = Host.GetShaderAssertDisplayName(assertId)
		else:
			self.fmtId = 0
			self.msg = ''
			self.displayName = ''
	
	def __eq__(self, r):
		return self.fmtId == r.fmtId and self.msg == r.msg and self.displayName == r.displayName

	@staticmethod
	def construct(fmtId, msg, displayName):
		a = AssertInfo()
		a.fmtId = fmtId
		a.msg = msg
		a.displayName = displayName
		return a

	def __str__(self) :
		return f"\nAssert:{{\n  DisplayName:{self.displayName}, \n  msg:{self.msg}, \n  fmtId:{self.fmtId}\n}}"

def getCollectedAsserts():
	assertsCount = Host.GetCollectedShaderAssertsCount()
	return [AssertInfo(assertId) for assertId in range(0, assertsCount)]

def assertsToStr(asserts):
	return '\n'.join([f"{a}" for a in asserts])

def runTechniques():
	#skip frame in flight
	Host.RunTechnique()
	Host.RunTechnique()
	Host.RunTechnique()

Tasks = [
	{
		'name' : "TEST BASIC ASSERT: VS:YES, PS:YES",
		'gg' : "Techniques/UnitTests/ShaderAssert/simpleAssertVsYesPsYes.gg",
		'expected' : {
			'asserts' : [
				AssertInfo.construct(0, "\"VS\"", "Node_1.__simpleAssertYES_VS__GigiAssertUAV"),
				AssertInfo.construct(1, "\"PS\"", "Node_1.__simpleAssertYES_PS__GigiAssertUAV")
			],
		}
	},
	{
		'name' : "TEST BASIC ASSERT: VS:NO, PS:YES",
		'gg' : "Techniques/UnitTests/ShaderAssert/simpleAssertVsNoPsYes.gg",
		'expected' : {
			'asserts' : [
				AssertInfo.construct(1, "\"PS\"", "Node_1.__simpleAssertYES_PS__GigiAssertUAV")
			],
		}
	},
	{
		'name' : "TEST BASIC ASSERT: CS:YES",
		'gg' : "Techniques/UnitTests/ShaderAssert/simpleAssertCsYes.gg",
		'expected' : {
			'asserts' : [
				AssertInfo.construct(0, "\"CS\"", "Node_1.__simpleAssertYES_CS__GigiAssertUAV")
			],
		}
	},
	{
		'name' : "ASSERT FORMATTING: COMPLEX FMT",
		'gg' : "Techniques/UnitTests/ShaderAssert/assertFormatting.gg",
		'expected' : {
			'asserts' : [
				AssertInfo.construct(0, "\"v0:1.1 v1:1.2 v2:1.3 v3:1.4 v4:1.5 v5:1.6\"", "Node_1.__assertFormatting_CS__GigiAssertUAV")
			],
		}
	},
	{
		'name' : "ASSERT FORMATTING: NO FMT STRING",
		'gg' : "Techniques/UnitTests/ShaderAssert/noFmtStringAssert.gg",
		'expected' : {
			'asserts' : [
				AssertInfo.construct(0, "1>4", "Node_1.__noFmtStringAssert_CS__GigiAssertUAV")
			],
		}
	},
	{
		'name' : "SAME CS SHADER AND SUBGRAPH",
		'gg' : "Techniques/UnitTests/ShaderAssert/sameShaderWithSubgraphs.gg",
		'expected' : {
			'asserts' : [
				AssertInfo.construct(0, "1>4", "MainNode.__noFmtStringAssert_CS__GigiAssertUAV"),
				AssertInfo.construct(0, "1>4", "SubGraphNode_InsideSubGraphNode.__SubGraphNode_noFmtStringAssert_CS__GigiAssertUAV")
			],
		}
	},
]

def DoTask(task, i):
	name = task['name']

	Host.Log("Info", f"Processing Assert test '{name} [{i+1}/{len(Tasks)}]'")

	if not Host.LoadGG(task['gg']):
		return [False, f"Failed to load {task['gg']}"]
	
	runTechniques()
	Host.WaitOnGPU()

	expected = task['expected']

	collectedAsserts = getCollectedAsserts()
	expectedAssertsCount = len(expected['asserts'])

	if not expectedAssertsCount == len(collectedAsserts):
		return [False, f"Wrong fired asserts count expected:{expectedAssertsCount},"
					+ f"got:{len(collectedAsserts)}.\n\nFIRED ASSERTS: {assertsToStr(collectedAsserts)}"
					+ f"\n\nEXPECTED ASSERTS: {assertsToStr(expected['asserts'])}"
				]
	
	invalidAsserts = []
	for i in range(0, len(collectedAsserts)):
		expectedAssert = expected['asserts'][i]
		collectedAssert = collectedAsserts[i]
		if not expectedAssert == collectedAssert:
			invalidAsserts.append([i, expectedAssert, collectedAssert])

	if len(invalidAsserts) != 0:
		header = '\n------'
		return [False, f"Unexpected asserts:\n"
					+ header
					+ header.join([f"\n i:{a[0]}\n EXPECTED:\n{a[1]}\n GOT:{a[2]}\n" for a in invalidAsserts])]


	return [True, '']

def DoTasks():
	totalTasks = len(Tasks)

	failedTasks = []
	for i in range(0, totalTasks):
		task = Tasks[i]
		res = DoTask(task, i)
		if res[0] == False:
			failedTasks.append([task, res[1]])
	
	failedTasksCount = len(failedTasks)
	if failedTasksCount > 0:
		header = '\n-----------------------'
		resultStr = header + header.join(
			[f"\nFAILED_ASSERT_TASK:\n  name:{failedTasks[i][0]['name']}\n  gg:  {failedTasks[i][0]['gg']}\n  msg: {failedTasks[i][1]}" for i in range(0, len(failedTasks))]
		)
		Host.Log("Error", f"\nShader Asserts failed tasks [{len(failedTasks)}/{totalTasks}]\n {resultStr}")
		return False
	
	return True

def DoTest():
	Host.SetShaderAssertsLogging(False)
	TestPassed = DoTasks()
	Host.Log("Info", f"Finished processing of the {len(Tasks)} Shader Assert tests")
	Host.SetShaderAssertsLogging(True)

	return TestPassed

# This is so the test can be ran by itself directly
if __name__ == "builtins":
	if DoTest():
		Host.Log("Info", "test Passed")
	else:
		Host.Log("Error", "Test Failed")
