#--------------------------------------------------------------------------------------------------
# (c) Electronic Arts. All Rights Reserved.
#--------------------------------------------------------------------------------------------------

# Instantiate and start a new stopwatch
$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()

# Powershell script which calls clang-format on the codebase, to make sure the coding standard
# is followed. Originally taken from the Halcyon repo. A job is launched per file due to the complixity of handling excluded folders/files

# List of folders we want to run the code formatter on
$folders = @('GigiCompilerLib','GigiEdit','GigiViewerDX12','Nodes','RenderGraph','Schemas','Tests')

# Set the clang format executable path
$clangFormat = $PSScriptRoot + "\clang-format\clang-format.exe"
      
# Clear the shell
cls

# Welcome Header
Write-Host -ForegroundColor Green -BackgroundColor Black "
[Running clang-format on Gigi's codebase]" 

Write-Host -ForegroundColor Yellow "
This script runs on all *.cpp, and *.h files
Based on the rules defined in $PSScriptRoot\.clang-format
"

#excluded folders
$excludeDirectories = @('imgui\\','python\\','external\\','packages\\','curl\\','WinPixEventRuntime','WinPixEventRuntime.1.0.230302001\\','spdlog\\','stb\\','tinyexr\\','Pix\\','tinyobjloader\\','templates\\','Halcyon\\')
#some counters and value holders
$count_valid=0
$count_invalid=0
$files = @()
# Iterate over each folder
foreach ($folder in $folders) {
	Write-Host "Processing: $folder"

	$workingDirectory="$PSScriptRoot\..\..\$folder"
	Set-Location $workingDirectory
	Get-ChildItem -Recurse -Include *.cpp, *.h |
	foreach {
		$allowed = $true
		$folder =  $_.FullName
		ForEach ($Pattern in $excludeDirectories) {
			#Write-Host $Pattern vs $_.FullName
			if ($_.FullName -match $Pattern) { 
				$allowed = $false
				#Write-Host "$allowed"
				break
			} 
		}
		if($allowed) {
			#Write-Host "$allowed : $_"
			$count_valid++
			$files += $_
		} else {
			#Write-Host "$allowed : $_"
			$count_invalid++
		}
	} 		
}

#process root as well as there is some source codes in there
Write-Host "Processing: root folder"
Get-ChildItem -Depth 0 -Path "$PSScriptRoot\..\.." -Filter *.cpp |
foreach {
	#Write-Host "$_"
	$count_valid++
	$files += $_	
} 
Get-ChildItem -Depth 0 -Path "$PSScriptRoot\..\.." -Filter *.h |
foreach {
	#Write-Host "$_"
	$count_valid++
	$files += $_	
} 


# Iterate over each folder
foreach ($file in $files) {
	& $clangformat -style=file:$PSScriptRoot\.clang-format -i  $file | Out-Null
}

Write-Host "valid : $count_valid , skipped : $count_invalid"

# Wait for all jobs to complete
Wait-Job * | Out-Null

Write-Host -ForegroundColor Green "
[Script ran in" $stopwatch.Elapsed.Seconds "seconds]"