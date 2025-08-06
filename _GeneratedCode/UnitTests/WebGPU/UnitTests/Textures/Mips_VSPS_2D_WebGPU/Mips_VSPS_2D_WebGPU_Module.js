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

class class_Mips_VSPS_2D_WebGPU
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Vertex shader "VS", node "Node_1"
static ShaderCode_Node_1_VS = `
struct VSOutput_0
{
    @builtin(position) position_0 : vec4<f32>,
};

@vertex
fn main(@builtin(vertex_index) id_0 : u32) -> VSOutput_0
{
    var ret_0 : VSOutput_0;
    ret_0.position_0 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    ret_0.position_0 = vec4<f32>(vec2<f32>(f32((((id_0 << (u32(1)))) & (u32(2)))), f32((id_0 & (u32(2))))) * vec2<f32>(2.0f, -2.0f) + vec2<f32>(-1.0f, 1.0f), 0.0f, 1.0f);
    return ret_0;
}

`;

// Shader code for Pixel shader "PS", node "Node_1"
static ShaderCode_Node_1_PS = `
@binding(100) @group(0) var MipTexPS : texture_storage_2d</*(MipTexPS_format)*/, write>;

struct PSOutput_0
{
    @location(0) colorTarget_0 : vec4<f32>,
};

@fragment
fn main(@builtin(position) position_0 : vec4<f32>) -> PSOutput_0
{
    var dimensions_0 : vec2<u32>;
    var _S1 : u32 = dimensions_0[i32(0)];
    var _S2 : u32 = dimensions_0[i32(1)];
    {var dim = textureDimensions((MipTexPS));((_S1)) = dim.x;((_S2)) = dim.y;};
    dimensions_0[i32(0)] = _S1;
    dimensions_0[i32(1)] = _S2;
    var _S3 : vec2<u32> = vec2<u32>(position_0.xy) % dimensions_0;
    textureStore((MipTexPS), (_S3), (vec4<f32>(0.80000001192092896f, 0.60000002384185791f, 0.40000000596046448f, 0.20000000298023224f)));
    var ret_0 : PSOutput_0;
    ret_0.colorTarget_0 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    ret_0.colorTarget_0 = vec4<f32>(sin(position_0.x / 10.0f) * 0.5f + 0.5f, 0.20000000298023224f, 0.20000000298023224f, 1.0f);
    return ret_0;
}

`;

// -------------------- Private Members

// Texture Output
texture_Output = null;
texture_Output_size = [0, 0, 0];
texture_Output_format = "";
texture_Output_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.RENDER_ATTACHMENT;

// Texture Node_1_MipTexPS_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Node_1_MipTexPS_ReadOnly = null;
texture_Node_1_MipTexPS_ReadOnly_size = [0, 0, 0];
texture_Node_1_MipTexPS_ReadOnly_format = "";
texture_Node_1_MipTexPS_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Draw call Shader Node_1
Hash_DrawCall_Node_1 = 0;
ShaderModule_DrawCallVS_Node_1 = null;
ShaderModule_DrawCallPS_Node_1 = null;
BindGroupLayout_DrawCall_Node_1 = null;
PipelineLayout_DrawCall_Node_1 = null;
Pipeline_DrawCall_Node_1 = null;

// -------------------- Exported Members

// Texture MipTex
texture_MipTex = null;
texture_MipTex_size = [0, 0, 0];
texture_MipTex_format = "";
texture_MipTex_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// -------------------- Public Variables

variable_RenderSize = [ 256, 256 ];
variableDefault_RenderSize = [ 256, 256 ];
variableChanged_RenderSize = [ false, false ];


async SetVarsBefore()
{

}

async SetVarsAfter()
{

}

async ValidateImports()
{
    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture Output
    {
        const baseSize = [ this.variable_RenderSize[0], this.variable_RenderSize[1], 1];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba8unorm-srgb";
        if (this.texture_Output !== null && (this.texture_Output_format != desiredFormat || this.texture_Output_size[0] != desiredSize[0] || this.texture_Output_size[1] != desiredSize[1] || this.texture_Output_size[2] != desiredSize[2]))
        {
            this.texture_Output.destroy();
            this.texture_Output = null;
        }

        if (this.texture_Output === null)
        {
            this.texture_Output_size = desiredSize.slice();
            this.texture_Output_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Output_format))
                viewFormats.push(this.texture_Output_format);

            this.texture_Output = device.createTexture({
                label: "texture Mips_VSPS_2D_WebGPU.Output",
                size: this.texture_Output_size,
                format: Shared.GetNonSRGBFormat(this.texture_Output_format),
                usage: this.texture_Output_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture MipTex
    {
        const baseSize = [ 1, 1, 1 ];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 64) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 64) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba8unorm-srgb";
        if (this.texture_MipTex !== null && (this.texture_MipTex_format != desiredFormat || this.texture_MipTex_size[0] != desiredSize[0] || this.texture_MipTex_size[1] != desiredSize[1] || this.texture_MipTex_size[2] != desiredSize[2]))
        {
            this.texture_MipTex.destroy();
            this.texture_MipTex = null;
        }

        if (this.texture_MipTex === null)
        {
            this.texture_MipTex_size = desiredSize.slice();
            this.texture_MipTex_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_MipTex_format))
                viewFormats.push(this.texture_MipTex_format);

            let numMips = 1;
            {
                let maxSize = Math.max(this.texture_MipTex_size[0], this.texture_MipTex_size[1]);
                while (maxSize > 1)
                {
                    maxSize = Math.floor(maxSize / 2);
                    numMips = numMips + 1;
                }
            }
            this.texture_MipTex = device.createTexture({
                label: "texture Mips_VSPS_2D_WebGPU.MipTex",
                size: this.texture_MipTex_size,
                mipLevelCount: numMips,
                format: Shared.GetNonSRGBFormat(this.texture_MipTex_format),
                usage: this.texture_MipTex_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Node_1_MipTexPS_ReadOnly
    {
        const baseSize = this.texture_MipTex_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_MipTex_format;
        if (this.texture_Node_1_MipTexPS_ReadOnly !== null && (this.texture_Node_1_MipTexPS_ReadOnly_format != desiredFormat || this.texture_Node_1_MipTexPS_ReadOnly_size[0] != desiredSize[0] || this.texture_Node_1_MipTexPS_ReadOnly_size[1] != desiredSize[1] || this.texture_Node_1_MipTexPS_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Node_1_MipTexPS_ReadOnly.destroy();
            this.texture_Node_1_MipTexPS_ReadOnly = null;
        }

        if (this.texture_Node_1_MipTexPS_ReadOnly === null)
        {
            this.texture_Node_1_MipTexPS_ReadOnly_size = desiredSize.slice();
            this.texture_Node_1_MipTexPS_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Node_1_MipTexPS_ReadOnly_format))
                viewFormats.push(this.texture_Node_1_MipTexPS_ReadOnly_format);

            let numMips = 1;
            {
                let maxSize = Math.max(this.texture_Node_1_MipTexPS_ReadOnly_size[0], this.texture_Node_1_MipTexPS_ReadOnly_size[1]);
                while (maxSize > 1)
                {
                    maxSize = Math.floor(maxSize / 2);
                    numMips = numMips + 1;
                }
            }
            this.texture_Node_1_MipTexPS_ReadOnly = device.createTexture({
                label: "texture Mips_VSPS_2D_WebGPU.Node_1_MipTexPS_ReadOnly",
                size: this.texture_Node_1_MipTexPS_ReadOnly_size,
                mipLevelCount: numMips,
                format: Shared.GetNonSRGBFormat(this.texture_Node_1_MipTexPS_ReadOnly_format),
                usage: this.texture_Node_1_MipTexPS_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create draw call shader Node_1
    {
        const bindGroupEntries =
        [
            {
                // MipTexPS
                binding: 100,
                visibility: GPUShaderStage.FRAGMENT,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_MipTex_format), viewDimension: "2d" }
            },
            {
                // MipTexPSReadOnly
                binding: 101,
                visibility: GPUShaderStage.FRAGMENT,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Node_1_MipTexPS_ReadOnly_format), viewDimension: "2d" }
            },
        ];

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (this.ShaderModule_DrawCall_Node_1 === null || newHash !== this.Hash_DrawCall_Node_1)
        {
            this.Hash_DrawCall_Node_1 = newHash;

            let shaderCodeVS = class_Mips_VSPS_2D_WebGPU.ShaderCode_Node_1_VS;
            let shaderCodePS = class_Mips_VSPS_2D_WebGPU.ShaderCode_Node_1_PS;

            shaderCodePS = shaderCodePS.replace("/*(MipTexPS_format)*/", Shared.GetNonSRGBFormat(this.texture_MipTex_format));
            shaderCodePS = shaderCodePS.replace("/*(MipTexPSReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Node_1_MipTexPS_ReadOnly_format));

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
                            format: this.texture_Output_format,
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
    encoder.pushDebugGroup("Mips_VSPS_2D_WebGPU.Output");

    encoder.popDebugGroup(); // "Mips_VSPS_2D_WebGPU.Output"

    encoder.pushDebugGroup("Mips_VSPS_2D_WebGPU.MipTex");

    encoder.popDebugGroup(); // "Mips_VSPS_2D_WebGPU.MipTex"

    encoder.pushDebugGroup("Mips_VSPS_2D_WebGPU.Node_1_MipTexPS_ReadOnly");

    encoder.popDebugGroup(); // "Mips_VSPS_2D_WebGPU.Node_1_MipTexPS_ReadOnly"

    encoder.pushDebugGroup("Mips_VSPS_2D_WebGPU.Copy_Node_1_MipTexPS");

    // Copy texture MipTex to texture Node_1_MipTexPS_ReadOnly
    {
        const numMips = Math.min(this.texture_MipTex.mipLevelCount, this.texture_Node_1_MipTexPS_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_MipTex.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_MipTex.height >> mipIndex, 1);
            let mipDepth = this.texture_MipTex.depthOrArrayLayers;

            if (this.texture_MipTex.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_MipTex, mipLevel: mipIndex },
                { texture: this.texture_Node_1_MipTexPS_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "Mips_VSPS_2D_WebGPU.Copy_Node_1_MipTexPS"

    encoder.pushDebugGroup("Mips_VSPS_2D_WebGPU.Node_1");

    // Run draw call shader Node_1
    {
        const bindGroup = device.createBindGroup({
            label: "DrawCall Bind Group Node_1",
            layout: this.BindGroupLayout_DrawCall_Node_1,
            entries:
            [
                {
                    // MipTexPS
                    binding: 100,
                    resource: this.texture_MipTex.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 1 })
                },
                {
                    // MipTexPSReadOnly
                    binding: 101,
                    resource: this.texture_Node_1_MipTexPS_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 1 })
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
                    view: this.texture_Output.createView({ dimension: "2d", format: this.texture_Output_format, usage: GPUTextureUsage.RENDER_ATTACHMENT, baseArrayLayer: 0, arrayLayerCount: 1, baseMipLevel: 0, mipLevelCount: 1 }),
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

    encoder.popDebugGroup(); // "Mips_VSPS_2D_WebGPU.Node_1"

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

var Mips_VSPS_2D_WebGPU = new class_Mips_VSPS_2D_WebGPU;

export default Mips_VSPS_2D_WebGPU;
