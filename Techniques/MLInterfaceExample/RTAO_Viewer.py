import Host
import GigiArray
import numpy as np
from PIL import Image
import math
import os

resource = "DoRT.g_texture: AO_Result (UAV - After)"

noiseTexture = "RayDirectionsTexture"
noiseTextureResource = "RayDirectionsTexture.resource"

groundTruth = []

NOISE_TEXTURE_SIZE = 128
GROUND_TRUTH_SAMPLES = 10000

def MakeGroundTruth():

	global groundTruth

	outFileName = "Techniques/MLInterfaceExample/RTAO_Viewer_GroundTruth"

	if os.path.exists(outFileName + ".npy"):
		groundTruth = np.load(outFileName + ".npy")
		return

	lastPercent = -1
	for frameIndex in range(GROUND_TRUTH_SAMPLES):

		percent = math.floor(100.0 * float(frameIndex) / float(GROUND_TRUTH_SAMPLES-1))
		if lastPercent != percent:
			lastPercent = percent
			Host.Print("Making Ground Truth: " + str(lastPercent) + "%")

		# Make cosine weighted hemispherical samples
		# 1) Generate random points in the square [0,1]^2
		# 2) Convert those to a uniform point in the unit radius disk by calculating an angle and radius
		# 3) Convert to cosine weighted hemisphere by making a positive z value that makes it a unit vector
		# Note: samples are indexed as [Y, X, Channel]
		samples = np.zeros((NOISE_TEXTURE_SIZE, NOISE_TEXTURE_SIZE, 4)).astype(np.single)
		rngPointInSquare = np.random.rand(NOISE_TEXTURE_SIZE, NOISE_TEXTURE_SIZE, 2).astype(np.single)
		for ix in range(NOISE_TEXTURE_SIZE):
			for iy in range(NOISE_TEXTURE_SIZE):
				angle = math.pi * 2 * rngPointInSquare[ix,iy,0]
				radius = math.sqrt(rngPointInSquare[ix,iy,1])
				x = math.cos(angle) * radius
				y = math.sin(angle) * radius
				z = math.sqrt(1 - (x*x + y*y))
				samples[ix,iy,0] = x
				samples[ix,iy,1] = y
				samples[ix,iy,2] = z
				samples[ix,iy,3] = 1.0

		# Write our samples into the noise texture
		Host.WriteGPUResource(noiseTextureResource, samples.tobytes())
		
		# render a frame and wait for the GPU to be done
		Host.RunTechnique()
		Host.WaitOnGPU()

		# Read back the output
		lastReadback, success = Host.Readback(resource)

		# make it into a numpy array of the right shape
		lastReadbackNp = np.array(lastReadback)
		lastReadbackNp = lastReadbackNp.reshape((lastReadbackNp.shape[1], lastReadbackNp.shape[2])).astype(np.single) / 255.0

		# update ground truth
		if frameIndex == 0:
			groundTruth = lastReadbackNp
		else:
			alpha = 1.0 / float(frameIndex+1)
			groundTruth = groundTruth * (1.0 - alpha) + lastReadbackNp * alpha

		# Save each step
		#Image.fromarray((groundTruth*255.0).astype(np.uint8), "L").save("Techniques/MLInterfaceExample/_" + str(frameIndex) + ".png")	

	# Save the ground truth to disk, to verify it looks correct
	np.save(outFileName + ".npy", groundTruth)
	Image.fromarray((groundTruth*255.0).astype(np.uint8), "L").save(outFileName + ".png")

# Turn off vsync to make it run faster
Host.SetVSync(False)

# Load the gg technique
Host.LoadGG("Techniques/MLInterfaceExample/RTAO_Viewer.gg")

# Set up the ray directions noise texture
Host.SetImportedTextureSize(noiseTexture, NOISE_TEXTURE_SIZE, NOISE_TEXTURE_SIZE, 1)
Host.SetImportedTextureFormat(noiseTexture, Host.TextureFormat_RGBA32_Float)

# Set the variables to the correct settings
Host.SetVariable("SourceIsUnorm", "false")
Host.SetVariable("ApplyInTangentSpace", "true")

# Say that we want readback of the resource
Host.SetWantReadback(resource)

# Do one execution to ensure everything is initialized
Host.RunTechnique(3)

# make the ground truth data
MakeGroundTruth()

# Make cosine weighted hemispherical samples
# 1) Generate random points in the square [0,1]^2
# 2) Convert those to a uniform point in the unit radius disk by calculating an angle and radius
# 3) Convert to cosine weighted hemisphere by making a positive z value that makes it a unit vector
# Note: samples are indexed as [Y, X, Channel]
samples = np.zeros((NOISE_TEXTURE_SIZE, NOISE_TEXTURE_SIZE, 4)).astype(np.single)
rngPointInSquare = np.random.rand(NOISE_TEXTURE_SIZE, NOISE_TEXTURE_SIZE, 2).astype(np.single)
for ix in range(NOISE_TEXTURE_SIZE):
	for iy in range(NOISE_TEXTURE_SIZE):
		angle = math.pi * 2 * rngPointInSquare[ix,iy,0]
		radius = math.sqrt(rngPointInSquare[ix,iy,1])
		x = math.cos(angle) * radius
		y = math.sin(angle) * radius
		z = math.sqrt(1 - (x*x + y*y))
		samples[ix,iy,0] = x
		samples[ix,iy,1] = y
		samples[ix,iy,2] = z
		samples[ix,iy,3] = 1.0

for i in range(10):

	# Write our samples into the noise texture
	Host.WriteGPUResource(noiseTextureResource, samples.tobytes())
	
	# render a frame and wait for the GPU to be done
	Host.RunTechnique()
	Host.WaitOnGPU()

	# Read back the output
	lastReadback, success = Host.Readback(resource)

	# make it into a numpy array of the right shape
	lastReadbackNp = np.array(lastReadback)
	lastReadbackNp = lastReadbackNp.reshape((lastReadbackNp.shape[1], lastReadbackNp.shape[2])).astype(np.single) / 255.0

	# Calculate and show RMSE
	RMSE = math.sqrt(np.square(np.subtract(lastReadbackNp, groundTruth)).mean())
	Host.Print("[" + str(i) + "] RMSE = " + str(RMSE))

	# Save image
	#Image.fromarray((lastReadbackNp*255.0).astype(np.uint8), "L").save("Techniques/MLInterfaceExample/_" + str(i) + ".png")	

	## ==========================================================
	## HERE: samples could be updated to try and lower the RMSE.
	## ==========================================================


