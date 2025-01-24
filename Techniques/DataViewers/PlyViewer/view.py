import Host
import GigiArray
import sys
import os
import numpy

ImportedResourceName = "VB"
RGResourceName = "VB.resource"

#Host.Print("Argc: " + str(len(sys.argv)))
#Host.Print("Argv: " + str(sys.argv))

Host.LoadGG("PlyView.gg")

fileName = sys.argv[0]
Host.SetImportedBufferFile(ImportedResourceName, fileName)

Host.SetCameraPos(0,0,0)
Host.SetCameraAltitudeAzimuth(0, 3.14)
Host.SetWantReadback(RGResourceName)
Host.RunTechnique(2)
Host.WaitOnGPU()

lastReadback, success = Host.Readback(RGResourceName)
lastReadbackNP = numpy.array(lastReadback)

#Host.Print(str(lastReadbackNP.shape))

vertexCount = lastReadbackNP.shape[0]
minX = lastReadbackNP[0][0]
maxX = lastReadbackNP[0][0]
minY = lastReadbackNP[0][1]
maxY = lastReadbackNP[0][1]
minZ = lastReadbackNP[0][2]
maxZ = lastReadbackNP[0][2]
for index in range(vertexCount):
	minX = min(minX, lastReadbackNP[index][0])
	maxX = max(maxX, lastReadbackNP[index][0])
	minY = min(minY, lastReadbackNP[index][1])
	maxY = max(maxY, lastReadbackNP[index][1])
	minZ = min(minZ, lastReadbackNP[index][2])
	maxZ = max(maxZ, lastReadbackNP[index][2])

Host.Print(str(minX))
Host.Print(str(maxX))

Host.Print(str(minY))
Host.Print(str(maxY))

Host.Print(str(minZ))
Host.Print(str(maxZ))

biggestSide = max([-minX, maxX, -minY, maxY, maxZ])
Host.Print("Biggest Side = " + str(biggestSide))

Host.SetCameraPos((minX + maxX) / 2, (minY + maxY) / 2, biggestSide * 5)

farZ = (biggestSide-minZ)*20
nearZ = farZ / 10000
Host.SetCameraNearFarZ(nearZ, farZ)
Host.SetCameraFlySpeed((biggestSide - minZ) / 25)

# Vertex buffer contents: position(3), Color(3), Normal(3), Tagent(4), UV(2), MaterialID(1), ShapeID(1)
