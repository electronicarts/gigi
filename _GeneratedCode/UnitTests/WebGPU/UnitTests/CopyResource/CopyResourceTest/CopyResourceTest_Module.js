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

class class_CopyResourceTest
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// -------------------- Imported Members

// Texture Source_Texture
texture_Source_Texture = null;
texture_Source_Texture_size = [0, 0, 0];
texture_Source_Texture_format = "";
texture_Source_Texture_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC;

// Buffer Source_Buffer
buffer_Source_Buffer = null;
buffer_Source_Buffer_count = 0;
buffer_Source_Buffer_stride = 0;
buffer_Source_Buffer_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_SRC;

// -------------------- Exported Members

// Texture Texture_From_Texture
texture_Texture_From_Texture = null;
texture_Texture_From_Texture_size = [0, 0, 0];
texture_Texture_From_Texture_format = "";
texture_Texture_From_Texture_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC;

// Texture Texture_From_Buffer
texture_Texture_From_Buffer = null;
texture_Texture_From_Buffer_size = [0, 0, 0];
texture_Texture_From_Buffer_format = "";
texture_Texture_From_Buffer_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC;

// Buffer Buffer_From_Texture
buffer_Buffer_From_Texture = null;
buffer_Buffer_From_Texture_count = 0;
buffer_Buffer_From_Texture_stride = 0;
buffer_Buffer_From_Texture_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;

// Buffer Buffer_From_Buffer
buffer_Buffer_From_Buffer = null;
buffer_Buffer_From_Buffer_count = 0;
buffer_Buffer_From_Buffer_stride = 0;
buffer_Buffer_From_Buffer_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.STORAGE;


async SetVarsBefore()
{

}

async SetVarsAfter()
{

}

async ValidateImports()
{
    // Validate texture Source_Texture
    if (this.texture_Source_Texture === null)
    {
        Shared.LogError("Imported resource texture_Source_Texture was not provided");
        return false;
    }

    // Validate buffer Source_Buffer
    if (this.buffer_Source_Buffer === null)
    {
        Shared.LogError("Imported resource buffer_Source_Buffer was not provided");
        return false;
    }

    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture Texture_From_Texture
    {
        const baseSize = this.texture_Source_Texture_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Source_Texture_format;
        if (this.texture_Texture_From_Texture !== null && (this.texture_Texture_From_Texture_format != desiredFormat || this.texture_Texture_From_Texture_size[0] != desiredSize[0] || this.texture_Texture_From_Texture_size[1] != desiredSize[1] || this.texture_Texture_From_Texture_size[2] != desiredSize[2]))
        {
            this.texture_Texture_From_Texture.destroy();
            this.texture_Texture_From_Texture = null;
        }

        if (this.texture_Texture_From_Texture === null)
        {
            this.texture_Texture_From_Texture_size = desiredSize.slice();
            this.texture_Texture_From_Texture_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Texture_From_Texture_format))
                viewFormats.push(this.texture_Texture_From_Texture_format);

            this.texture_Texture_From_Texture = device.createTexture({
                label: "texture CopyResourceTest.Texture_From_Texture",
                size: this.texture_Texture_From_Texture_size,
                format: Shared.GetNonSRGBFormat(this.texture_Texture_From_Texture_format),
                usage: this.texture_Texture_From_Texture_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Texture_From_Buffer
    {
        const baseSize = this.texture_Source_Texture_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Source_Texture_format;
        if (this.texture_Texture_From_Buffer !== null && (this.texture_Texture_From_Buffer_format != desiredFormat || this.texture_Texture_From_Buffer_size[0] != desiredSize[0] || this.texture_Texture_From_Buffer_size[1] != desiredSize[1] || this.texture_Texture_From_Buffer_size[2] != desiredSize[2]))
        {
            this.texture_Texture_From_Buffer.destroy();
            this.texture_Texture_From_Buffer = null;
        }

        if (this.texture_Texture_From_Buffer === null)
        {
            this.texture_Texture_From_Buffer_size = desiredSize.slice();
            this.texture_Texture_From_Buffer_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Texture_From_Buffer_format))
                viewFormats.push(this.texture_Texture_From_Buffer_format);

            this.texture_Texture_From_Buffer = device.createTexture({
                label: "texture CopyResourceTest.Texture_From_Buffer",
                size: this.texture_Texture_From_Buffer_size,
                format: Shared.GetNonSRGBFormat(this.texture_Texture_From_Buffer_format),
                usage: this.texture_Texture_From_Buffer_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of buffer Buffer_From_Texture
    {
        const baseCount = this.buffer_Source_Buffer_count;
        const desiredCount = Math.floor(((parseInt(baseCount) + 0 ) * 1) / 1) + 0;
        const desiredStride = this.buffer_Source_Buffer_stride;
        if (this.buffer_Buffer_From_Texture !== null && (this.buffer_Buffer_From_Texture_count != desiredCount || this.buffer_Buffer_From_Texture_stride != desiredStride))
        {
            this.buffer_Buffer_From_Texture.destroy();
            this.buffer_Buffer_From_Texture = null;
        }

        if (this.buffer_Buffer_From_Texture === null)
        {
            this.buffer_Buffer_From_Texture_count = desiredCount;
            this.buffer_Buffer_From_Texture_stride = desiredStride;
            this.buffer_Buffer_From_Texture = device.createBuffer({
                label: "buffer CopyResourceTest.Buffer_From_Texture",
                size: Shared.Align(16, this.buffer_Buffer_From_Texture_count * this.buffer_Buffer_From_Texture_stride),
                usage: this.buffer_Buffer_From_Texture_usageFlags,
            });
        }
    }

    // Handle (re)creation of buffer Buffer_From_Buffer
    {
        const baseCount = this.buffer_Source_Buffer_count;
        const desiredCount = Math.floor(((parseInt(baseCount) + 0 ) * 1) / 1) + 0;
        const desiredStride = this.buffer_Source_Buffer_stride;
        if (this.buffer_Buffer_From_Buffer !== null && (this.buffer_Buffer_From_Buffer_count != desiredCount || this.buffer_Buffer_From_Buffer_stride != desiredStride))
        {
            this.buffer_Buffer_From_Buffer.destroy();
            this.buffer_Buffer_From_Buffer = null;
        }

        if (this.buffer_Buffer_From_Buffer === null)
        {
            this.buffer_Buffer_From_Buffer_count = desiredCount;
            this.buffer_Buffer_From_Buffer_stride = desiredStride;
            this.buffer_Buffer_From_Buffer = device.createBuffer({
                label: "buffer CopyResourceTest.Buffer_From_Buffer",
                size: Shared.Align(16, this.buffer_Buffer_From_Buffer_count * this.buffer_Buffer_From_Buffer_stride),
                usage: this.buffer_Buffer_From_Buffer_usageFlags,
            });
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("CopyResourceTest.Source_Texture");

    encoder.popDebugGroup(); // "CopyResourceTest.Source_Texture"

    encoder.pushDebugGroup("CopyResourceTest.Texture_From_Texture");

    encoder.popDebugGroup(); // "CopyResourceTest.Texture_From_Texture"

    encoder.pushDebugGroup("CopyResourceTest.Source_Buffer");

    encoder.popDebugGroup(); // "CopyResourceTest.Source_Buffer"

    encoder.pushDebugGroup("CopyResourceTest.Texture_From_Buffer");

    encoder.popDebugGroup(); // "CopyResourceTest.Texture_From_Buffer"

    encoder.pushDebugGroup("CopyResourceTest.Buffer_From_Texture");

    encoder.popDebugGroup(); // "CopyResourceTest.Buffer_From_Texture"

    encoder.pushDebugGroup("CopyResourceTest.Buffer_From_Buffer");

    encoder.popDebugGroup(); // "CopyResourceTest.Buffer_From_Buffer"

    encoder.pushDebugGroup("CopyResourceTest.Copy_Texture_To_Texture");

    // Copy texture Source_Texture to texture Texture_From_Texture
    {
        const numMips = Math.min(this.texture_Source_Texture.mipLevelCount, this.texture_Texture_From_Texture.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Source_Texture.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Source_Texture.height >> mipIndex, 1);
            let mipDepth = this.texture_Source_Texture.depthOrArrayLayers;

            if (this.texture_Source_Texture.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Source_Texture, mipLevel: mipIndex },
                { texture: this.texture_Texture_From_Texture, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "CopyResourceTest.Copy_Texture_To_Texture"

    encoder.pushDebugGroup("CopyResourceTest.Copy_Texture_To_Buffer");

    // Copy texture Source_Texture to buffer Buffer_From_Texture
    {
        const bytesPerRow = Shared.GetTextureFormatInfo(this.texture_Source_Texture.format).bytesPerPixel * this.texture_Source_Texture.width;

        encoder.copyTextureToBuffer(
            { texture: this.texture_Source_Texture },
            { buffer: this.buffer_Buffer_From_Texture, bytesPerRow: bytesPerRow },
            {
                width: this.texture_Source_Texture.width,
                height: this.texture_Source_Texture.height,
                depthOrArrayLayers: this.texture_Source_Texture.depthOrArrayLayers,
            }
        );
    }

    encoder.popDebugGroup(); // "CopyResourceTest.Copy_Texture_To_Buffer"

    encoder.pushDebugGroup("CopyResourceTest.Copy_Buffer_To_Texture");

    // Copy buffer Source_Buffer to texture Texture_From_Buffer
    {
        const bytesPerRow = Shared.GetTextureFormatInfo(this.texture_Texture_From_Buffer.format).bytesPerPixel * this.texture_Texture_From_Buffer.width;

        encoder.copyBufferToTexture(
            { buffer: this.buffer_Source_Buffer, bytesPerRow: bytesPerRow },
            { texture: this.texture_Texture_From_Buffer },
            {
                width: this.texture_Texture_From_Buffer.width,
                height: this.texture_Texture_From_Buffer.height,
                depthOrArrayLayers: this.texture_Texture_From_Buffer.depthOrArrayLayers,
            }
        );
    }

    encoder.popDebugGroup(); // "CopyResourceTest.Copy_Buffer_To_Texture"

    encoder.pushDebugGroup("CopyResourceTest.Copy_Buffer_To_Buffer");

    // Copy buffer Source_Buffer to buffer Buffer_From_Buffer
    {
        encoder.copyBufferToBuffer(
            this.buffer_Source_Buffer,
            0,
            this.buffer_Buffer_From_Buffer,
            0,
            Math.min(this.buffer_Source_Buffer.size, this.buffer_Buffer_From_Buffer.size)
        );
    }

    encoder.popDebugGroup(); // "CopyResourceTest.Copy_Buffer_To_Buffer"

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

var CopyResourceTest = new class_CopyResourceTest;

export default CopyResourceTest;
