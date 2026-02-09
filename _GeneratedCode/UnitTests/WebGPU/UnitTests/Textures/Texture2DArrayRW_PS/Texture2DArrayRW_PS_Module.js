import * as Shared from '../../../Shared.js';

function isNode() {
    return typeof process !== 'undefined' && process.versions?.node;
}

let fs;
let PNG;
if (isNode()) {
    await import('webgpu')
        .then((module) => { Object.assign(globalThis, module.globals); })
        .catch((error) => console.error('Error loading module:', error));

    fs = await import('node:fs');

    await import('pngjs')
        .then((module) => {PNG = module.PNG;})
        .catch((error) => console.error('Error loading module:', error));
}

String.prototype.hashCode = function ()
{
    let hash = 0;
    for (let i = 0; i < this.length; i++)
    {
        let chr = this.charCodeAt(i);
        hash = ((hash << 5) - hash) + chr;
        hash |= 0;
    }
    return hash;
}

class class_Texture2DArrayRW_PS
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Vertex shader "RWVS", node "Node_1"
static ShaderCode_Node_1_RWVS = `
struct VSOutput_0
{
    @builtin(position) position_0 : vec4<f32>,
};

@vertex
fn vsmain(@builtin(vertex_index) vertexId_0 : u32) -> VSOutput_0
{
    var ret_0 : VSOutput_0;
    ret_0.position_0 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    switch(vertexId_0)
    {
    case u32(0), :
        {
            ret_0.position_0 = vec4<f32>(-1.0f, -1.0f, 0.0f, 1.0f);
            break;
        }
    case u32(1), :
        {
            ret_0.position_0 = vec4<f32>(3.0f, -1.0f, 0.0f, 1.0f);
            break;
        }
    case u32(2), :
        {
            ret_0.position_0 = vec4<f32>(-1.0f, 3.0f, 0.0f, 1.0f);
            break;
        }
    case default, :
        {
            break;
        }
    }
    return ret_0;
}

`;

// Shader code for Pixel shader "RWPS", node "Node_1"
static ShaderCode_Node_1_RWPS = `
@binding(105) @group(0) var _loadedTexture_0 : texture_2d_array<f32>;

@binding(104) @group(0) var importedTextureReadOnly : texture_storage_2d_array</*(importedTextureReadOnly_format)*/, read>;

@binding(102) @group(0) var importedColor : texture_2d_array<f32>;

@binding(100) @group(0) var nodeTexture : texture_storage_2d_array</*(nodeTexture_format)*/, write>;

@binding(101) @group(0) var importedTexture : texture_storage_2d_array</*(importedTexture_format)*/, write>;

struct PSOutput_0
{
    @location(0) colorTarget_0 : vec4<f32>,
};

@fragment
fn psmain(@builtin(position) position_0 : vec4<f32>) -> PSOutput_0
{
    var _S1 : vec2<u32> = vec2<u32>(position_0.xy);
    var _S2 : vec3<u32> = vec3<u32>(_S1, u32(0));
    var _S3 : vec4<i32> = vec4<i32>(vec4<u32>(_S2, u32(0)));
    var loadedTexturePx_0 : vec3<f32> = (textureLoad((_loadedTexture_0), ((_S3)).xy, i32(((_S3)).z), ((_S3)).w)).xyz;
    var _S4 : vec3<i32> = vec3<i32>(vec3<u32>(_S1, u32(1)));
    var _S5 : vec4<f32> = (textureLoad((importedTextureReadOnly), ((_S4)).xy, i32(((_S4)).z)));
    var importedTexturePx_0 : vec3<f32> = _S5.xyz;
    var _S6 : vec3<u32> = vec3<u32>(_S1, u32(2));
    var _S7 : vec4<i32> = vec4<i32>(vec4<u32>(_S6, u32(0)));
    var importedColorPx_0 : vec3<f32> = (textureLoad((importedColor), ((_S7)).xy, i32(((_S7)).z), ((_S7)).w)).xyz;
    textureStore((nodeTexture), ((_S2)).xy, i32(((_S2)).z), (vec4<f32>(loadedTexturePx_0 * importedTexturePx_0 * importedColorPx_0, 1.0f)));
    textureStore((importedTexture), ((_S6)).xy, i32(((_S6)).z), (vec4<f32>((loadedTexturePx_0 + importedTexturePx_0) / vec3<f32>(2.0f) * importedColorPx_0, 1.0f)));
    var ret_0 : PSOutput_0;
    ret_0.colorTarget_0 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    ret_0.colorTarget_0 = vec4<f32>(1.0f, 0.5f, 0.25f, 1.0f);
    return ret_0;
}

`;

// -------------------- Private Members

// Texture Node_1_nodeTexture_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Node_1_nodeTexture_ReadOnly = null;
texture_Node_1_nodeTexture_ReadOnly_size = [0, 0, 0];
texture_Node_1_nodeTexture_ReadOnly_format = "";
texture_Node_1_nodeTexture_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Node_1_importedTexture_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Node_1_importedTexture_ReadOnly = null;
texture_Node_1_importedTexture_ReadOnly_size = [0, 0, 0];
texture_Node_1_importedTexture_ReadOnly_format = "";
texture_Node_1_importedTexture_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture _loadedTexture_0
texture__loadedTexture_0 = null;
texture__loadedTexture_0_size = [0, 0, 0];
texture__loadedTexture_0_format = "";
texture__loadedTexture_0_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Draw call Shader Node_1
Hash_DrawCall_Node_1 = 0;
ShaderModule_DrawCallVS_Node_1 = null;
ShaderModule_DrawCallPS_Node_1 = null;
BindGroupLayout_DrawCall_Node_1 = null;
PipelineLayout_DrawCall_Node_1 = null;
Pipeline_DrawCall_Node_1 = null;

// -------------------- Imported Members

// Texture ImportedTexture
texture_ImportedTexture = null;
texture_ImportedTexture_size = [0, 0, 0];
texture_ImportedTexture_format = "";
texture_ImportedTexture_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture ImportedColor
texture_ImportedColor = null;
texture_ImportedColor_size = [0, 0, 0];
texture_ImportedColor_format = "";
texture_ImportedColor_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING;

// Texture Color
texture_Color = null;
texture_Color_size = [0, 0, 0];
texture_Color_format = "";
texture_Color_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.RENDER_ATTACHMENT;

// -------------------- Exported Members

// Texture NodeTexture
texture_NodeTexture = null;
texture_NodeTexture_size = [0, 0, 0];
texture_NodeTexture_format = "";
texture_NodeTexture_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;


async SetVarsBefore()
{

}

async SetVarsAfter()
{

}

async ValidateImports()
{
    // Validate texture ImportedTexture
    if (this.texture_ImportedTexture === null)
    {
        Shared.LogError("Imported resource texture_ImportedTexture was not provided");
        return false;
    }

    // Validate texture ImportedColor
    if (this.texture_ImportedColor === null)
    {
        Shared.LogError("Imported resource texture_ImportedColor was not provided");
        return false;
    }

    // Validate texture Color
    if (this.texture_Color === null)
    {
        Shared.LogError("Imported resource texture_Color was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture NodeTexture
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 64) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 64) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 3) / 1) + 0
        ];
        const desiredFormat = "rgba8unorm";
        if (this.texture_NodeTexture !== null && (this.texture_NodeTexture_format != desiredFormat || this.texture_NodeTexture_size[0] != desiredSize[0] || this.texture_NodeTexture_size[1] != desiredSize[1] || this.texture_NodeTexture_size[2] != desiredSize[2]))
        {
            this.texture_NodeTexture.destroy();
            this.texture_NodeTexture = null;
        }

        if (this.texture_NodeTexture === null)
        {
            this.texture_NodeTexture_size = desiredSize.slice();
            this.texture_NodeTexture_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_NodeTexture_format))
                viewFormats.push(this.texture_NodeTexture_format);

            this.texture_NodeTexture = device.createTexture({
                label: "texture Texture2DArrayRW_PS.NodeTexture",
                size: this.texture_NodeTexture_size,
                format: Shared.GetNonSRGBFormat(this.texture_NodeTexture_format),
                usage: this.texture_NodeTexture_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Node_1_nodeTexture_ReadOnly
    {
        const baseSize = this.texture_NodeTexture_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_NodeTexture_format;
        if (this.texture_Node_1_nodeTexture_ReadOnly !== null && (this.texture_Node_1_nodeTexture_ReadOnly_format != desiredFormat || this.texture_Node_1_nodeTexture_ReadOnly_size[0] != desiredSize[0] || this.texture_Node_1_nodeTexture_ReadOnly_size[1] != desiredSize[1] || this.texture_Node_1_nodeTexture_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Node_1_nodeTexture_ReadOnly.destroy();
            this.texture_Node_1_nodeTexture_ReadOnly = null;
        }

        if (this.texture_Node_1_nodeTexture_ReadOnly === null)
        {
            this.texture_Node_1_nodeTexture_ReadOnly_size = desiredSize.slice();
            this.texture_Node_1_nodeTexture_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Node_1_nodeTexture_ReadOnly_format))
                viewFormats.push(this.texture_Node_1_nodeTexture_ReadOnly_format);

            this.texture_Node_1_nodeTexture_ReadOnly = device.createTexture({
                label: "texture Texture2DArrayRW_PS.Node_1_nodeTexture_ReadOnly",
                size: this.texture_Node_1_nodeTexture_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Node_1_nodeTexture_ReadOnly_format),
                usage: this.texture_Node_1_nodeTexture_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Node_1_importedTexture_ReadOnly
    {
        const baseSize = this.texture_ImportedTexture_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_ImportedTexture_format;
        if (this.texture_Node_1_importedTexture_ReadOnly !== null && (this.texture_Node_1_importedTexture_ReadOnly_format != desiredFormat || this.texture_Node_1_importedTexture_ReadOnly_size[0] != desiredSize[0] || this.texture_Node_1_importedTexture_ReadOnly_size[1] != desiredSize[1] || this.texture_Node_1_importedTexture_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Node_1_importedTexture_ReadOnly.destroy();
            this.texture_Node_1_importedTexture_ReadOnly = null;
        }

        if (this.texture_Node_1_importedTexture_ReadOnly === null)
        {
            this.texture_Node_1_importedTexture_ReadOnly_size = desiredSize.slice();
            this.texture_Node_1_importedTexture_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Node_1_importedTexture_ReadOnly_format))
                viewFormats.push(this.texture_Node_1_importedTexture_ReadOnly_format);

            this.texture_Node_1_importedTexture_ReadOnly = device.createTexture({
                label: "texture Texture2DArrayRW_PS.Node_1_importedTexture_ReadOnly",
                size: this.texture_Node_1_importedTexture_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Node_1_importedTexture_ReadOnly_format),
                usage: this.texture_Node_1_importedTexture_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Load texture _loadedTexture_0 from "imageb%i.png"
    if (this.texture__loadedTexture_0 === null)
    {
        const loadedTex = await Shared.CreateTextureWithPNG(device, "./assets/imageb%i.png", this.texture__loadedTexture_0_usageFlags, "2d");
        this.texture__loadedTexture_0 = loadedTex.texture;
        this.texture__loadedTexture_0_size = loadedTex.size;
        this.texture__loadedTexture_0_format = "rgba8unorm";
        this.texture__loadedTexture_0_usageFlags = loadedTex.usageFlags;
    }
    // (Re)create draw call shader Node_1
    {
        const bindGroupEntries =
        [
            {
                // nodeTexture
                binding: 100,
                visibility: GPUShaderStage.FRAGMENT,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_NodeTexture_format), viewDimension: "2d-array" }
            },
            {
                // importedTexture
                binding: 101,
                visibility: GPUShaderStage.FRAGMENT,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_ImportedTexture_format), viewDimension: "2d-array" }
            },
            {
                // importedColor
                binding: 102,
                visibility: GPUShaderStage.FRAGMENT,
                texture : { viewDimension: "2d-array", sampleType: Shared.GetTextureFormatInfo(this.texture_ImportedColor_format).sampleType }
            },
            {
                // nodeTextureReadOnly
                binding: 103,
                visibility: GPUShaderStage.FRAGMENT,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Node_1_nodeTexture_ReadOnly_format), viewDimension: "2d-array" }
            },
            {
                // importedTextureReadOnly
                binding: 104,
                visibility: GPUShaderStage.FRAGMENT,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Node_1_importedTexture_ReadOnly_format), viewDimension: "2d-array" }
            },
            {
                // _loadedTexture_0
                binding: 105,
                visibility: GPUShaderStage.FRAGMENT,
                texture : { viewDimension: "2d-array", sampleType: Shared.GetTextureFormatInfo(this.texture__loadedTexture_0_format).sampleType }
            },
        ];

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (this.ShaderModule_DrawCall_Node_1 === null || newHash !== this.Hash_DrawCall_Node_1)
        {
            this.Hash_DrawCall_Node_1 = newHash;

            let shaderCodeVS = class_Texture2DArrayRW_PS.ShaderCode_Node_1_RWVS;
            let shaderCodePS = class_Texture2DArrayRW_PS.ShaderCode_Node_1_RWPS;

            shaderCodePS = shaderCodePS.replace("/*(nodeTexture_format)*/", Shared.GetNonSRGBFormat(this.texture_NodeTexture_format));
            shaderCodePS = shaderCodePS.replace("/*(importedTexture_format)*/", Shared.GetNonSRGBFormat(this.texture_ImportedTexture_format));
            shaderCodePS = shaderCodePS.replace("/*(nodeTextureReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Node_1_nodeTexture_ReadOnly_format));
            shaderCodePS = shaderCodePS.replace("/*(importedTextureReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Node_1_importedTexture_ReadOnly_format));

            this.ShaderModule_DrawCallVS_Node_1 = device.createShaderModule({ code: shaderCodeVS, label: "DrawCall Shader VS Node_1"});
            this.ShaderModule_DrawCallPS_Node_1 = device.createShaderModule({ code: shaderCodePS, label: "DrawCall Shader PS Node_1"});

            this.BindGroupLayout_DrawCall_Node_1 = device.createBindGroupLayout({
                label: "DrawCall Bind Group Layout Node_1",
                entries: bindGroupEntries
            });

            this.PipelineLayout_DrawCall_Node_1 = device.createPipelineLayout({
                label: "DrawCall Pipeline Layout Node_1",
                bindGroupLayouts: [this.BindGroupLayout_DrawCall_Node_1],
            });

            let renderPipelineDesc = {
                label: "DrawCall Pipeline Node_1",
                layout: this.PipelineLayout_DrawCall_Node_1,
                primitive:
                {
                    cullMode: "none",
                    frontFace: "ccw",
                    topology: "triangle-list",
                },
                multisample:
                {
                    alphaToCoverageEnabled: false,
                },
                vertex:
                {
                    module: this.ShaderModule_DrawCallVS_Node_1,
                },
                fragment:
                {
                    module: this.ShaderModule_DrawCallPS_Node_1,
                    targets:
                    [
                        {
                            blend:
                            {
                                color:
                                {
                                    srcFactor: "one",
                                    dstFactor: "zero",
                                },
                                alpha:
                                {
                                    srcFactor: "one",
                                    dstFactor: "zero",
                                },
                            },
                            format: this.texture_Color_format,
                            writeMask: GPUColorWrite.RED | GPUColorWrite.GREEN | GPUColorWrite.BLUE | GPUColorWrite.ALPHA,
                        },
                    ],
                },
            };

            if (useBlockingAPIs)
            {
                this.Pipeline_DrawCall_Node_1 = device.createRenderPipeline(renderPipelineDesc);
            }
            else
            {
                this.loadingPromises.add("Node_1");
                device.createRenderPipelineAsync(renderPipelineDesc).then( handle => { this.Pipeline_DrawCall_Node_1 = handle; this.loadingPromises.delete("Node_1"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("Texture2DArrayRW_PS.NodeTexture");

    encoder.popDebugGroup(); // "Texture2DArrayRW_PS.NodeTexture"

    encoder.pushDebugGroup("Texture2DArrayRW_PS.ImportedTexture");

    encoder.popDebugGroup(); // "Texture2DArrayRW_PS.ImportedTexture"

    encoder.pushDebugGroup("Texture2DArrayRW_PS.ImportedColor");

    encoder.popDebugGroup(); // "Texture2DArrayRW_PS.ImportedColor"

    encoder.pushDebugGroup("Texture2DArrayRW_PS.Color");

    encoder.popDebugGroup(); // "Texture2DArrayRW_PS.Color"

    encoder.pushDebugGroup("Texture2DArrayRW_PS.Node_1_nodeTexture_ReadOnly");

    encoder.popDebugGroup(); // "Texture2DArrayRW_PS.Node_1_nodeTexture_ReadOnly"

    encoder.pushDebugGroup("Texture2DArrayRW_PS.Copy_Node_1_nodeTexture");

    // Copy texture NodeTexture to texture Node_1_nodeTexture_ReadOnly
    {
        const numMips = Math.min(this.texture_NodeTexture.mipLevelCount, this.texture_Node_1_nodeTexture_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_NodeTexture.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_NodeTexture.height >> mipIndex, 1);
            let mipDepth = this.texture_NodeTexture.depthOrArrayLayers;

            if (this.texture_NodeTexture.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_NodeTexture, mipLevel: mipIndex },
                { texture: this.texture_Node_1_nodeTexture_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "Texture2DArrayRW_PS.Copy_Node_1_nodeTexture"

    encoder.pushDebugGroup("Texture2DArrayRW_PS.Node_1_importedTexture_ReadOnly");

    encoder.popDebugGroup(); // "Texture2DArrayRW_PS.Node_1_importedTexture_ReadOnly"

    encoder.pushDebugGroup("Texture2DArrayRW_PS.Copy_Node_1_importedTexture");

    // Copy texture ImportedTexture to texture Node_1_importedTexture_ReadOnly
    {
        const numMips = Math.min(this.texture_ImportedTexture.mipLevelCount, this.texture_Node_1_importedTexture_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_ImportedTexture.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_ImportedTexture.height >> mipIndex, 1);
            let mipDepth = this.texture_ImportedTexture.depthOrArrayLayers;

            if (this.texture_ImportedTexture.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_ImportedTexture, mipLevel: mipIndex },
                { texture: this.texture_Node_1_importedTexture_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "Texture2DArrayRW_PS.Copy_Node_1_importedTexture"

    encoder.pushDebugGroup("Texture2DArrayRW_PS._loadedTexture_0");

    encoder.popDebugGroup(); // "Texture2DArrayRW_PS._loadedTexture_0"

    encoder.pushDebugGroup("Texture2DArrayRW_PS.Node_1");

    // Run draw call shader Node_1
    {
        const bindGroup = device.createBindGroup({
            label: "DrawCall Bind Group Node_1",
            layout: this.BindGroupLayout_DrawCall_Node_1,
            entries:
            [
                {
                    // nodeTexture
                    binding: 100,
                    resource: this.texture_NodeTexture.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // importedTexture
                    binding: 101,
                    resource: this.texture_ImportedTexture.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // importedColor
                    binding: 102,
                    resource: this.texture_ImportedColor.createView({ dimension: "2d-array", format: this.texture_ImportedColor_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // nodeTextureReadOnly
                    binding: 103,
                    resource: this.texture_Node_1_nodeTexture_ReadOnly.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // importedTextureReadOnly
                    binding: 104,
                    resource: this.texture_Node_1_importedTexture_ReadOnly.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _loadedTexture_0
                    binding: 105,
                    resource: this.texture__loadedTexture_0.createView({ dimension: "2d-array", format: this.texture__loadedTexture_0_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
            ]
        });

        let renderPassDescriptor =
        {
            label: "DrawCall Render Pass Descriptor Node_1",
            colorAttachments:
            [
                {
                    clearValue: [ 1, 1, 1, 1 ],
                    loadOp: "load",
                    storeOp: "store",
                    view: this.texture_Color.createView({ dimension: "2d", format: this.texture_Color_format, usage: GPUTextureUsage.RENDER_ATTACHMENT, baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 0, mipLevelCount: 1 }),
                },
            ],
        };

        const vertexCountPerInstance = 3;
        const instanceCount = 1;

        if (this.Pipeline_DrawCall_Node_1 !== null)
        {
            const passEncoder = encoder.beginRenderPass(renderPassDescriptor);
                passEncoder.setPipeline(this.Pipeline_DrawCall_Node_1);
                passEncoder.setBindGroup(0, bindGroup);
                passEncoder.setStencilReference(0);
                passEncoder.draw(vertexCountPerInstance, instanceCount);
            passEncoder.end();
        }
    }

    encoder.popDebugGroup(); // "Texture2DArrayRW_PS.Node_1"

}

/*
When useBlockingAPIs is false, this function works differently:
1) When first called, it does frame start SetVariables, and starts asynchronous operations.
2) If any async operations are in flight, it exits out.
3) When this function is called again, if there are still async operations in flight it will do nothing.
4) if there are not async operations in flight, it will add passes to the encoder and do end frame SetVariables.

This dance is needed because we don't want to block the main thread in browsers.
However, some logic in the render graph may only happen during the first execution (driven by variables) which
could cause it never to happen if the async operation took longer than one Execute() call, and the start frame
set variables were allowed to be called more than once before execution, or the end frame variables were allowed
to be called at all.

When useBlockingAPIs is true, this function does everything every frame, blocking as needed.
Node.js has useBlockingAPIs=true to simplify logic. Node has no shader compilation timeouts, unlike the browser.

*/
async Execute(device, encoder, useBlockingAPIs = false)
{
    // Always make sure we have our required imported resources
    if (!await this.ValidateImports())
        return false;

    // If we are waiting on promises it means we've already done these steps and are waiting for the promises to be done
    if (!this.waitingOnPromises)
    {
        await this.SetVarsBefore();
        await this.Init(device, encoder, useBlockingAPIs);
    }

    // If we are waiting on promises, remember that and exit out
    if (this.loadingPromises.size > 0)
    {
        this.waitingOnPromises = true;
        return true;
    }
    this.waitingOnPromises = false;

    // If we got here, all async work is done, so fill the encoder and do the end of frame set variable logic
    await this.FillEncoder(device, encoder);
    await this.SetVarsAfter();

    return true;
}

};

var Texture2DArrayRW_PS = new class_Texture2DArrayRW_PS;

export default Texture2DArrayRW_PS;
