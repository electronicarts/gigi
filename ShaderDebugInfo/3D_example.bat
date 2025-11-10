rem Make sure rga.exe is installed in the referenced location.
rem use "rgb.exe -l" to find the right ASIC to target.
rem Run this from command line.

C:\Apps\RadeonDeveloperToolSuite\rga.exe -s dx12 --cs-model "cs_6_1" --cs-blob 3D_example.cso --cs-entry mainCS -c gfx1032 --isa 3D_example.dis.txt -a 3D_example.stats.txt --offline --rs-bin 3D_example.rs.fxo

