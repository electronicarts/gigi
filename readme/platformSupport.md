# Platform Support

The Viewer interprets the gigi graph using DX12 to allow you to do rapid iteration, debuging, and profiling.  The viewer can also be scripted with python to automate tedious tasks such as gathering data or images for diagrams, or for running tests.

The following code generation targets are also available:
* DX12
* WebGPU
* Unreal Engine 5.3

Below are the details of what is supported on each platform, based on the unit tests that expected to pass on each platform.

## Platform Support Summary

| Platform | Supported Tests | Unsupported Tests | Support Rate |
|----------|----------------|-------------------|--------------|
| Viewer | 94/94 | 0/94 | 100.0% |
| DX12 | 68/94 | 26/94 | 72.3% |
| WebGPU | 50/94 | 44/94 | 53.2% |
| UE 5.3 | 20/94 | 74/94 | 21.3% |

## Detailed Test Support by Platform

| Test Name | Viewer | DX12 | WebGPU | UE 5.3 |
|-----------|--------|--------|--------|--------|
| `AMD\Upscale` | ✅ | ❌ | ❌ | ❌ |
| `Barrier\BarrierTest` | ✅ | ✅ | ✅ | ✅ |
| `Buffers\MultipleUVMesh` | ✅ | ❌ | ❌ | ❌ |
| `Buffers\Strides` | ✅ | ❌ | ❌ | ✅ |
| `Buffers\StructuredBuffer` | ✅ | ✅ | ✅ | ✅ |
| `Buffers\bufferViewDescriptorTable` | ✅ | ✅ | ❌ | ❌ |
| `Buffers\buffertest` | ✅ | ✅ | ❌ | ✅ |
| `Buffers\buffertest_viewOffsetCS` | ✅ | ✅ | ❌ | ❌ |
| `Buffers\buffertest_viewOffsetCS_Vars` | ✅ | ✅ | ❌ | ❌ |
| `Buffers\buffertest_webgpu` | ✅ | ❌ | ✅ | ❌ |
| `Buffers\uint16` | ✅ | ❌ | ❌ | ❌ |
| `Compute\BufferAtomics` | ✅ | ✅ | ✅ | ❌ |
| `Compute\Defines` | ✅ | ✅ | ✅ | ❌ |
| `Compute\IndirectDispatch` | ✅ | ✅ | ✅ | ❌ |
| `Compute\OutputToMultipleInputs` | ✅ | ❌ | ❌ | ❌ |
| `Compute\ReadbackSequence` | ✅ | ✅ | ✅ | ✅ |
| `Compute\SlangAutoDiff` | ✅ | ✅ | ✅ | ✅ |
| `Compute\VariableAliases` | ✅ | ✅ | ✅ | ✅ |
| `Compute\boxblur` | ✅ | ✅ | ✅ | ✅ |
| `Compute\simple` | ✅ | ✅ | ✅ | ✅ |
| `CopyResource\CopyPartialBuffer` | ✅ | ✅ | ❌ | ❌ |
| `CopyResource\CopyResourceTest` | ✅ | ✅ | ✅ | ❌ |
| `CopyResource\CopyResourceTest_FB` | ✅ | ✅ | ❌ | ✅ |
| `Data\binaryTexF32` | ✅ | ❌ | ❌ | ❌ |
| `Data\binaryTexU8` | ✅ | ❌ | ❌ | ❌ |
| `Data\ply_cube_binary_type` | ✅ | ❌ | ❌ | ❌ |
| `Data\ply_cube_uv_struct` | ✅ | ❌ | ❌ | ❌ |
| `HLSLTime\HLSLTime` | ✅ | ❌ | ❌ | ❌ |
| `MeshShaders\Mesh` | ✅ | ✅ | ❌ | ❌ |
| `MeshShaders\MeshAmplification` | ✅ | ✅ | ❌ | ❌ |
| `MeshShaders\MeshAmplificationLines` | ✅ | ✅ | ❌ | ❌ |
| `Python\GPUWrite` | ✅ | ❌ | ❌ | ❌ |
| `Python\profiling` | ✅ | ❌ | ❌ | ❌ |
| `Raster\IndirectDraw` | ✅ | ❌ | ❌ | ❌ |
| `Raster\NoVertex_NoIndex_NoInstance` | ✅ | ✅ | ✅ | ✅ |
| `Raster\Stencil` | ✅ | ✅ | ✅ | ✅ |
| `Raster\VRS` | ✅ | ✅ | ❌ | ❌ |
| `Raster\YesVertexStruct_NoIndex_NoInstance` | ✅ | ✅ | ✅ | ✅ |
| `Raster\YesVertexStruct_NoIndex_YesInstanceStruct` | ✅ | ✅ | ✅ | ✅ |
| `Raster\YesVertexStruct_NoIndex_YesInstanceType` | ✅ | ✅ | ✅ | ✅ |
| `Raster\YesVertexStruct_YesIndex_NoInstance` | ✅ | ✅ | ✅ | ✅ |
| `Raster\YesVertexType_NoIndex_NoInstance` | ✅ | ✅ | ✅ | ✅ |
| `Raster\simpleRaster` | ✅ | ✅ | ✅ | ❌ |
| `Raster\simpleRaster2` | ✅ | ✅ | ✅ | ✅ |
| `Raster\simpleRasterInSubgraph` | ✅ | ✅ | ✅ | ❌ |
| `Raster\simpleRasterMSAA` | ✅ | ❌ | ❌ | ❌ |
| `Raster\simpleRaster_Lines` | ✅ | ✅ | ✅ | ❌ |
| `Raster\simpleRaster_Points` | ✅ | ✅ | ✅ | ❌ |
| `RayTrace\AnyHit` | ✅ | ✅ | ✅ | ❌ |
| `RayTrace\AnyHitSimple` | ✅ | ✅ | ✅ | ❌ |
| `RayTrace\IntersectionShader` | ✅ | ✅ | ✅ | ❌ |
| `RayTrace\TwoRayGens` | ✅ | ✅ | ✅ | ❌ |
| `RayTrace\TwoRayGensSubgraph` | ✅ | ✅ | ✅ | ✅ |
| `RayTrace\simpleRT` | ✅ | ✅ | ✅ | ❌ |
| `RayTrace\simpleRTDynamic` | ✅ | ❌ | ✅ | ❌ |
| `RayTrace\simpleRT_inline` | ✅ | ✅ | ✅ | ❌ |
| `ShaderAssert\assertsTest` | ✅ | ❌ | ❌ | ❌ |
| `SubGraph\ConstOverride` | ✅ | ✅ | ✅ | ❌ |
| `SubGraph\SetVarNode` | ✅ | ❌ | ❌ | ❌ |
| `SubGraph\SubGraphLoops` | ✅ | ✅ | ✅ | ❌ |
| `SubGraph\SubGraphTest` | ✅ | ✅ | ✅ | ✅ |
| `SubGraph\SubInSub` | ✅ | ✅ | ✅ | ❌ |
| `Textures\Load_Tex2DArray` | ✅ | ❌ | ❌ | ❌ |
| `Textures\Load_Tex3D` | ✅ | ❌ | ❌ | ❌ |
| `Textures\Mips_CS_2D` | ✅ | ✅ | ✅ | ❌ |
| `Textures\Mips_CS_2DArray` | ✅ | ✅ | ✅ | ❌ |
| `Textures\Mips_CS_3D` | ✅ | ✅ | ✅ | ❌ |
| `Textures\Mips_CS_Cube` | ✅ | ✅ | ❌ | ❌ |
| `Textures\Mips_DrawCall` | ✅ | ✅ | ✅ | ❌ |
| `Textures\Mips_Imported_2D` | ✅ | ❌ | ❌ | ❌ |
| `Textures\Mips_Imported_2DArray` | ✅ | ❌ | ❌ | ❌ |
| `Textures\Mips_Imported_3D` | ✅ | ❌ | ❌ | ❌ |
| `Textures\Mips_Imported_Cube` | ✅ | ❌ | ❌ | ❌ |
| `Textures\Mips_RGS_2D` | ✅ | ✅ | ❌ | ❌ |
| `Textures\Mips_ShaderToken_2D` | ✅ | ✅ | ❌ | ❌ |
| `Textures\Mips_ShaderToken_2DArray` | ✅ | ✅ | ❌ | ❌ |
| `Textures\Mips_ShaderToken_3D` | ✅ | ✅ | ❌ | ❌ |
| `Textures\Mips_ShaderToken_Cube` | ✅ | ✅ | ❌ | ❌ |
| `Textures\Mips_VSPS_2D` | ✅ | ✅ | ❌ | ❌ |
| `Textures\Mips_VSPS_2D_WebGPU` | ✅ | ❌ | ✅ | ❌ |
| `Textures\Save` | ✅ | ❌ | ❌ | ❌ |
| `Textures\Texture2DArrayRW_CS` | ✅ | ✅ | ✅ | ❌ |
| `Textures\Texture2DArrayRW_PS` | ✅ | ✅ | ✅ | ❌ |
| `Textures\Texture2DArrayRW_RGS` | ✅ | ✅ | ✅ | ❌ |
| `Textures\Texture2DRW_CS` | ✅ | ✅ | ✅ | ❌ |
| `Textures\Texture2DRW_PS` | ✅ | ✅ | ✅ | ❌ |
| `Textures\Texture2DRW_RGS` | ✅ | ✅ | ✅ | ❌ |
| `Textures\Texture3DRW_CS` | ✅ | ✅ | ✅ | ❌ |
| `Textures\Texture3DRW_PS` | ✅ | ✅ | ❌ | ❌ |
| `Textures\Texture3DRW_RGS` | ✅ | ✅ | ✅ | ❌ |
| `Textures\TextureCubeRW_CS` | ✅ | ✅ | ❌ | ❌ |
| `Textures\TextureCubeRW_PS` | ✅ | ✅ | ❌ | ❌ |
| `Textures\TextureCubeRW_RGS` | ✅ | ✅ | ❌ | ❌ |
| `Textures\TextureFormats` | ✅ | ✅ | ✅ | ❌ |