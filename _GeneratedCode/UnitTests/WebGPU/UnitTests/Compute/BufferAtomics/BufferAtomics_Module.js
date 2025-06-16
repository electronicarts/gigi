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

class class_BufferAtomics
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "BufferAtomicsCS", node "RunShader"
static ShaderCode_RunShader_BufferAtomicsCS = `
@binding(0) @group(0) var<storage, read_write> TheBuffer : array<atomic<u32>>;

@compute
@workgroup_size(64, 1, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    if((DTid_0.x) >= u32(4))
    {
        return;
    }
    var i_0 : u32 = u32(0);
    for(;;)
    {
        if(i_0 < u32(4))
        {
        }
        else
        {
            break;
        }
        var _S1 : u32 = i_0 + u32(1);
        var _S2 : u32 = atomicAdd(&(TheBuffer[i_0]), _S1);
        i_0 = _S1;
    }
    return;
}

`;

// Shader code for Compute shader "ClearBufferCS", node "ClearBuffer"
static ShaderCode_ClearBuffer_ClearBufferCS = `
@binding(0) @group(0) var<storage, read_write> TheBuffer : array<u32>;

@compute
@workgroup_size(64, 1, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S1 : u32 = DTid_0.x;
    if(_S1 >= u32(4))
    {
        return;
    }
    TheBuffer[_S1] = u32(0);
    return;
}

`;

// -------------------- Private Members

// Compute Shader ClearBuffer
Hash_Compute_ClearBuffer = 0;
ShaderModule_Compute_ClearBuffer = null;
BindGroupLayout_Compute_ClearBuffer = null;
PipelineLayout_Compute_ClearBuffer = null;
Pipeline_Compute_ClearBuffer = null;

// Compute Shader RunShader
Hash_Compute_RunShader = 0;
ShaderModule_Compute_RunShader = null;
BindGroupLayout_Compute_RunShader = null;
PipelineLayout_Compute_RunShader = null;
Pipeline_Compute_RunShader = null;

// -------------------- Exported Members

// Buffer TheBuffer
buffer_TheBuffer = null;
buffer_TheBuffer_count = 0;
buffer_TheBuffer_stride = 0;
buffer_TheBuffer_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;


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
    // Handle (re)creation of buffer TheBuffer
    {
        const baseCount = 1;
        const desiredCount = Math.floor(((parseInt(baseCount) + 0 ) * 4) / 1) + 0;
        const desiredStride = 4;
        if (this.buffer_TheBuffer !== null && (this.buffer_TheBuffer_count != desiredCount || this.buffer_TheBuffer_stride != desiredStride))
        {
            this.buffer_TheBuffer.destroy();
            this.buffer_TheBuffer = null;
        }

        if (this.buffer_TheBuffer === null)
        {
            this.buffer_TheBuffer_count = desiredCount;
            this.buffer_TheBuffer_stride = desiredStride;
            this.buffer_TheBuffer = device.createBuffer({
                label: "buffer BufferAtomics.TheBuffer",
                size: Shared.Align(16, this.buffer_TheBuffer_count * this.buffer_TheBuffer_stride),
                usage: this.buffer_TheBuffer_usageFlags,
            });
        }
    }

    // (Re)create compute shader ClearBuffer
    {
        const bindGroupEntries =
        [
            {
                // TheBuffer
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_ClearBuffer === null || newHash !== this.Hash_Compute_ClearBuffer)
        {
            this.Hash_Compute_ClearBuffer = newHash;

            let shaderCode = class_BufferAtomics.ShaderCode_ClearBuffer_ClearBufferCS;

            this.ShaderModule_Compute_ClearBuffer = device.createShaderModule({ code: shaderCode, label: "Compute Shader ClearBuffer"});
            this.BindGroupLayout_Compute_ClearBuffer = device.createBindGroupLayout({
                label: "Compute Bind Group Layout ClearBuffer",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_ClearBuffer = device.createPipelineLayout({
                label: "Compute Pipeline Layout ClearBuffer",
                bindGroupLayouts: [this.BindGroupLayout_Compute_ClearBuffer],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_ClearBuffer = device.createComputePipeline({
                    label: "Compute Pipeline ClearBuffer",
                    layout: this.PipelineLayout_Compute_ClearBuffer,
                    compute: {
                        module: this.ShaderModule_Compute_ClearBuffer,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("ClearBuffer");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline ClearBuffer",
                    layout: this.PipelineLayout_Compute_ClearBuffer,
                    compute: {
                        module: this.ShaderModule_Compute_ClearBuffer,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_ClearBuffer = handle; this.loadingPromises.delete("ClearBuffer"); } );
            }
        }
    }

    // (Re)create compute shader RunShader
    {
        const bindGroupEntries =
        [
            {
                // TheBuffer
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "storage" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_RunShader === null || newHash !== this.Hash_Compute_RunShader)
        {
            this.Hash_Compute_RunShader = newHash;

            let shaderCode = class_BufferAtomics.ShaderCode_RunShader_BufferAtomicsCS;

            this.ShaderModule_Compute_RunShader = device.createShaderModule({ code: shaderCode, label: "Compute Shader RunShader"});
            this.BindGroupLayout_Compute_RunShader = device.createBindGroupLayout({
                label: "Compute Bind Group Layout RunShader",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_RunShader = device.createPipelineLayout({
                label: "Compute Pipeline Layout RunShader",
                bindGroupLayouts: [this.BindGroupLayout_Compute_RunShader],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_RunShader = device.createComputePipeline({
                    label: "Compute Pipeline RunShader",
                    layout: this.PipelineLayout_Compute_RunShader,
                    compute: {
                        module: this.ShaderModule_Compute_RunShader,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("RunShader");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline RunShader",
                    layout: this.PipelineLayout_Compute_RunShader,
                    compute: {
                        module: this.ShaderModule_Compute_RunShader,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_RunShader = handle; this.loadingPromises.delete("RunShader"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("BufferAtomics.TheBuffer");

    encoder.popDebugGroup(); // "BufferAtomics.TheBuffer"

    encoder.pushDebugGroup("BufferAtomics.ClearBuffer");

    // Run compute shader ClearBuffer
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group ClearBuffer",
            layout: this.BindGroupLayout_Compute_ClearBuffer,
            entries: [
                {
                    // TheBuffer
                    binding: 0,
                    resource: { buffer: this.buffer_TheBuffer }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = [this.buffer_TheBuffer_count, 1, 1];
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 64 - 1) / 64),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 1 - 1) / 1),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_ClearBuffer !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_ClearBuffer);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "BufferAtomics.ClearBuffer"

    encoder.pushDebugGroup("BufferAtomics.RunShader");

    // Run compute shader RunShader
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group RunShader",
            layout: this.BindGroupLayout_Compute_RunShader,
            entries: [
                {
                    // TheBuffer
                    binding: 0,
                    resource: { buffer: this.buffer_TheBuffer }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = [this.buffer_TheBuffer_count, 1, 1];
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 64 - 1) / 64),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 1 - 1) / 1),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_RunShader !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_RunShader);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "BufferAtomics.RunShader"

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

var BufferAtomics = new class_BufferAtomics;

export default BufferAtomics;
