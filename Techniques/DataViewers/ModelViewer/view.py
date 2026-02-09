import Host
import GigiArray
import sys
import os
import numpy

RGResourceName = "VertexBuffer.resource"

#Host.Print("Argc: " + str(len(sys.argv)))
#Host.Print("Argv: " + str(sys.argv))

Host.LoadGG("ModelViewer.gg")

fileName = sys.argv[0]
Host.SetImportedBufferFile("VertexBuffer", fileName)
Host.SetImportedBufferFile("LightBuffer", fileName)
Host.SetImportedBufferFile("MaterialBuffer", fileName)
Host.SetImportedBufferMaterialShaderFile("MaterialBuffer", "_material.hlsli")

Host.SetCameraPos(0,0,0)
Host.SetCameraAltitudeAzimuth(0, 3.14)
Host.SetWantReadback(RGResourceName)
Host.RunTechnique(2)
Host.WaitOnGPU()

lastReadback, success = Host.Readback(RGResourceName)
lastReadbackNP = numpy.array(lastReadback)

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

#Host.Print(str(minZ))
#Host.Print(str(maxZ))

Host.SetCameraPos((minX + maxX) / 2, (minY + maxY) / 2, maxZ * 5)

farZ = (maxZ-minZ)*20
nearZ = farZ / 10000
Host.SetCameraNearFarZ(nearZ, farZ)
Host.SetCameraFlySpeed((maxZ - minZ) / 25)

# Vertex buffer contents: position(3), Color(3), Normal(3), Tagent(4), UV(2), MaterialID(1), ShapeID(1)
