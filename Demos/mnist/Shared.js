
export function isNode() {
    return typeof process !== 'undefined' && process.versions?.node;
}

let $;
await import("./jquery-csv.js")
    .then((module) => { $ = module.$ })
    .catch((error) => console.error('Error loading module:', error));

let fs;
let PNG;
if (isNode()) {
    await import('webgpu')
        .then((module) => { Object.assign(globalThis, module.globals); })
        .catch((error) => console.error('Error loading module:', error));

    fs = await import('node:fs');

    await import('pngjs')
        .then((module) => { PNG = module.PNG; })
        .catch((error) => console.error('Error loading module:', error));
}

// -------------------- Load CSV

function LoadCSV_Node(fileName, lambda)
{
    const fileContent = fs.readFileSync(fileName, { encoding: 'utf-8' });
    //const records = parse(fileContent, { columns: true, skip_empty_lines: true });
    //console.log(records);
    //return records.map(lambda)
    let csvData = $.csv.toObjects(fileContent);
    //console.log(csvData);
    return csvData.map(lambda);
}
async function LoadCSV_Browser(url, lambda)
{
    const res = await fetch(url);
    const text = await res.text();
    let csvData = $.csv.toObjects(text);
    //console.log(csvData);
    return csvData.map(lambda);
}

export async function LoadCSV(fileName, lambda)
{
    if (isNode())
        return LoadCSV_Node(fileName, lambda);
    else
        return await LoadCSV_Browser(fileName, lambda);
}

// -------------------- Load Binary Files

function LoadBin_Node(fileName, lambda)
{
    let fileContents = fs.readFileSync(fileName);
    let fileContentsArrayBuffer = fileContents.buffer.slice(fileContents.byteOffset, fileContents.byteOffset + fileContents.byteLength);
    lambda(fileContentsArrayBuffer);
    return true;
}

async function LoadBin_Browser(url, lambda)
{
    const res = await fetch(url);
    if (!res.ok)
        return false;
    lambda(await res.arrayBuffer());
    return true;
}

export async function LoadBin(fileName, lambda)
{
    if (isNode())
        return LoadBin_Node(fileName, lambda);
    else
        return await LoadBin_Browser(fileName, lambda);
}

// -------------------- Load PNG Files

const c_cubeMapNames = [
    "Right",
    "Left",
    "Up",
    "Down",
    "Front",
    "Back"
]

export function LoadPNG_Node(fileName)
{
    var data = fs.readFileSync(fileName);
    return PNG.sync.read(data);
}

function CreateTextureWithPNG_Node(device, fileName, usageFlags, dimension)
{
    const format = "rgba8unorm-srgb";

    const useCubeMapNames = fileName.includes("%s");
    const hasPercentI = fileName.includes("%i");

    let textureIndex = -1;
    let bitmaps = []
    while(true)
    {
        textureIndex++;

        let indexedFileName = fileName;
        if (hasPercentI)
            indexedFileName = fileName.replace("%i", textureIndex.toString());
        else if (useCubeMapNames)
            indexedFileName = fileName.replace("%s", c_cubeMapNames[textureIndex]);

        let loaded = true;
        try
        {
            const source = LoadPNG_Node(indexedFileName);
            bitmaps.push(source);
        }
        catch(error)
        {
            loaded = false;
        }

        if (!loaded) {
            if (textureIndex == 0) {
                LogError("Could not load " + indexedFileName + ": " + error.message)
                return null;
            }
            break;
        }

        if (bitmaps.length > 1 && (bitmaps[0].width != bitmaps[bitmaps.length - 1].width || bitmaps[0].height != bitmaps[bitmaps.length - 1].height))
        {
            LogError("Images must all be the same size in " + indexedFileName);
            return null;
        }

        if (useCubeMapNames && textureIndex == 5)
            break;

        if (!useCubeMapNames && !hasPercentI)
            break;
    }

    let ret = {};
    ret.size = [ bitmaps[0].width, bitmaps[0].height, bitmaps.length ];
    ret.format = format;
    ret.usageFlags = usageFlags | GPUTextureUsage.RENDER_ATTACHMENT;

    let viewFormats = [];
    if (IsSRGBFormat(format))
        viewFormats.push(format);

    ret.texture = device.createTexture({
        dimension: dimension,
        label: fileName,
        format: GetNonSRGBFormat(format),
        size: ret.size,
        usage: ret.usageFlags,
        viewFormats: viewFormats,
    });

    for (let i = 0; i < bitmaps.length; i++)
    {
        device.queue.writeTexture(
            { texture: ret.texture, origin: [0, 0, i] },
            bitmaps[i].data.buffer,
            { bytesPerRow: bitmaps[i].width * 4 },
            { width: bitmaps[i].width, height: bitmaps[i].height }
        );
    }

    return ret;
}

async function CreateTextureWithPNG_Browser(device, url, usageFlags, dimension)
{
    const format = "rgba8unorm-srgb";

    const useCubeMapNames = url.includes("%s");
    const hasPercentI = url.includes("%i");

    let textureIndex = -1;
    let bitmaps = []
    while(true)
    {
        textureIndex++;

        let indexedURL = url;
        if (hasPercentI)
            indexedURL = url.replace("%i", textureIndex.toString());
        else if (useCubeMapNames)
            indexedURL = url.replace("%s", c_cubeMapNames[textureIndex]);

        let loaded = true;
        let errorMessage = "";
        try
        {
            const res = await fetch(indexedURL);
            const blob = await res.blob();
            const source = await createImageBitmap(blob, { colorSpaceConversion: 'none' });
            bitmaps.push(source);
        }
        catch(error)
        {
            loaded = false;
            errorMessage = error.message;
        }

        if (!loaded) {
            if (textureIndex == 0) {
                LogError("Could not load " + indexedURL + ": " + errorMessage)
                return null;
            }
            break;
        }

        if (bitmaps.length > 1 && (bitmaps[0].width != bitmaps[bitmaps.length - 1].width || bitmaps[0].height != bitmaps[bitmaps.length - 1].height))
        {
            LogError("Images must all be the same size in " + url);
            return null;
        }

        if (useCubeMapNames && textureIndex == 5)
            break;

        if (!useCubeMapNames && !hasPercentI)
            break;
    }

    let ret = {};
    ret.size = [ bitmaps[0].width, bitmaps[0].height, bitmaps.length ];
    ret.format = format;
    ret.usageFlags = usageFlags | GPUTextureUsage.RENDER_ATTACHMENT;

    let viewFormats = [];
    if (IsSRGBFormat(format))
        viewFormats.push(format);

    ret.texture = device.createTexture({
        dimension: dimension,
        label: url,
        format: GetNonSRGBFormat(format),
        size: ret.size,
        usage: ret.usageFlags,
        viewFormats: viewFormats,
    });

    let encoder = null;
    let tempCopyTexture = null;
    if (dimension == "3d")
    {
        tempCopyTexture = device.createTexture({
            dimension: "2d",
            label: "tempCopyTexture",
            format: GetNonSRGBFormat(format),
            size: [ret.size[0], ret.size[1], 1 ],
            usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.COPY_SRC | GPUTextureUsage.COPY_DST,
            viewFormats: viewFormats,
        });

        encoder = device.createCommandEncoder();
    }

    for (let i = 0; i < bitmaps.length; i++)
    {
        if (dimension == "3d")
        {
            // copyExternalImageToTexture can only copy to a 2d in dawn (chrome)
            // So go through a temporary 2d texture
            // https://chromium.googlesource.com/chromium/src/+/6e4d654eb28a69ed989230f1a59189a2cca803e8/third_party/blink/renderer/modules/webgpu/gpu_queue.cc#522
            device.queue.copyExternalImageToTexture(
                { source: bitmaps[i] },
                { texture: tempCopyTexture, origin: [0, 0, 0] },
                [ bitmaps[0].width, bitmaps[0].height, 1 ],
            );

            encoder.copyTextureToTexture(
                { texture: tempCopyTexture },
                { texture: ret.texture, origin: [0, 0, i] },
                [bitmaps[0].width, bitmaps[0].height, 1],
            );
        }
        else
        {
            device.queue.copyExternalImageToTexture(
                { source: bitmaps[i] },
                { texture: ret.texture, origin: [0, 0, i] },
                [ bitmaps[0].width, bitmaps[0].height, 1 ],
            );
        }
    }

    if (dimension == "3d")
    {
        device.queue.submit([encoder.finish()]);
    }

    return ret;
}

export async function CreateTextureWithPNG(device, fileName, usageFlags, dimension = "2d")
{
    if (isNode())
        return CreateTextureWithPNG_Node(device, fileName, usageFlags, dimension);
    else
        return await CreateTextureWithPNG_Browser(device, fileName, usageFlags, dimension);
}

// -------------------- Save Files

export function SavePNG(fileName, width, height, dataView) {
    if (!isNode()) {
        LogError("Can only save a PNG in node!")
        return;
    }

    if (fileName.includes("/")) {
        const pathWithoutFilename = fileName.substring(0, fileName.lastIndexOf("/"));
        if (!fs.existsSync(pathWithoutFilename))
            fs.mkdirSync(pathWithoutFilename, { recursive: true });
    }

    const png = new PNG({
        width: width,
        height: height,
        filterType: -1,
    });

    png.data = Buffer.from(dataView.buffer, dataView.byteOffset, dataView.byteLength);

    fs.writeFileSync(fileName, PNG.sync.write(png, { colorType: 6 }));
}

export function SaveBin(fileName, dataView) {
    if (!isNode()) {
        LogError("Can only save a PNG in node!")
        return;
    }

    if (fileName.includes("/")) {
        const pathWithoutFilename = fileName.substring(0, fileName.lastIndexOf("/"));
        if (!fs.existsSync(pathWithoutFilename))
            fs.mkdirSync(pathWithoutFilename, { recursive: true });
    }

    fs.writeFileSync(fileName, Buffer.from(dataView.buffer, dataView.byteOffset, dataView.byteLength));
}

// -------------------- Matrix Math

export function MatTranspose(matA)
{
    let ret = new Array(16);

    for (let i = 0; i < 4; i++)
        for (let j = 0; j < 4; j++)
            ret[i*4+j] = matA[j*4+i];
    return ret;
}

export function MatMul(matA, matB)
{
    let ret = new Array(16).fill(0);

    for (let i = 0; i < 4; i++)
    {
        for (let j = 0; j < 4; j++)
        {
            for (let k = 0; k < 4; k++)
            {
                ret[j*4+i] += matA[j*4+k] * matB[k*4+i];
            }
        }
    }

    return ret;
}

export function GetMatrixTranslation(x, y, z)
{
    let ret = new Array(16).fill(0);

    ret[0] = 1.0;
    ret[5] = 1.0;
    ret[10] = 1.0;
    ret[15] = 1.0;

    ret[12] = x;
    ret[13] = y;
    ret[14] = z;

    return ret;
}

export function GetMatrixRotationX(theta)
{
    let ret = new Array(16).fill(0);

    ret[0] = 1.0;
    ret[15] = 1.0;

    ret[5] = Math.cos(theta);
    ret[6] = Math.sin(theta);

    ret[9] = -ret[6];
    ret[10] = ret[5];

    return ret;
}

export function GetMatrixRotationY(theta)
{
    let ret = new Array(16).fill(0);

    ret[5] = 1.0;
    ret[15] = 1.0;

    ret[0] = Math.cos(theta);
    ret[2] = -Math.sin(theta);

    ret[8] = -ret[2];
    ret[10] = ret[0];

    return ret;
}

export function GetViewMatrix(pos, altitudeAzimuth)
{
    let altitude = altitudeAzimuth[0];
    let azimuth = altitudeAzimuth[1];
    const rotY = GetMatrixRotationY(azimuth);
    const rotX = GetMatrixRotationX(altitude);
    const trans = GetMatrixTranslation(-pos[0], -pos[1], -pos[2]);
    return MatMul(trans, MatMul(rotY, rotX));
}

export function GetInvViewMatrix(pos, altitudeAzimuth)
{
    let altitude = altitudeAzimuth[0];
    let azimuth = altitudeAzimuth[1];
    const rotY = GetMatrixRotationY(-azimuth);
    const rotX = GetMatrixRotationX(-altitude);
    const trans = GetMatrixTranslation(pos[0], pos[1], pos[2]);
    return MatMul(rotX, MatMul(rotY, trans));
}

export function GetPerspectiveMatrixLH(fovY, aspectRatio, nearZ, farZ)
{
    const height = Math.cos(fovY / 2.0) / Math.sin(fovY / 2.0);
    const width = height / aspectRatio;
    const range = farZ / (farZ - nearZ);

    let ret = new Array(16).fill(0);

    ret[0] = width;
    ret[5] = height;
    ret[10] = range;
    ret[11] = 1.0;
    ret[14] = -range * nearZ;

    return ret;
}

export function GetInvPerspectiveMatrixLH(fovY, aspectRatio, nearZ, farZ)
{
    const height = Math.cos(fovY / 2.0) / Math.sin(fovY / 2.0);
    const width = height / aspectRatio;
    const range = farZ / (farZ - nearZ);

    let ret = new Array(16).fill(0);

    ret[0] = 1.0 / width;
    ret[5] = 1.0 / height;
    ret[11] = 1.0 / (-range * nearZ);
    ret[14] = 1.0;
    ret[15] = 1.0 / nearZ;

    return ret;
}

export function GetPerspectiveMatrixRH(fovY, aspectRatio, nearZ, farZ)
{
    const height = Math.cos(fovY / 2.0) / Math.sin(fovY / 2.0);
    const width = height / aspectRatio;
    const range = farZ / (nearZ - farZ);

    let ret = new Array(16).fill(0);

    ret[0] = width;
    ret[5] = height;
    ret[10] = range;
    ret[11] = -1.0;
    ret[14] = range * nearZ;

    return ret;
}

export function GetInvPerspectiveMatrixRH(fovY, aspectRatio, nearZ, farZ)
{
    const height = Math.cos(fovY / 2.0) / Math.sin(fovY / 2.0);
    const width = height / aspectRatio;
    const range = farZ / (nearZ - farZ);

    let ret = new Array(16).fill(0);

    ret[0] = 1.0 / width;
    ret[5] = 1.0 / height;
    ret[11] = 1.0 / (range * nearZ);
    ret[14] = -1.0;
    ret[15] = 1.0 / nearZ;

    return ret;
}

export function GetOrthoMatrixLH(width, height, nearZ, farZ)
{
    const range = 1.0 / (farZ - nearZ);

    let ret = new Array(16).fill(0);

    ret[0] = 2.0 / width;
    ret[5] = 2.0 / height;
    ret[10] = range;
    ret[14] = -range * nearZ;
    ret[15] = 1.0;

    return ret;
}

export function GetInvOrthoMatrixLH(width, height, nearZ, farZ)
{
    const range = 1.0 / (farZ - nearZ);

    let ret = new Array(16).fill(0);

    ret[0] = width / 2;
    ret[5] = height / 2;
    ret[10] = 1.0 / range;
    ret[14] = nearZ;
    ret[15] = 1.0;

    return ret;
}

export function GetOrthoMatrixRH(width, height, nearZ, farZ)
{
    const range = 1.0 / (nearZ - farZ);

    let ret = new Array(16).fill(0);

    ret[0] = 2.0 / width;
    ret[5] = 2.0 / height;
    ret[10] = range;
    ret[14] = range * nearZ;
    ret[15] = 1.0;

    return ret;
}

export function GetInvOrthoMatrixRH(width, height, nearZ, farZ)
{
    const range = 1.0 / (nearZ - farZ);

    let ret = new Array(16).fill(0);

    ret[0] = width / 2;
    ret[5] = height / 2;
    ret[10] = 1.0 / range;
    ret[14] = -nearZ;
    ret[15] = 1.0;

    return ret;
}

export function GetProjMatrix(fovDegrees, resolution, nearZ, farZ, reverseZ, perspective, leftHanded)
{
    let fov = fovDegrees * Math.PI / 180.0;

    nearZ = Math.max(nearZ, 0.001);
    farZ = Math.max(farZ, 0.001);
    fov = Math.max(fov, 0.001);

    if (nearZ == farZ)
        farZ = nearZ + 0.01;

    if (reverseZ)
    {
        let temp = nearZ;
        nearZ = farZ;
        farZ = temp;
    }

    if (perspective)
    {
        if (leftHanded)
            return GetPerspectiveMatrixLH(fov, resolution[0] / resolution[1], nearZ, farZ);
        else
            return GetPerspectiveMatrixRH(fov, resolution[0] / resolution[1], nearZ, farZ);
    }
    else
    {
        if (leftHanded)
            return GetOrthoMatrixLH(resolution[0], resolution[1], nearZ, farZ);
        else
            return GetOrthoMatrixRH(resolution[0], resolution[1], nearZ, farZ);
    }
}

export function GetInvProjMatrix(fovDegrees, resolution, nearZ, farZ, reverseZ, perspective, leftHanded)
{
    let fov = fovDegrees * Math.PI / 180.0;

    nearZ = Math.max(nearZ, 0.001);
    farZ = Math.max(farZ, 0.001);
    fov = Math.max(fov, 0.001);

    if (nearZ == farZ)
        farZ = nearZ + 0.01;

    if (reverseZ)
    {
        let temp = nearZ;
        nearZ = farZ;
        farZ = temp;
    }

    if (perspective)
    {
        if (leftHanded)
            return GetInvPerspectiveMatrixLH(fov, resolution[0] / resolution[1], nearZ, farZ);
        else
            return GetInvPerspectiveMatrixRH(fov, resolution[0] / resolution[1], nearZ, farZ);
    }
    else
    {
        if (leftHanded)
            return GetInvOrthoMatrixLH(resolution[0], resolution[1], nearZ, farZ);
        else
            return GetInvOrthoMatrixRH(resolution[0], resolution[1], nearZ, farZ);
    }
}

// -------------------- Utilities

export function Align(alignment, value)
{
    return (Math.floor((value + alignment - 1) / alignment) * alignment)
}

// Using info from https://gpuweb.github.io/gpuweb/#texture-format-caps
export function GetTextureFormatInfo(format)
{
    let ret = {};
    ret.isDepthStencil = false;
    ret.sampleType = "float";

    switch(format)
    {
        case "r8unorm":
        {
            ret.bytesPerPixel = 1;
            ret.sampleType = "float";
            break;
        }
        case "rgba8unorm":
        case "rgba8unorm-srgb":
        {
            ret.bytesPerPixel = 4;
            ret.sampleType = "float";
            break;
        }
        case "rgba8uint":
        {
            ret.bytesPerPixel = 4;
            ret.sampleType = "uint";
            break;
        }
        case "rgba8sint":
        {
            ret.bytesPerPixel = 4;
            ret.sampleType = "sint";
            break;
        }
        case "rgba16float":
        {
            ret.bytesPerPixel = 8;
            ret.sampleType = "float";
            break;
        }
        case "r32float":
        {
            ret.bytesPerPixel = 4;
            ret.sampleType = "float";
            //ret.sampleType = "unfilterable-float"; // we require filterable floats now
            break;
        }
        case "r32uint":
        {
            ret.bytesPerPixel = 4;
            ret.sampleType = "uint";
            break;
        }
        case "rg32uint":
        {
            ret.bytesPerPixel = 8;
            ret.sampleType = "uint";
            break;
        }
        case "rg32float":
        {
            ret.bytesPerPixel = 8;
            ret.sampleType = "float";
            break;
        }
        case "rgba32uint":
        {
            ret.bytesPerPixel = 16;
            ret.sampleType = "uint";
            break;
        }
        case "rgba32float":
        {
            ret.bytesPerPixel = 16;
            ret.sampleType = "float";
            break;
        }
        case "depth32float":
        {
            ret.bytesPerPixel = 4;
            ret.sampleType = "float";
            break;
        }
        case "depth24plus-stencil8":
        {
            ret.bytesPerPixel = 4;
            ret.isDepthStencil = true;
            ret.sampleType = "float";
            break;
        }
        default:
        {
            LogError("Unhandled format in GetTextureFormatInfo(): " + format);
        }
    }
    return ret;
}

export function GetNonSRGBFormat(format)
{
    switch (format)
    {
        case "rgba8unorm-srgb": return "rgba8unorm";
    }
    return format;
}

export function IsSRGBFormat(format)
{
    return format != GetNonSRGBFormat(format);
}

export function CopyCompatibleFormats(formatA, formatB)
{
    // https://www.w3.org/TR/webgpu/#copy-compatible states:
    // Two GPUTextureFormats format1 and format2 are copy-compatible if:
    // * format1 equals format2, or
    // * format1 and format2 differ only in whether they are srgb formats(have the - srgb suffix).
    return GetNonSRGBFormat(formatA) == GetNonSRGBFormat(formatB);
}

export function GetReadbackBuffer_FromBuffer(device, encoder, buffer)
{
    const readbackBuffer = device.createBuffer({
        label: "Readback of buffer " + buffer.label,
        size: buffer.size,
        usage: GPUBufferUsage.COPY_DST | GPUBufferUsage.MAP_READ,
    });
    encoder.copyBufferToBuffer(buffer, 0, readbackBuffer, 0, buffer.size);
    return readbackBuffer;
}

export function GetReadbackBuffer_FromTexture(device, encoder, texture, mipLevel = 0)
{
    const bytesPerPixel = GetTextureFormatInfo(texture.format).bytesPerPixel;

    const mipWidth = Math.max(texture.width >> mipLevel, 1);
    const mipHeight = Math.max(texture.height >> mipLevel, 1);

    let mipDepth = texture.depthOrArrayLayers;
    if (texture.dimension == "3d")
        mipDepth = Math.max(mipDepth >> mipLevel, 1);

    const mipPitchUnaligned = mipWidth * bytesPerPixel;
    const mipPitch = Align(256, mipPitchUnaligned);

    const readbackBuffer = device.createBuffer({
        label: "Readback of texture " + texture.label + " mip " + mipLevel.toString(),
        size: mipPitch * mipHeight * mipDepth,
        usage: GPUBufferUsage.COPY_DST | GPUBufferUsage.MAP_READ,
    });

    encoder.copyTextureToBuffer(
        { texture: texture, mipLevel: mipLevel },
        { buffer: readbackBuffer, bytesPerRow: mipPitch, rowsPerImage: mipHeight },
        {
            width: mipWidth,
            height: mipHeight,
            depthOrArrayLayers: mipDepth,
        }
    );

    return readbackBuffer;
}

export async function ReadbackBuffer(buffer, lambda)
{
    await buffer.mapAsync(GPUMapMode.READ);
    lambda(new DataView(buffer.getMappedRange()));
    buffer.unmap();
}

// Pass a DataView and StructOffset structs for srcType and dstType.
// This will copy the N items from the source buffer of srcType, into a buffer of N items of dstType.
// Returns it as an ArrayBuffer
export function CopyBufferByFieldNames(srcView, srcType, dstType)
{
    const numEntries = Math.floor(srcView.byteLength / srcType._size)

    let dst = new ArrayBuffer(numEntries * dstType._size);
    const dstView = new DataView(dst);

    for (let entryIndex = 0; entryIndex < numEntries; entryIndex++)
    {
        const srcBase = entryIndex * srcType._size;
        const dstBase = entryIndex * dstType._size;
        for (const key in dstType)
        {
            if (key.startsWith("_"))
                continue;

            const srcOffset = srcBase + srcType[key];
            const dstOffset = dstBase + dstType[key];

            dstView.setUint32(dstOffset, srcView.getUint32(srcOffset));
        }
    }
    return dst;
}

// -------------------- Logging

export function LogError(message) {
    console.log("\x1b[1;31m" + message + "\x1b[0m")
    throw new Error(message);
}

export function LogWarning(message) {
    console.log("\x1b[1;33m" + message + "\x1b[0m")
}

export function LogInfo(message) {
    console.log("\x1b[39m" + message + "\x1b[0m")
}

export function LogVerbose(message) {
    console.log("\x1b[2;39m" + message + "\x1b[0m")
}

export function LogSuccess(message) {
    console.log("\x1b[1;32m" + message + "\x1b[0m")
}

// -------------------- CopyTextureReinterpret

// This copies a texture to another texture, but re-interprets the values from the first texture as appropriate.
// Handling:
// 1) sRGB vs linear
// 2) type of value stored (float vs uint etc)
// 3) number of channels

let Utils_ShaderCode_VS_CopyTextureReinterpret = `
@vertex
fn main(
  @builtin(vertex_index) VertexIndex : u32
) -> @builtin(position) vec4f
{
    var pos = array<vec2f, 3>(
        vec2<f32>(-1.0, -1.0),
        vec2<f32>( 3.0, -1.0),
        vec2<f32>(-1.0,  3.0)
    );

  return vec4f(pos[VertexIndex], 0.0, 1.0);
}
`;

let Utils_ShaderCode_PS_CopyTextureReinterpret = `
@binding(0) @group(0) var Input : texture_2d<f32>;

@fragment
fn main(@builtin(position) fragCoord: vec4<f32>) -> @location(0) vec4f
{
  return textureLoad(Input, vec2<u32>(fragCoord.xy), 0);
}
`;

export function CopyTextureReinterpret(device, encoder, texSrc, texDst, srcFormat = texSrc.format, dstFormat = texDst.format)
{
    const bindGroupEntries =
    [
        {
            // Input
            binding: 0,
            visibility: GPUShaderStage.FRAGMENT,
            texture : { viewDimension: "2d" }
        },
    ];

    const bindGroupLayout = device.createBindGroupLayout({
        label: "CopyTextureReinterpret Bind Group Layout",
        entries: bindGroupEntries
    });

    const pipelineLayout = device.createPipelineLayout({
        label: "CopyTextureReinterpret Pipeline Layout",
        bindGroupLayouts: [bindGroupLayout],
    });

    const pipeline = device.createRenderPipeline({
        label: "CopyTextureReinterpret Pipeline",
        layout: pipelineLayout,
        vertex: {
          module: device.createShaderModule({
            code: Utils_ShaderCode_VS_CopyTextureReinterpret,
          }),
        },
        fragment: {
          module: device.createShaderModule({
            code: Utils_ShaderCode_PS_CopyTextureReinterpret,
          }),
          targets: [
            {
              format: dstFormat,
            },
          ],
        },
        primitive: {
          topology: 'triangle-list',
        },
    });

    const renderPassDescriptor = {
        label: "CopyTextureReinterpret Render Pass",
        colorAttachments: [
          {
            view: texDst.createView({ dimension: "2d" }),
            clearValue: [0, 0, 0, 0],
            loadOp: 'clear',
            storeOp: 'store',
          },
        ],
      };

    const bindGroup = device.createBindGroup({
        label: "CopyTextureReinterpret Bind Group",
        layout: bindGroupLayout,
        entries: [
            {
                binding: 0,
                resource: texSrc.createView({ dimension: "2d", format: srcFormat, usage: GPUTextureUsage.TEXTURE_BINDING }),
            },
        ],
    });

    const passEncoder = encoder.beginRenderPass(renderPassDescriptor);
        passEncoder.setPipeline(pipeline);
        passEncoder.setBindGroup(0, bindGroup);
        passEncoder.draw(3);
    passEncoder.end();
}

// -------------------- CopyTextureLinearToSRGB

// Compute Shader CopyTextureLinearToSRGB
let Utils_Hash_Compute_CopyTextureLinearToSRGB = 0;
let Utils_ShaderModule_Compute_CopyTextureLinearToSRGB = null;
let Utils_Pipeline_Compute_CopyTextureLinearToSRGB = null;
let Utils_BindGroupLayout_Compute_CopyTextureLinearToSRGB = null;
let Utils_PipelineLayout_Compute_CopyTextureLinearToSRGB = null;

let Utils_ShaderCode_CopyTextureLinearToSRGB = `
@binding(0) @group(0) var Src : texture_storage_2d</*(Src_format)*/, read>;
@binding(1) @group(0) var Dst : texture_storage_2d</*(Dst_format)*/, write>;

fn LinearToSRGB( linearCol : vec3<f32>) -> vec3<f32>
{
    var sRGBLo_0 : vec3<f32> = linearCol * vec3<f32>(12.92000007629394531f);
    var sRGBHi_0 : vec3<f32> = pow(abs(linearCol), vec3<f32>(0.4166666567325592f, 0.4166666567325592f, 0.4166666567325592f)) * vec3<f32>(1.0549999475479126f) - vec3<f32>(0.05499999970197678f);
    var sRGB_0 : vec3<f32>;
    var _S4 : f32;
    if((linearCol.x) <= 0.00313080009073019f)
    {
        _S4 = sRGBLo_0.x;
    }
    else
    {
        _S4 = sRGBHi_0.x;
    }
    sRGB_0[i32(0)] = _S4;
    if((linearCol.y) <= 0.00313080009073019f)
    {
        _S4 = sRGBLo_0.y;
    }
    else
    {
        _S4 = sRGBHi_0.y;
    }
    sRGB_0[i32(1)] = _S4;
    if((linearCol.z) <= 0.00313080009073019f)
    {
        _S4 = sRGBLo_0.z;
    }
    else
    {
        _S4 = sRGBHi_0.z;
    }
    sRGB_0[i32(2)] = _S4;
    return sRGB_0;
}

@compute
@workgroup_size(8, 8, 1)
fn csmain(@builtin(global_invocation_id) DTid : vec3<u32>)
{
    var colorLinear : vec3<f32> = textureLoad(Src, DTid.xy).xyz;
    var colorsRGB : vec3<f32> = LinearToSRGB(colorLinear);
    textureStore(Dst, DTid.xy, vec4<f32>(colorsRGB, 1.0));
    return;
}
`;

export function CopyTextureLinearToSRGB(device, encoder, texSrc, texDst)
{
    let width = Math.min(texSrc.width, texDst.width);
    let height = Math.min(texSrc.height, texDst.height);

    // (Re)create compute shader
    {
        const bindGroupEntries =
        [
            {
                // Src
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: texSrc.format, viewDimension: "2d" }
            },
            {
                // Dst
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: texDst.format, viewDimension: "2d" }
            },
        ];

        const newHash = JSON.stringify(bindGroupEntries).hashCode();

        if (Utils_ShaderModule_Compute_CopyTextureLinearToSRGB === null || newHash !== Utils_Hash_Compute_CopyTextureLinearToSRGB)
        {
            Utils_Hash_Compute_CopyTextureLinearToSRGB = newHash;

            let shaderCode = Utils_ShaderCode_CopyTextureLinearToSRGB;
            shaderCode = shaderCode.replace("/*(Src_format)*/", texSrc.format);
            shaderCode = shaderCode.replace("/*(Dst_format)*/", texDst.format);

            Utils_ShaderModule_Compute_CopyTextureLinearToSRGB = device.createShaderModule({ code: shaderCode, label: "Compute Shader CopyTextureLinearToSRGB"});
            Utils_BindGroupLayout_Compute_CopyTextureLinearToSRGB = device.createBindGroupLayout({
                label: "Compute Bind Group CopyTextureLinearToSRGB",
                entries: bindGroupEntries
            });

            Utils_PipelineLayout_Compute_CopyTextureLinearToSRGB = device.createPipelineLayout({
                label: "Compute Bind Group Layout CopyTextureLinearToSRGB",
                bindGroupLayouts: [Utils_BindGroupLayout_Compute_CopyTextureLinearToSRGB],
            });

            Utils_Pipeline_Compute_CopyTextureLinearToSRGB = device.createComputePipeline({
                label: "Compute Pipeline Layout CopyTextureLinearToSRGB",
                layout: Utils_PipelineLayout_Compute_CopyTextureLinearToSRGB,
                compute: {
                    module: Utils_ShaderModule_Compute_CopyTextureLinearToSRGB,
                    entryPoint: "csmain",
                }
            });
        }
    }

    // Run compute shader Initialize
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group CopyTextureLinearToSRGB",
            layout: Utils_BindGroupLayout_Compute_CopyTextureLinearToSRGB,
            entries: [
                {
                    // Src
                    binding: 0,
                    resource: texSrc.createView({ dimension: "2d" })
                },
                {
                    // Dst
                    binding: 1,
                    resource: texDst.createView({ dimension: "2d" })
                },
            ]
        });

        const baseDispatchSize = [width, height, 1];
        const dispatchSize = [
            Math.floor((Math.floor(((baseDispatchSize[0] + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((baseDispatchSize[1] + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((baseDispatchSize[2] + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        const computePass = encoder.beginComputePass();
            computePass.setPipeline(Utils_Pipeline_Compute_CopyTextureLinearToSRGB);
            computePass.setBindGroup(0, bindGroup);
            computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
        computePass.end();
    }
}
