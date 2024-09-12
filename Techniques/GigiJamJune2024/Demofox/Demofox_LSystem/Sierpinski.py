import Host
import GigiArray

Host.SetVariable("SymbolsSizeMax", "150000")
Host.SetVariable("LinesMax", "150000")
Host.SetVariable("CursorRotationDegrees", "60")
Host.SetVariable("BBMin", "-3")
Host.SetVariable("BBMax", "3")

Host.SetImportedBufferFile("Rules","SierpinskiRules.csv")

# Note: "button" boolean variables get set to false before the variable is read, so the below doesn't work. Need to fix that at some point, somehow.

# Do one execution to ensure everything is initialized
#Host.RunTechnique()

# Initialize
#Host.SetVariable("Initialize", "true")
#Host.RunTechnique()

#for i in range(10):
    #Host.SetVariable("Step", "true")
    #Host.RunTechnique()
