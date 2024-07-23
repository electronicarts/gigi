# How To Make A Public Gigi Release

This document explains how to make a public release of Gigi.

In general, development should end up in the main branch of the private repo.  It should flow from there into the ExternalRelease branch, and then to the public github repo.  If development is done directly into the ExternalRelease branch or the public github repo, it will make merge development hell, so it is not recomended!

* Private Repo (VPN): https://gitlab.ea.com/seed/gigi
* Public Repo: https://github.com/electronicarts/gigi

## Preparing The Public Repo

The internal Gigi repo has a "ExternalRelease" branch based off of main which is meant to be what goes out to the public.

That branch is meant to be mirrored verbatim to the public repo at https://github.com/electronicarts/gigi.

So, the first step is to get the "ExternalRelease" branch to contain what you want the public repo to contain.  Like for instance, by merging main into the branch.

This branch should be tested to verify that things are working correctly.

The Viewer and DX12 unit tests should also be ran, to help ensure everything is working correctly.

To run the viewer unit tests, open the viewer and run the python script **Techniques/UnitTests/RunTests.py**. It should report that there were no errors.

To run the DX12 unit tests you must first generate the code for the unit tests by running **MakeCode_UnitTests_DX12.bat**.  If running that results in no code differences, you can consider it a success.  To actually run the unit tests, build and run the solution **_GeneratedCode/UnitTests/DX12/UnitTests.sln**.  It should report that there were no errors.

After testing, this branch should be pushed to the internal gitlab repo which also causes the build machine to make release binaries.  These release binaries are what will be used to make the release in the public repo, talked about further down.

Next, pull down the latest from the public repo main branch.  Delete all files except the .git folder.  Next, copy all files from the internal gitlab "ExternalRelease" branch except the .git folder, and paste it into this folder.

Review the diff to ensure nothing proprietary is leaking out to the public or similar.

You may also want to smoke test this code and run unit tests again.

You may then push the changes to a branch and merge that branch down to main (main branch is protected from directly being written).

## Gigi Version Numbers

Public Gigi versions are of the form v&lt;major&gt;.&lt;minor&gt;.&lt;build&gt; such as v0.99.0.  The major and minor version number come from Version.h.  The version number in Version.h refers to the file version of .gg files, so incrementing that number should only be done for functional reasons involving schema changes or default functionality changes.  The build number part of the version number exists only in github, and should be incremented if the major and minor version numbers are the same as the last build, or set to zero if the major or minor version number has changed since the last release.

In short, there is no need to update the version number in version.h.  Either it has been updated or it has not, but either way, the build portion of the version number will be what makes it unique in the github repo.

## Making The Release

In the private gitlab repo, find the job for the ExternalRelease branch and download the artifacts from the Release job when complete.  It will give you a file named artifacts.zip.  Rename it to Gigi-&lt;major&gt;.&lt;minor&gt;.&lt;build&gt;.x64.windows.zip, for example, Gigi-0.99.0.x64.windows.zip.  This will be one of the two files added to the public release.

To make the installer, extract that zip file. Inside, open **Install.nsi** in a text editor and update the version numbers, save and close.  Next, right click **Install.nsi** and select "Compile NSIS Script".  If you don't have NSIS installed, you can download it from https://sourceforge.net/projects/nsis/.  This will make a file "Gigi Setup.exe".  This is the other file that will be added to the public release.

In the public github repo, create a new release through the web interface at https://github.com/electronicarts/gigi/releases/new.  You should create a new tag named from the version number (such as v0.99.0) and the release should be the same name.  You can put change notes into the description.  Then click "Publish release" and you are done!