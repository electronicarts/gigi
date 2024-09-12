import pywavefront
import numpy as np

scene = pywavefront.Wavefront('pighead.obj', collect_faces=True)

mesh = scene.mesh_list[0]

vertices = scene.vertices
vertexCount = len(vertices)
edges = {}
halfEdges = {}
faces = []

for face in mesh.faces:
    
    faceIdx = len(faces)
    faces.append(face)
    
    for edgeIdx in range(3):
        i0 = edgeIdx
        i1 = (edgeIdx + 1) % 3
        
        edge = [face[i0], face[i1]]

        sortedEdge = edge
        sortedEdge.sort()
        sortedEdge = tuple(sortedEdge)

        if sortedEdge not in edges:
            
            v0 = np.array(vertices[sortedEdge[0]])
            v1 = np.array(vertices[sortedEdge[1]])

            initialLength = np.linalg.norm(v1 - v0)

            edges[sortedEdge] = {
                'initialLength': initialLength,
                'foundFace': faceIdx
            }

        else: # Edge is already in the list of edges, we have discovered the second half edge.

            # This is the vertex in the current face that is not part of the edge
            thisHingeVertex = face[(i1 + 1) % 3]

            try:

                otherFace = faces[edges[sortedEdge]['foundFace']]

                otherHingeVertex = None

                if otherFace[0] not in face:
                    otherHingeVertex = otherFace[0]
                elif otherFace[1] not in face:
                    otherHingeVertex = otherFace[1]
                else:
                    otherHingeVertex = otherFace[2]
                assert otherHingeVertex not in face

                assert otherHingeVertex < vertexCount
                assert thisHingeVertex < vertexCount

                v0 = np.array(vertices[thisHingeVertex])
                v1 = np.array(vertices[otherHingeVertex])

                initialLength = np.linalg.norm(v1 - v0)
                
                newSortedEdge = [thisHingeVertex, otherHingeVertex]
                newSortedEdge.sort()
                newSortedEdge = tuple(newSortedEdge)
                if newSortedEdge not in edges:
                    edges[newSortedEdge] = {'initialLength': initialLength}
                else:
                    print(f'Edge {newSortedEdge} is already in edges with length {edges[newSortedEdge]['initialLength']}')

            except:
                ()

edgeIndices = []
edgeLengths = []
vertexEdges = [] 
vertexConnectedVertices = []
for i in range(vertexCount):
    vertexEdges.append(set())
    vertexConnectedVertices.append(set())


for edge in edges:

    vertexEdges[edge[0]].add(len(edgeIndices))
    vertexEdges[edge[1]].add(len(edgeIndices))

    vertexConnectedVertices[edge[0]].add(edge[1])
    vertexConnectedVertices[edge[1]].add(edge[0])

    edgeIndices.append(edge)
    edgeLengths.append(edges[edge]['initialLength'])

vertexEdgeIndices = []
vertexEdgeIndicesBegin = []
for i in range(vertexCount):
    vertexEdgeIndicesBegin.append(len(vertexEdgeIndices))    
    vertexEdgeIndices.extend([j for j in vertexEdges[i]])

vertexEdgeIndicesBegin.append(len(vertexEdgeIndices))

# Greedy vertex colouring
vertexColors = [-1]*vertexCount

colorCount = 0
for i in range(vertexCount):
    neighboringColors = set()

    for connectedVertex in vertexConnectedVertices[i]:
        connectedVertexColor = vertexColors[connectedVertex]
        if connectedVertexColor != -1:
            neighboringColors.add(connectedVertexColor)
    
    color = 0
    while color in neighboringColors:
        color += 1
    
    colorCount = max(colorCount, color+1)

    vertexColors[i] = color

print(f'Color count: [{colorCount}]')

verticesOfColor = []
for colorIdx in range(colorCount):
    verticesOfColor.append([])

for i in range(vertexCount):
    verticesOfColor[vertexColors[i]].append(i)



vertexColoring = []
vertexColoringBegins = []

for colorIdx in range(colorCount):
    vertexColoringBegins.append(len(vertexColoring))
    vertexColoring.extend(verticesOfColor[colorIdx])

vertexColoringBegins.append(len(vertexColoring))

# Compressed row storage format:
# - row count (1 u32)
# - start index of each row (row count + 1 u32)
# - columns for each row
#
vertexColoringData = [colorCount]
vertexColoringData.extend(vertexColoringBegins)
vertexColoringData.extend(vertexColoring)

vertexEdgeData = [vertexCount]
vertexEdgeData.extend(vertexEdgeIndicesBegin)
vertexEdgeData.extend(vertexEdgeIndices)

for vertexIdx in range(len(vertices)):
    vertex = vertices[vertexIdx]
    vertex = (vertex[0], vertex[1], vertex[2], 0)
    vertices[vertexIdx] = vertex

np.savetxt('vertexPositions.csv', vertices, delimiter = ',', fmt='%f')
np.savetxt('faces.csv', np.array(faces).reshape((-1)), fmt='%d')
np.savetxt('edgeIndices.csv', edgeIndices, delimiter=',', fmt='%d')
np.savetxt('edgeLengths.csv', edgeLengths, delimiter=',', fmt='%f')
np.savetxt('vertexColoringData.csv', vertexColoringData, delimiter=',', fmt='%d')
np.savetxt('vertexEdgeData.csv', vertexEdgeData, delimiter=',', fmt='%d')
