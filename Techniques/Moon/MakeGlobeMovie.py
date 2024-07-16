import Host
import GigiArray
import numpy
import os
from PIL import Image
import ffmpeg

resource = "Tonemapping_Tonemap.SDR: Tonemapping_Color_SDR (UAV - After)"
frameCount = 240
fps = 60
rotations = 1
outdir = "Techniques/Moon/Temp/"

# make sure the outdir exists
os.makedirs(outdir, exist_ok=True)

# don't save gguser files during this script execution
Host.DisableGGUserSave(True)

# Load the technique
Host.LoadGG("Techniques/Moon/Globe.gg")

# Set the rotation to do the right number of rotations in this number of frames
Host.SetVariable("RotationRadiansPerFrame", str(6.28319 * rotations / frameCount))

# say that we want to readback a specific image
Host.SetWantReadback(resource)

# Run for a bit to ensure everything is initialized, and that the taa buffer is warmed up
Host.RunTechnique(30)

# Render frames
for frameIndex in range(frameCount):
    Host.RunTechnique()
    Host.WaitOnGPU()
    lastReadback, success = Host.Readback(resource)
    lastReadbackNp = numpy.array(lastReadback)
    lastReadbackNp = lastReadbackNp.reshape((lastReadbackNp.shape[1], lastReadbackNp.shape[2], lastReadbackNp.shape[3]))
    outFileName = outdir + str(frameIndex) + ".png"
    Image.fromarray(lastReadbackNp, "RGBA").save(outFileName)

Host.Warn(os.getcwd()+"/" + outdir+"%d.png")

# Make the video
(
    ffmpeg
    .input(os.getcwd() + "/" + outdir + "%d.png", pattern_type='sequence', framerate=fps)
    .output(os.getcwd() + "/" + outdir + "moon.mp4", **{'crf':0})  # Lossless compression
    .run()
)

Host.Exit()
