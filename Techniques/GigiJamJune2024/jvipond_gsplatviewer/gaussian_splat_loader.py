import subprocess
import sys
import Host

def get_file_path(title):
    PS_Commands = "Add-Type -AssemblyName System.Windows.Forms;"
    PS_Commands += "$fileBrowser = New-Object System.Windows.Forms.OpenFileDialog;"
    PS_Commands += '$fileBrowser.Title = "' + title + '";'
    PS_Commands += "$Null = $fileBrowser.ShowDialog();"
    PS_Commands += "echo $fileBrowser.FileName"
    file_path = subprocess.run(["powershell.exe", PS_Commands], stdout=subprocess.PIPE)
    file_path = file_path.stdout.decode()
    file_path = file_path.rstrip()
    return file_path

def display_message_box(message, title, error):
    PS_Commands = "Add-Type -AssemblyName System.Windows.Forms;"
    icon = ' [System.Windows.Forms.MessageBoxIcon]::Error)' if error else ' [System.Windows.Forms.MessageBoxIcon]::None)'
    PS_Commands += '[System.Windows.Forms.MessageBox]::Show("' + message + '", "' + title + '", [System.Windows.Forms.MessageBoxButtons]::OK,' + icon
    subprocess.run(["powershell.exe", PS_Commands], stdout=subprocess.PIPE)
    
display_message_box("Please select the GaussianSplatLoader.exe. It should be in the same folder as the gaussian_splat_loader.py script", "Select GaussianSplatLoader.exe", False)
gaussian_splat_loader_path = get_file_path("Select the GaussianSplatLoader.exe")
if not gaussian_splat_loader_path.endswith("GaussianSplatLoader.exe"):
    display_message_box("GaussianSplatLoader.exe not found", "Error", True)
    Host.Print("GaussianSplatLoader.exe not found")
    sys.exit()

display_message_box("Please select a gaussian splat Ply file. You can download some from https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/datasets/pretrained/models.zip", "Open Ply file", False)
ply_file_path = get_file_path("Please select a gaussian splat Ply file")
if not ply_file_path.endswith(".ply"):
    display_message_box("The file selected doesn't have the correct extension", "Error", True)
    Host.Print("The file selected doesn't have the correct extension")
    sys.exit()

cmd = subprocess.run([gaussian_splat_loader_path, ply_file_path], capture_output=True, text=True)
params = {}
lines = cmd.stdout.splitlines()
for line in lines:
    key, value = line.strip().split(' = ')
    params[key] = int(value) if key in ["ColorWidth", "ColorHeight"] else value

Host.DisableGGUserSave(True)
Host.LoadGG("GaussianSplatRenderer.gg")
Host.SetImportedBufferType("IndexBuffer", Host.DataFieldType_Uint_16)
Host.SetImportedBufferCSVHeaderRow("IndexBuffer", False)
Host.SetImportedBufferFile("IndexBuffer", "index.csv")

if params['SplatChunkCount'] is not '0':
    Host.SetImportedBufferFile("ChunkData", params["ChunkData"])
else:
    Host.SetImportedBufferCount("ChunkData", 1)

Host.SetImportedBufferStruct("ChunkData", "GaussianSplatChunkInfo") 
Host.SetImportedBufferType("OtherData", Host.DataFieldType_Uint)
Host.SetImportedBufferFile("OtherData", params["OtherData"])
Host.SetImportedBufferType("PosData", Host.DataFieldType_Uint)
Host.SetImportedBufferFile("PosData", params["PosData"])
Host.SetImportedBufferType("ShData", Host.DataFieldType_Uint)
Host.SetImportedBufferFile("ShData", params["ShData"])

Host.SetImportedTextureFormat("ColorData", Host.TextureFormat_RGBA32_Float)
Host.SetImportedTextureSourceIsSRGB("ColorData", False)
Host.SetImportedTextureSize("ColorData", params["ColorWidth"], params["ColorHeight"], 1) 
Host.SetImportedTextureFile("ColorData", params["ColorData"])
Host.SetImportedTextureBinaryType("ColorData", Host.GGUserFile_ImportedTexture_BinaryType_Float)
Host.SetImportedTextureBinarySize("ColorData", params["ColorWidth"], params["ColorHeight"], 1)
Host.SetImportedTextureBinaryChannels("ColorData", 4)
Host.SetVariable("_SplatCount", params["SplatCount"])
Host.SetVariable("_SplatChunkCount", params["SplatChunkCount"])
Host.SetVariable("_SplatFormat", params["SplatFormat"])

'''
DEVICE_RADIX_SORT_PARTITION_SIZE = 3840
Host.SetVariable("ThreadBlocks", str((int(params["SplatCount"]) + DEVICE_RADIX_SORT_PARTITION_SIZE - 1) / DEVICE_RADIX_SORT_PARTITION_SIZE))
DEVICE_RADIX_SORT_RADIX = 256
Host.SetVariable("ScratchBufferSize", str(((int(params["SplatCount"]) + DEVICE_RADIX_SORT_PARTITION_SIZE - 1) / DEVICE_RADIX_SORT_PARTITION_SIZE) * DEVICE_RADIX_SORT_RADIX))
DEVICE_RADIX_SORT_PASSES = 4
Host.SetVariable("ReducedScratchBufferSize", str(DEVICE_RADIX_SORT_RADIX * DEVICE_RADIX_SORT_PASSES))
'''

FFX_PARALLELSORT_MAX_THREADGROUPS_TO_RUN = 800
BlockSize = 512
NumBlocks = (int(params["SplatCount"]) + BlockSize - 1) / BlockSize

numThreadGroupsToRun = FFX_PARALLELSORT_MAX_THREADGROUPS_TO_RUN
BlocksPerThreadGroup = NumBlocks / numThreadGroupsToRun
numThreadGroupsWithAdditionalBlocks = NumBlocks % numThreadGroupsToRun
if (NumBlocks < FFX_PARALLELSORT_MAX_THREADGROUPS_TO_RUN):
    BlocksPerThreadGroup = 1
    numThreadGroupsToRun = NumBlocks
    numThreadGroupsWithAdditionalBlocks = 0

numReduceThreadgroupPerBin = 2
numScanValues = 32

Host.SetVariable("numBlocksPerThreadGroup", str(BlocksPerThreadGroup))
Host.SetVariable("numThreadGroups", str(numThreadGroupsToRun))
Host.SetVariable("numThreadGroupsWithAdditionalBlocks", str(numThreadGroupsWithAdditionalBlocks))
Host.SetVariable("numReduceThreadgroupPerBin", str(numReduceThreadgroupPerBin))
Host.SetVariable("numScanValues", str(numScanValues))