#///////////////////////////////////////////////////////////////////////////////
#//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
#//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
#///////////////////////////////////////////////////////////////////////////////

import os
import glob
import subprocess
import shutil

passCount = 0
failCount = 0

print("\x1b[1;33mRunning WebGPU Unit Tests...\x1b[0m")

for fileName in glob.glob(os.getcwd() + "/UnitTests/**/index.html", recursive = True):
    relFileName = os.path.relpath(fileName, os.getcwd())
    relPath = os.path.dirname(relFileName)
    cwd = os.getcwd()
    os.chdir(relPath)
    returnCode = subprocess.run("node .").returncode
    os.chdir(cwd)

    if returnCode == 0:
        passCount = passCount + 1
        print("[\x1b[1;32mPass\x1b[0m] " + relPath)
    else:
        failCount = failCount + 1
        print("[\x1b[1;31mFail\x1b[0m] " + relPath)

if failCount == 0:
    print("[\x1b[1;32mPass\x1b[0m] " + str(passCount) + " passed, " + str(failCount) + " failed.")
else:
    print("[\x1b[1;31mFail\x1b[0m] " + str(passCount) + " passed, " + str(failCount) + " failed.")
