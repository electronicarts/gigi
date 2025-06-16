import * as Shared from './Shared.js';

let fs;
if (Shared.isNode()) {
    await import('webgpu')
        .then((module) => { Object.assign(globalThis, module.globals); })
        .catch((error) => console.error('Error loading module:', error));

    fs = await import('node:fs');
}

// The standard console.log shows the whole buffer, not just the view
function LogDataViewU8(view, label)
{
    console.log(label + " = ")
    for (let i=0; i < view.byteLength; i++)
        console.log("[" + i + "] " + view.getUint8(i) + " (0x" + view.getUint8(i).toString(16) + ")");
    console.log("");
}

function LogDataViewF32(view, label, valuesPerRow = 1)
{
    console.log(label + " = ")
    for (let i = 0; i < view.byteLength; i += 4 * valuesPerRow)
    {
        let line = "";

        line += ("[" + i + "]");

        for(let j = 0; j < valuesPerRow; j++)
            line += " " + view.getFloat32(i + j * 4, true);

        console.log(line);
    }
    console.log("");
}

function LogDataViewU32(view, label, valuesPerRow = 1)
{
    console.log(label + " = ")
    for (let i = 0; i < view.byteLength; i += 4 * valuesPerRow)
    {
        let line = "";

        line += ("[" + i + "]");

        for(let j = 0; j < valuesPerRow; j++)
            line += " " + view.getUint32(i + j * 4, true);

        console.log(line);
    }
    console.log("");
}

function VerifyDataViewEqualsBinaryFile(dataView, dataViewName, fileName)
{
    if (!fs.existsSync(fileName))
    {
        Shared.SaveBin(fileName, dataView);
        Shared.LogError("file did not exist: " + fileName);
        return false;
    }

    const fileContents = fs.readFileSync(fileName);
    const fileView = new DataView(fileContents.buffer, fileContents.byteOffset, fileContents.byteLength);

    if (dataView.byteLength != fileView.byteLength)
    {
        Shared.LogError("\"" + dataViewName + "\" size mismatch vs " + fileName + "\n" + dataView.byteLength + " vs " + fileView.byteLength);
        return false;
    }

    const showDifferenceCount = 20;
    let bytesDifferent = 0;
    for (let i=0; i < dataView.byteLength; i++)
    {
        if (dataView.getUint8(i) !== fileView.getUint8(i))
        {
            if (bytesDifferent == 0)
                console.log("Byte Differences:\n");

            if (bytesDifferent == showDifferenceCount)
            {
                console.log("...\n");
            }
            else if (bytesDifferent < showDifferenceCount)
            {
                console.log("[" + i + "] " + dataView.getUint8(i).toString() + " vs " + fileView.getUint8(i).toString() + "     (0x" + dataView.getUint8(i).toString(16) + " vs 0x" + fileView.getUint8(i).toString(16) + ")");
            }

            bytesDifferent++;
        }
    }

    if (bytesDifferent > 0)
    {
        //LogDataViewU8(dataView, "dataView")
        //LogDataViewU8(fileView, "fileView")
        Shared.LogError("\"" + dataViewName + "\" data mismatch vs " + fileName + "\": " + bytesDifferent + " out of " + dataView.byteLength + " bytes\n");
        return false;
    }

    return true;
}

function UnpadTextureDataView(dataView_, texture, mipLevel = 0, zIndex = 0)
{
    const mipWidth = Math.max(texture.width >> mipLevel, 1);
    const mipHeight = Math.max(texture.height >> mipLevel, 1);

    const bytesPerPixel = Shared.GetTextureFormatInfo(texture.format).bytesPerPixel;
    const mipPitchUnaligned = mipWidth * bytesPerPixel;
    const mipPitch = Shared.Align(256, mipPitchUnaligned);

    const ZSliceBytes = mipPitch * mipHeight;
    let dataView = new DataView(dataView_.buffer, dataView_.byteOffset + ZSliceBytes * zIndex , ZSliceBytes);

    if (mipPitch == mipPitchUnaligned)
        return dataView;

    // Pitch is aligned to 256 bytes. If our image data rows are less than that we need to remove the padding
    let dataUnpadded = new Uint8Array(mipPitchUnaligned*mipHeight);

    for (let iy = 0; iy < mipHeight; ++iy)
    {
        const srcBase = iy * mipPitch;
        const dstBase = iy * mipPitchUnaligned;
        for (let ix = 0; ix < mipPitchUnaligned; ++ix)
            dataUnpadded[dstBase + ix] = dataView.getUint8(srcBase + ix);
    }

    let dataViewUnpadded = new DataView(dataUnpadded.buffer, dataUnpadded.byteOffset, dataUnpadded.byteLength);
    return dataViewUnpadded;
}

function VerifyDataViewEqualsPNGFile(dataView_, texture, fileName, mipLevel = 0, zIndex = 0)
{
    let dataViewUnpadded = UnpadTextureDataView(dataView_, texture, mipLevel, zIndex);

    if (!fs.existsSync(fileName))
    {
        const mipWidth = Math.max(texture.width >> mipLevel, 1);
        const mipHeight = Math.max(texture.height >> mipLevel, 1);

        Shared.SavePNG(fileName, mipWidth, mipHeight, dataViewUnpadded);
        Shared.LogError("file did not exist: " + fileName);
        return false;
    }

    var source = Shared.LoadPNG_Node(fileName).data;
    const fileView = new DataView(source.buffer);

    if (dataViewUnpadded.byteLength != fileView.byteLength)
    {
        Shared.LogError("\"" + texture.label + "\" size mismatch vs " + fileName + "\n" + dataViewUnpadded.byteLength + " vs " + fileView.byteLength);
        return false;
    }

    for (let i=0; i < dataViewUnpadded.byteLength; i++)
    {
        if (dataViewUnpadded.getUint8(i) !== fileView.getUint8(i))
        {
            Shared.LogError("\"" + texture.label + "\" data mismatch at byte " + i + " vs " + fileName + "\"\n" + dataViewUnpadded.getUint8(i).toString() + " vs " + fileView.getUint8(i).toString());
            return false;
        }
    }

    //Shared.LogSuccess("\"" + texture.label + "\" matched gold reference " + fileName);
    return true;
}

// ===================================================== TESTS BEGIN =====================================================

class Test_Barrier_BarrierTest
{
    async Init(device, module)
    {
        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Output_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Output);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Output_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Output, "../../../../../../Techniques/UnitTests/_GoldImages/Barrier/BarrierTest/0.png");
            }
        );
    }
};
export var test_Barrier_BarrierTest = new Test_Barrier_BarrierTest();

class Test_Buffers_StructuredBuffer
{
    async Init(device, module)
    {
        // Make sure we can copy the output buffers, for readback.
        module.buffer_buff_usageFlags = module.buffer_buff_usageFlags | GPUBufferUsage.COPY_SRC;
        module.constantBuffer__csmainCB_usageFlags = module.constantBuffer__csmainCB_usageFlags | GPUBufferUsage.COPY_SRC;

        // Create the buffer buff
        module.buffer_buff_count = 2;
        module.buffer_buff_stride = module.constructor.StructOffsets_TheStructure._size;

        const alignedBufferSize = Shared.Align(16, module.buffer_buff_count * module.buffer_buff_stride);

        module.buffer_buff = device.createBuffer({
            label: "StructuredBuffer.buff",
            size: alignedBufferSize,
            usage: module.buffer_buff_usageFlags,
        });

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
        module.variable_frameIndex = 11;
        module.variable_frameDeltaTime = 1.0 / 30.0;
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.buffer_buff_Readback = Shared.GetReadbackBuffer_FromBuffer(device, encoder, module.buffer_buff);
        this.constantBuffer__csmainCB_Readback = Shared.GetReadbackBuffer_FromBuffer(device, encoder, module.constantBuffer__csmainCB);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.buffer_buff_Readback,
            (view) =>
            {
                const tightView = new DataView(view.buffer, view.byteOffset, module.buffer_buff_count * module.buffer_buff_stride);
                const unpaddedData = Shared.CopyBufferByFieldNames(tightView, module.constructor.StructOffsets_TheStructure, module.constructor.StructOffsets_TheStructure_Unpadded);
                const unpaddedDataView = new DataView(unpaddedData);
                VerifyDataViewEqualsBinaryFile(unpaddedDataView, "buff", "../../../../../../Techniques/UnitTests/_GoldImages/Buffers/StructuredBuffer/0.bin");
            }
        );
        await Shared.ReadbackBuffer(this.constantBuffer__csmainCB_Readback,
            (view) =>
            {
                VerifyDataViewEqualsBinaryFile(view, "buff", "../../../../../../Techniques/UnitTests/_GoldImages/Buffers/StructuredBuffer/1.bin");
            }
        );
    }
};
export var test_Buffers_StructuredBuffer = new Test_Buffers_StructuredBuffer();

class Test_Buffers_buffertest_webgpu
{
    async Init(device, module)
    {
        // Create the InputTypedBuffer
        {
            const bufferCPU = new Float32Array(await Shared.LoadCSV('buffertest_typedbuffer.csv', item => item["value"]));

            module.buffer_InputTypedBuffer_count = bufferCPU.length;
            module.buffer_InputTypedBuffer_stride = 4;

            const alignedBufferSize = Shared.Align(16, module.buffer_InputTypedBuffer_count * module.buffer_InputTypedBuffer_stride);

            module.buffer_InputTypedBuffer = device.createBuffer({
                label: "module.InputTypedBuffer",
                size: alignedBufferSize,
                usage: module.buffer_InputTypedBuffer_usageFlags,
            });

            device.queue.writeBuffer(module.buffer_InputTypedBuffer, 0, bufferCPU);
        }

        // Create the InputStructuredBuffer
        {
            const csvData = await Shared.LoadCSV('buffertest_structuredbuffer.csv',
                item => [ item["TheFloat40"], item["TheFloat41"], item["TheFloat42"], item["TheFloat43"], item["TheInt40"], item["TheInt41"], item["TheInt42"], item["TheInt43"], item["TheBool"] ]
            );

            module.buffer_InputStructuredBuffer_count = csvData.length;
            module.buffer_InputStructuredBuffer_stride = module.constructor.StructOffsets_TestStruct._size;

            const alignedBufferSize = Shared.Align(16, module.buffer_InputStructuredBuffer_count * module.buffer_InputStructuredBuffer_stride);

            module.buffer_InputStructuredBuffer = device.createBuffer({
                label: "buffertest_webgpu.InputStructuredBuffer",
                size: alignedBufferSize,
                usage: module.buffer_InputStructuredBuffer_usageFlags,
            });

            const bufferCPU = new ArrayBuffer(alignedBufferSize);
            const view = new DataView(bufferCPU);

            csvData.forEach((value, index) =>
                {
                    const baseOffset = index * 36;
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_TestStruct.TheFloat4_0, value[0], true);
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_TestStruct.TheFloat4_1, value[1], true);
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_TestStruct.TheFloat4_2, value[2], true);
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_TestStruct.TheFloat4_3, value[3], true);
                    view.setInt32(baseOffset + module.constructor.StructOffsets_TestStruct.TheInt4_0, value[4], true);
                    view.setInt32(baseOffset + module.constructor.StructOffsets_TestStruct.TheInt4_1, value[5], true);
                    view.setInt32(baseOffset + module.constructor.StructOffsets_TestStruct.TheInt4_2, value[6], true);
                    view.setInt32(baseOffset + module.constructor.StructOffsets_TestStruct.TheInt4_3, value[7], true);
                    view.setUint32(baseOffset + module.constructor.StructOffsets_TestStruct.TheBool, Boolean(value[8]) === true ? 1 : 0, true);
                }
            );
            device.queue.writeBuffer(module.buffer_InputStructuredBuffer, 0, bufferCPU);
        }

        // Make sure we can copy the output buffers, for readback.
        module.buffer_OutputTypedBuffer_usageFlags = module.buffer_OutputTypedBuffer_usageFlags | GPUBufferUsage.COPY_SRC;
        module.buffer_OutputStructuredBuffer_usageFlags = module.buffer_OutputStructuredBuffer_usageFlags | GPUBufferUsage.COPY_SRC;

        // Set the variables
        module.variable_gain = 0.5;
        module.alpha1 = 1.0;
        module.alpha2 = 0.0;

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.buffer_OutputTypedBuffer_Readback = Shared.GetReadbackBuffer_FromBuffer(device, encoder, module.buffer_OutputTypedBuffer);
        this.buffer_OutputStructuredBuffer_Readback = Shared.GetReadbackBuffer_FromBuffer(device, encoder, module.buffer_OutputStructuredBuffer);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.buffer_OutputTypedBuffer_Readback,
            (view) =>
            {
                const tightView = new DataView(view.buffer, view.byteOffset, module.buffer_OutputTypedBuffer_count * module.buffer_OutputTypedBuffer_stride);
                VerifyDataViewEqualsBinaryFile(tightView, "OutputTypedBuffer", "../../../../../../Techniques/UnitTests/_GoldImages/Buffers/buffertest_webgpu/0.bin");
            }
        );

        await Shared.ReadbackBuffer(this.buffer_OutputStructuredBuffer_Readback,
            (view) =>
            {
                const tightView = new DataView(view.buffer, view.byteOffset, module.buffer_OutputStructuredBuffer_count * module.buffer_OutputStructuredBuffer_stride);

                // Remove padding, since the binary file doesn't have it.
                const unpaddedData = Shared.CopyBufferByFieldNames(view, module.constructor.StructOffsets_TestStruct, module.constructor.StructOffsets_TestStruct_Unpadded);
                const unpaddedDataView = new DataView(unpaddedData);

                VerifyDataViewEqualsBinaryFile(unpaddedDataView, "OutputStructuredBuffer", "../../../../../../Techniques/UnitTests/_GoldImages/Buffers/buffertest_webgpu/1.bin");
            }
        );
    }
};
export var test_Buffers_buffertest_webgpu = new Test_Buffers_buffertest_webgpu();

class Test_Compute_boxblur
{
    async Init(device, module)
    {
        this.cabinTex = await Shared.CreateTextureWithPNG(device, "cabinsmall.png", module.texture_InputTexture_usageFlags);

        module.texture_InputTexture_size = this.cabinTex.size;
        module.texture_InputTexture_format = this.cabinTex.format;
        module.texture_InputTexture = device.createTexture({
            label: "texture boxblur.texture_InputTexture",
            size: module.texture_InputTexture_size,
            format: Shared.GetNonSRGBFormat(module.texture_InputTexture_format),
            usage: module.texture_InputTexture_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });
        
        module.variable_enabled = true;
        module.variable_radius = 20;
        module.variable_sRGB = true;

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
        encoder.copyTextureToTexture(
            { texture: this.cabinTex.texture },
            { texture: module.texture_InputTexture },
            {
                width: Math.min(module.texture_InputTexture.width, this.cabinTex.texture.width),
                height: Math.min(module.texture_InputTexture.height, this.cabinTex.texture.height),
                depthOrArrayLayers: Math.min(module.texture_InputTexture.depthOrArrayLayers, this.cabinTex.texture.depthOrArrayLayers)
            }
        );
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        // Copy output to a readback buffer
        this.texture_InputTexture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_InputTexture);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        // Validate readback data
        await Shared.ReadbackBuffer(this.texture_InputTexture_Readback,
            (view) =>
            {
                //Shared.SavePNG("Out.png", module.texture_InputTexture.width, module.texture_InputTexture.height, view);
                VerifyDataViewEqualsPNGFile(view, module.texture_InputTexture, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Compute/boxblur/0.png");
            }
        );
    }
};
export var test_Compute_boxblur = new Test_Compute_boxblur();

class Test_Compute_BufferAtomics
{
    async Init(device, module)
    {
        module.buffer_TheBuffer_usageFlags = module.buffer_TheBuffer_usageFlags | GPUBufferUsage.COPY_SRC;

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        // Copy output to a readback buffer
        this.buffer_TheBuffer_Readback = Shared.GetReadbackBuffer_FromBuffer(device, encoder, module.buffer_TheBuffer);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        // Validate readback data
        await Shared.ReadbackBuffer(this.buffer_TheBuffer_Readback,
            (view) =>
            {
                const tightView = new DataView(view.buffer, view.byteOffset, module.buffer_TheBuffer_count * module.buffer_TheBuffer_stride);
                VerifyDataViewEqualsBinaryFile(tightView, "TheBuffer", "../../../../../../Techniques/UnitTests/_GoldImages/Compute/BufferAtomics/0.bin");
            }
        );
    }
};
export var test_Compute_BufferAtomics = new Test_Compute_BufferAtomics();


class Test_Compute_Defines
{
    async Init(device, module)
    {
        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        // Copy output to a readback buffer
        this.texture_Output_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Output);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        // Validate readback data
        await Shared.ReadbackBuffer(this.texture_Output_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Output, "../../../../../../Techniques/UnitTests/_GoldImages/Compute/Defines/0.png");
            }
        );
    }
};
export var test_Compute_Defines = new Test_Compute_Defines();

class Test_Compute_IndirectDispatch
{
    async Init(device, module)
    {
        module.variable_Dispatch_Count_1 = [50, 50, 1];
        module.variable_Dispatch_Count_2 = [100, 100, 1];
        module.variable_Second_Dispatch_Offset = 2;
        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Render_Target_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Render_Target);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Render_Target_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Render_Target, "../../../../../../Techniques/UnitTests/_GoldImages/Compute/IndirectDispatch/out.png");
            }
        );
    }
};
export var test_Compute_IndirectDispatch = new Test_Compute_IndirectDispatch();

class Test_Compute_ReadbackSequence
{
    async Init(device, module)
    {
        // Create imported texture "Output"
        module.texture_Output_size = [64, 64, 1];
        module.texture_Output_format = "rgba8unorm";
        module.texture_Output = device.createTexture({
                label: "ReadbackSequence.texture_Output",
                format: module.texture_Output_format,
                size: [ module.texture_Output_size[0], module.texture_Output_size[1] ],
                usage: module.texture_Output_usageFlags,
            });

        return 10; // Execute 10 times
    }

    PreExecute(device, encoder, module, frameIndex)
    {
        module.variable_frameIndex = frameIndex;
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Output_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Output);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Output_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Output, "../../../../../../Techniques/UnitTests/_GoldImages/Compute/ReadbackSequence/" + frameIndex + ".png");
            }
        );
    }
};
export var test_Compute_ReadbackSequence = new Test_Compute_ReadbackSequence();

class Test_Compute_simple
{
    async Init(device, module)
    {
        this.cabinTex = await Shared.CreateTextureWithPNG(device, "cabinsmall.png", module.texture_Input_usageFlags);

        module.texture_Input_size = this.cabinTex.size;
        module.texture_Input_format = this.cabinTex.format;
        module.texture_Input = device.createTexture({
            label: "texture simple.texture_Input",
            size: module.texture_Input_size,
            format: Shared.GetNonSRGBFormat(module.texture_Input_format),
            usage: module.texture_Input_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
        encoder.copyTextureToTexture(
            { texture: this.cabinTex.texture },
            { texture: module.texture_Input },
            {
                width: Math.min(module.texture_Input.width, this.cabinTex.texture.width),
                height: Math.min(module.texture_Input.height, this.cabinTex.texture.height),
                depthOrArrayLayers: Math.min(module.texture_Input.depthOrArrayLayers, this.cabinTex.texture.depthOrArrayLayers)
            }
        );
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Input_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Input);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Input_Readback,
            (view) =>
            {
                //Shared.SavePNG("_output.png", simple.texture_Input.width, simple.texture_Input.height, view);
                VerifyDataViewEqualsPNGFile(view, module.texture_Input, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Compute/simple/0.png");
            }
        );
    }
};
export var test_Compute_simple = new Test_Compute_simple();

class Test_Compute_SlangAutoDiff
{
    async Init(device, module)
    {
        // Create imported texture "Output"
        module.texture_Output_size = [512, 512, 1];
        module.texture_Output_format = "rgba8unorm-srgb";
        module.texture_Output = device.createTexture({
                label: "SlangAutoDiff.texture_Output",
                format: Shared.GetNonSRGBFormat(module.texture_Output_format),
                size: [ module.texture_Output_size[0], module.texture_Output_size[1] ],
                usage: module.texture_Output_usageFlags,
                viewFormats: ["rgba8unorm-srgb"],
            });

        // make sure we can copy buffer "Data"
        module.buffer_Data_usageFlags |= GPUBufferUsage.COPY_SRC;

        //module.variable_NumGaussians = 10;
        //module.variable_LearningRate = 0.1;
        //module.variable_MaximumStepSize = 0.01;
        //module.variable_UseBackwardAD = true;
        module.variable_QuantizeDisplay = false;
        //module.variable_Reset = true;
        //module.variable_initialized = false;

        //module.variable_FrameIndex = 0;
        //module.variable_iResolution[0] = 512.0;
        //module.variable_iResolution[1] = 512.0;
        //module.variable_iResolution[2] = 1.0;

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Output_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Output);
        this.buffer_Data_Readback = Shared.GetReadbackBuffer_FromBuffer(device, encoder, module.buffer_Data);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Output_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Output, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Compute/SlangAutoDiff/0.png");
            }
        );

        await Shared.ReadbackBuffer(this.buffer_Data_Readback,
            (view) =>
            {
                const tightView = new DataView(view.buffer, view.byteOffset, module.buffer_Data_count * module.buffer_Data_stride);
                VerifyDataViewEqualsBinaryFile(tightView, "Data", "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Compute/SlangAutoDiff/1.bin");
            }
        );
    }
};
export var test_Compute_SlangAutoDiff = new Test_Compute_SlangAutoDiff();

class Test_CopyResource_CopyResourceTest
{
    async Init(device, module)
    {
        // Load Source Texture
        {
            // Update usage flags so we can do readback
            module.texture_Source_Texture_usageFlags = module.texture_Source_Texture_usageFlags | GPUTextureUsage.RENDER_ATTACHMENT;
            module.texture_Texture_From_Texture_usageFlags = module.texture_Texture_From_Texture_usageFlags | GPUTextureUsage.COPY_SRC;
            module.texture_Texture_From_Buffer_usageFlags  = module.texture_Texture_From_Buffer_usageFlags  | GPUTextureUsage.COPY_SRC;
            module.buffer_Buffer_From_Texture_usageFlags   = module.buffer_Buffer_From_Texture_usageFlags   | GPUBufferUsage.COPY_SRC;
            module.buffer_Buffer_From_Buffer_usageFlags    = module.buffer_Buffer_From_Buffer_usageFlags    | GPUBufferUsage.COPY_SRC;

            this.imgTex = await Shared.CreateTextureWithPNG(device, "img.png", GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.TEXTURE_BINDING);

            module.texture_Source_Texture_size = this.imgTex.size;
            module.texture_Source_Texture_format = "rgba32float";
            module.texture_Source_Texture = device.createTexture({
                label: "texture CopyResourceTest.Source_Texture",
                size: module.texture_Source_Texture_size,
                format: Shared.GetNonSRGBFormat(module.texture_Source_Texture_format),
                usage: module.texture_Source_Texture_usageFlags
            });
        }

        // Load Source Buffer
        {
            const csvData = await Shared.LoadCSV('out.csv',
                item => [ item["R"], item["G"], item["B"], item["A"] ]
            );

            module.buffer_Source_Buffer_count = csvData.length;
            module.buffer_Source_Buffer_stride = 16; // 4 channels of floats; R,G,B,A

            const alignedBufferSize = Shared.Align(16, module.buffer_Source_Buffer_count * module.buffer_Source_Buffer_stride);

            module.buffer_Source_Buffer = device.createBuffer({
                label: "buffertest_webgpu.Source_Buffer",
                size: alignedBufferSize,
                usage: module.buffer_Source_Buffer_usageFlags,
            });

            const bufferCPU = new ArrayBuffer(alignedBufferSize);
            const view = new DataView(bufferCPU);

            csvData.forEach((value, index) =>
                {
                    const baseOffset = index * 16;
                    view.setFloat32(baseOffset + 0, value[0], true);
                    view.setFloat32(baseOffset + 4, value[1], true);
                    view.setFloat32(baseOffset + 8, value[2], true);
                    view.setFloat32(baseOffset + 12, value[3], true);
                }
            );
            device.queue.writeBuffer(module.buffer_Source_Buffer, 0, bufferCPU);
        }

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
        // Copy the source image into texture Source Texture
        Shared.CopyTextureReinterpret(device, encoder, this.imgTex.texture, module.texture_Source_Texture, "rgba8unorm-srgb", module.texture_Source_Texture.format);
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Texture_From_Texture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Texture_From_Texture);
        this.texture_Texture_From_Buffer_Readback  = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Texture_From_Buffer );

        this.texture_Buffer_From_Texture_Readback  = Shared.GetReadbackBuffer_FromBuffer(device, encoder, module.buffer_Buffer_From_Texture   );
        this.texture_Buffer_From_Buffer_Readback   = Shared.GetReadbackBuffer_FromBuffer(device, encoder, module.buffer_Buffer_From_Buffer    );
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        // Validate readback data
        // The "... From Texture" tests diverge in results from the viewer because we convert png to rgba32float differently using web gpu. we use a compute shader instead of cpu side logic.
        await Shared.ReadbackBuffer(this.texture_Texture_From_Texture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsBinaryFile(view, "Texture_From_Texture", "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/CopyResource/CopyResourceTest/0.bin");
            }
        );

        await Shared.ReadbackBuffer(this.texture_Texture_From_Texture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsBinaryFile(view, "Texture_From_Texture", "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/CopyResource/CopyResourceTest/2.bin");
            }
        );

        await Shared.ReadbackBuffer(this.texture_Texture_From_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsBinaryFile(view, "Texture_From_Buffer", "../../../../../../Techniques/UnitTests/_GoldImages/CopyResource/CopyResourceTest/1.bin");
            }
        );

        await Shared.ReadbackBuffer(this.texture_Buffer_From_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsBinaryFile(view, "Texture_From_Buffer", "../../../../../../Techniques/UnitTests/_GoldImages/CopyResource/CopyResourceTest/3.bin");
            }
        );
    }
};
export var test_CopyResource_CopyResourceTest = new Test_CopyResource_CopyResourceTest();

class Test_Raster_NoVertex_NoIndex_NoInstance
{
    async Init(device, module)
    {
        // Update usage flags so we can do readback
        module.texture_Color_Buffer_usageFlags = module.texture_Color_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;
        module.texture_Depth_Buffer_usageFlags = module.texture_Depth_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;

        // Set the camera to be the exact binary values used by the pother platforms
        let viewProjMtxSrc = [
            0x401A8279, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x401A8279, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0xB8D1BC76, 0x3DCAC5B4,
            0x00000000, 0x00000000, 0x3F800000, 0x41200000
        ];
        const viewProjMtx = new ArrayBuffer(64);
        const viewProjMtxView = new DataView(viewProjMtx);
        for (let i = 0; i < 16; ++i)
        {
            viewProjMtxView.setUint32(i*4, viewProjMtxSrc[i], true);
            module.variable_ViewProjMtx[i] = viewProjMtxView.getFloat32(i*4, true);
        }
        module.variable_ViewProjMtx = Shared.MatTranspose(module.variable_ViewProjMtx);

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Depth_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Depth_Buffer);
        this.texture_Color_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Color_Buffer);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Depth_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsBinaryFile(view, "Depth_Buffer", "../../../../../../Techniques/UnitTests/_GoldImages/Raster/NoVertex_NoIndex_NoInstance/0.bin");
            }
        );

        await Shared.ReadbackBuffer(this.texture_Color_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Color_Buffer, "../../../../../../Techniques/UnitTests/_GoldImages/Raster/NoVertex_NoIndex_NoInstance/1.png");
            }
        );
    }
};
export var test_Raster_NoVertex_NoIndex_NoInstance = new Test_Raster_NoVertex_NoIndex_NoInstance();

class Test_Raster_simpleRaster
{
    async Init(device, module)
    {
        // make sure we can read back the things we want to read back
        module.texture_Color_Buffer_usageFlags = module.texture_Color_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;
        module.texture_Depth_Buffer_usageFlags = module.texture_Depth_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;

        // Load the vertex buffer
        let simpleRasterVBUnpadded;
        if (!await Shared.LoadBin("simpleRasterVB.bin", (data) => simpleRasterVBUnpadded = data ))
        {
            Shared.LogError("Could not load simpleRasterVB.bin");
        }
        const simpleRasterVB = Shared.CopyBufferByFieldNames(new DataView(simpleRasterVBUnpadded), module.constructor.StructOffsets_VertexFormat_Unpadded, module.constructor.StructOffsets_VertexFormat);

        // Create the buffer
        module.buffer_VertexBuffer = device.createBuffer({
            label: "simpleRaster.buffer_VertexBuffer",
            size: simpleRasterVB.byteLength,
            usage: module.buffer_VertexBuffer_usageFlags,
        });

        // Write the data to the buffer
        device.queue.writeBuffer(module.buffer_VertexBuffer, 0, simpleRasterVB);

        // Set the vertex buffer attributes
        module.buffer_VertexBuffer_stride = module.constructor.StructOffsets_VertexFormat._size;
        module.buffer_VertexBuffer_count = Math.floor(simpleRasterVB.byteLength / module.buffer_VertexBuffer_stride);
        module.buffer_VertexBuffer_vertexBufferAttributes = module.constructor.StructVertexBufferAttributes_VertexFormat;

        // Set the camera to be the exact binary values used by the pother platforms
        let viewProjMtxSrc = [
            0xBFFD3CFB, 0x00000000, 0xBFB118D6, 0xBF7C1BFF,
            0xBDA6AEC6, 0x401A3DFA, 0x3DEE58AD, 0xBFE3F7B9,
            0xB86FFB0C, 0xB6C5670B, 0x38AB943C, 0x3DCC21FA,
            0x3F12753A, 0x3D70F21E, 0xBF516D16, 0x4056E7BA
        ];
        const viewProjMtx = new ArrayBuffer(64);
        const viewProjMtxView = new DataView(viewProjMtx);
        for (let i = 0; i < 16; ++i)
        {
            viewProjMtxView.setUint32(i*4, viewProjMtxSrc[i], true);
            module.variable_ViewProjMtx[i] = viewProjMtxView.getFloat32(i*4, true);
        }
        module.variable_ViewProjMtx = Shared.MatTranspose(module.variable_ViewProjMtx);

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Depth_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Depth_Buffer);
        this.texture_Color_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Color_Buffer);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Depth_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsBinaryFile(view, "Depth_Buffer", "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Raster/simpleRaster/0.bin");
            }
        );

        await Shared.ReadbackBuffer(this.texture_Color_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Color_Buffer, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Raster/simpleRaster/1.png");
            }
        );
    }
};
export var test_Raster_simpleRaster = new Test_Raster_simpleRaster();

class Test_Raster_simpleRaster2
{
    async Init(device, module)
    {
        // make sure we can read back the things we want to read back
        module.texture_Color_Buffer_usageFlags = module.texture_Color_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;
        module.texture_Depth_Buffer_usageFlags = module.texture_Depth_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;

        // Load the vertex buffer
        let simpleRasterVBUnpadded;
        if (!await Shared.LoadBin("simpleRasterVB.bin", (data) => simpleRasterVBUnpadded = data ))
        {
            Shared.LogError("Could not load simpleRasterVB.bin");
        }
        const simpleRasterVB = Shared.CopyBufferByFieldNames(new DataView(simpleRasterVBUnpadded), module.constructor.StructOffsets_VertexFormat_Unpadded, module.constructor.StructOffsets_VertexFormat);

        // Create the buffer
        module.buffer_VertexBuffer = device.createBuffer({
            label: "simpleRaster2.buffer_VertexBuffer",
            size: simpleRasterVB.byteLength,
            usage: module.buffer_VertexBuffer_usageFlags,
        });

        // Write the data to the buffer
        device.queue.writeBuffer(module.buffer_VertexBuffer, 0, simpleRasterVB);

        // Set the vertex buffer attributes
        module.buffer_VertexBuffer_stride = module.constructor.StructOffsets_VertexFormat._size;
        module.buffer_VertexBuffer_count = Math.floor(simpleRasterVB.byteLength / module.buffer_VertexBuffer_stride);
        module.buffer_VertexBuffer_vertexBufferAttributes = module.constructor.StructVertexBufferAttributes_VertexFormat;

        // Set the camera to be the exact binary values used by the pother platforms
        let viewProjMtxSrc = [
            0xBFFD3CFB, 0x00000000, 0xBFB118D6, 0xBF7C1BFF,
            0xBDA6AEC6, 0x401A3DFA, 0x3DEE58AD, 0xBFE3F7B9,
            0xB86FFB0C, 0xB6C5670B, 0x38AB943C, 0x3DCC21FA,
            0x3F12753A, 0x3D70F21E, 0xBF516D16, 0x4056E7BA
        ];
        const viewProjMtx = new ArrayBuffer(64);
        const viewProjMtxView = new DataView(viewProjMtx);
        for (let i = 0; i < 16; ++i)
        {
            viewProjMtxView.setUint32(i*4, viewProjMtxSrc[i], true);
            module.variable_ViewProjMtx[i] = viewProjMtxView.getFloat32(i*4, true);
        }
        module.variable_ViewProjMtx = Shared.MatTranspose(module.variable_ViewProjMtx);

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Depth_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Depth_Buffer);
        this.texture_Color_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Color_Buffer);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Depth_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsBinaryFile(view, "Depth_Buffer", "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Raster/simpleRaster2/0.bin");
            }
        );

        await Shared.ReadbackBuffer(this.texture_Color_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Color_Buffer, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Raster/simpleRaster2/1.png");
            }
        );
    }
};
export var test_Raster_simpleRaster2 = new Test_Raster_simpleRaster2();

class Test_Raster_simpleRasterInSubgraph
{
    async Init(device, module)
    {
        // make sure we can read back the things we want to read back
        module.texture_DoSimpleRaster_Color_Buffer_usageFlags = module.texture_DoSimpleRaster_Color_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;
        module.texture_DoSimpleRaster_Depth_Buffer_usageFlags = module.texture_DoSimpleRaster_Depth_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;

        // Load the vertex buffer
        let simpleRasterVBUnpadded;
        if (!await Shared.LoadBin("simpleRasterVB.bin", (data) => simpleRasterVBUnpadded = data ))
        {
            Shared.LogError("Could not load simpleRasterVB.bin");
        }
        const simpleRasterVB = Shared.CopyBufferByFieldNames(new DataView(simpleRasterVBUnpadded), module.constructor.StructOffsets_DoSimpleRaster_VertexFormat_Unpadded, module.constructor.StructOffsets_DoSimpleRaster_VertexFormat);

        // Create the buffer
        module.buffer_VB = device.createBuffer({
            label: "simpleRasterInSubgraph.buffer_VB",
            size: simpleRasterVB.byteLength,
            usage: module.buffer_VB_usageFlags,
        });

        // Write the data to the buffer
        device.queue.writeBuffer(module.buffer_VB, 0, simpleRasterVB);

        // Set the vertex buffer attributes
        module.buffer_VB_stride = module.constructor.StructOffsets_DoSimpleRaster_VertexFormat._size;
        module.buffer_VB_count = Math.floor(simpleRasterVB.byteLength / module.buffer_VB_stride);
        module.buffer_VB_vertexBufferAttributes = module.constructor.StructVertexBufferAttributes_DoSimpleRaster_VertexFormat;

        // Set the camera to be the exact binary values used by the pother platforms
        let viewProjMtxSrc = [
            0xBFFD3CFB, 0x00000000, 0xBFB118D6, 0xBF7C1BFF,
            0xBDA6AEC6, 0x401A3DFA, 0x3DEE58AD, 0xBFE3F7B9,
            0xB86FFB0C, 0xB6C5670B, 0x38AB943C, 0x3DCC21FA,
            0x3F12753A, 0x3D70F21E, 0xBF516D16, 0x4056E7BA
        ];
        const viewProjMtx = new ArrayBuffer(64);
        const viewProjMtxView = new DataView(viewProjMtx);
        for (let i = 0; i < 16; ++i)
        {
            viewProjMtxView.setUint32(i*4, viewProjMtxSrc[i], true);
            module.variable_DoSimpleRaster_ViewProjMtx[i] = viewProjMtxView.getFloat32(i*4, true);
        }
        module.variable_DoSimpleRaster_ViewProjMtx = Shared.MatTranspose(module.variable_DoSimpleRaster_ViewProjMtx);

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Depth_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_DoSimpleRaster_Depth_Buffer);
        this.texture_Color_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_DoSimpleRaster_Color_Buffer);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Depth_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsBinaryFile(view, "Depth_Buffer", "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Raster/simpleRasterInSubgraph/0.bin");
            }
        );

        await Shared.ReadbackBuffer(this.texture_Color_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_DoSimpleRaster_Color_Buffer, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Raster/simpleRasterInSubgraph/1.png");
            }
        );
    }
};
export var test_Raster_simpleRasterInSubgraph = new Test_Raster_simpleRasterInSubgraph();

class Test_Raster_simpleRaster_Lines
{
    async Init(device, module)
    {
        // make sure we can read back the things we want to read back
        module.texture_Color_Buffer_usageFlags = module.texture_Color_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;
        module.texture_Depth_Buffer_usageFlags = module.texture_Depth_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;

        // Load the vertex buffer
        let simpleRasterVBUnpadded;
        if (!await Shared.LoadBin("simpleRasterVB.bin", (data) => simpleRasterVBUnpadded = data ))
        {
            Shared.LogError("Could not load simpleRasterVB.bin");
        }
        const simpleRasterVB = Shared.CopyBufferByFieldNames(new DataView(simpleRasterVBUnpadded), module.constructor.StructOffsets_VertexFormat_Unpadded, module.constructor.StructOffsets_VertexFormat);

        // Create the buffer
        module.buffer_VertexBuffer = device.createBuffer({
            label: "simpleRaster.buffer_VertexBuffer",
            size: simpleRasterVB.byteLength,
            usage: module.buffer_VertexBuffer_usageFlags,
        });

        // Write the data to the buffer
        device.queue.writeBuffer(module.buffer_VertexBuffer, 0, simpleRasterVB);

        // Set the vertex buffer attributes
        module.buffer_VertexBuffer_stride = module.constructor.StructOffsets_VertexFormat._size;
        module.buffer_VertexBuffer_count = Math.floor(simpleRasterVB.byteLength / module.buffer_VertexBuffer_stride);
        module.buffer_VertexBuffer_vertexBufferAttributes = module.constructor.StructVertexBufferAttributes_VertexFormat;

        // Set the camera to be the exact binary values used by the pother platforms
        let viewProjMtxSrc = [
            0xBFFD3CFB, 0x00000000, 0xBFB118D6, 0xBF7C1BFF,
            0xBDA6AEC6, 0x401A3DFA, 0x3DEE58AD, 0xBFE3F7B9,
            0xB86FFB0C, 0xB6C5670B, 0x38AB943C, 0x3DCC21FA,
            0x3F12753A, 0x3D70F21E, 0xBF516D16, 0x4056E7BA
        ];
        const viewProjMtx = new ArrayBuffer(64);
        const viewProjMtxView = new DataView(viewProjMtx);
        for (let i = 0; i < 16; ++i)
        {
            viewProjMtxView.setUint32(i*4, viewProjMtxSrc[i], true);
            module.variable_ViewProjMtx[i] = viewProjMtxView.getFloat32(i*4, true);
        }
        module.variable_ViewProjMtx = Shared.MatTranspose(module.variable_ViewProjMtx);

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Depth_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Depth_Buffer);
        this.texture_Color_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Color_Buffer);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Depth_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsBinaryFile(view, "Depth_Buffer", "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Raster/simpleRaster_Lines/0.bin");
            }
        );

        await Shared.ReadbackBuffer(this.texture_Color_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Color_Buffer, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Raster/simpleRaster_Lines/1.png");
            }
        );
    }
};
export var test_Raster_simpleRaster_Lines = new Test_Raster_simpleRaster_Lines();

class Test_Raster_simpleRaster_Points
{
    async Init(device, module)
    {
        // make sure we can read back the things we want to read back
        module.texture_Color_Buffer_usageFlags = module.texture_Color_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;
        module.texture_Depth_Buffer_usageFlags = module.texture_Depth_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;

        // Load the vertex buffer
        let simpleRasterVBUnpadded;
        if (!await Shared.LoadBin("simpleRasterVB.bin", (data) => simpleRasterVBUnpadded = data ))
        {
            Shared.LogError("Could not load simpleRasterVB.bin");
        }
        const simpleRasterVB = Shared.CopyBufferByFieldNames(new DataView(simpleRasterVBUnpadded), module.constructor.StructOffsets_VertexFormat_Unpadded, module.constructor.StructOffsets_VertexFormat);

        // Create the buffer
        module.buffer_VertexBuffer = device.createBuffer({
            label: "simpleRaster.buffer_VertexBuffer",
            size: simpleRasterVB.byteLength,
            usage: module.buffer_VertexBuffer_usageFlags,
        });

        // Write the data to the buffer
        device.queue.writeBuffer(module.buffer_VertexBuffer, 0, simpleRasterVB);

        // Set the vertex buffer attributes
        module.buffer_VertexBuffer_stride = module.constructor.StructOffsets_VertexFormat._size;
        module.buffer_VertexBuffer_count = Math.floor(simpleRasterVB.byteLength / module.buffer_VertexBuffer_stride);
        module.buffer_VertexBuffer_vertexBufferAttributes = module.constructor.StructVertexBufferAttributes_VertexFormat;

        // Set the camera to be the exact binary values used by the pother platforms
        let viewProjMtxSrc = [
            0xBFFD3CFB, 0x00000000, 0xBFB118D6, 0xBF7C1BFF,
            0xBDA6AEC6, 0x401A3DFA, 0x3DEE58AD, 0xBFE3F7B9,
            0xB86FFB0C, 0xB6C5670B, 0x38AB943C, 0x3DCC21FA,
            0x3F12753A, 0x3D70F21E, 0xBF516D16, 0x4056E7BA
        ];
        const viewProjMtx = new ArrayBuffer(64);
        const viewProjMtxView = new DataView(viewProjMtx);
        for (let i = 0; i < 16; ++i)
        {
            viewProjMtxView.setUint32(i*4, viewProjMtxSrc[i], true);
            module.variable_ViewProjMtx[i] = viewProjMtxView.getFloat32(i*4, true);
        }
        module.variable_ViewProjMtx = Shared.MatTranspose(module.variable_ViewProjMtx);

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Depth_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Depth_Buffer);
        this.texture_Color_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Color_Buffer);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Depth_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsBinaryFile(view, "Depth_Buffer", "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Raster/simpleRaster_Points/0.bin");
            }
        );

        await Shared.ReadbackBuffer(this.texture_Color_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Color_Buffer, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Raster/simpleRaster_Points/1.png");
            }
        );
    }
};
export var test_Raster_simpleRaster_Points = new Test_Raster_simpleRaster_Points();

class Test_Raster_Stencil
{
    async Init(device, module)
    {
        // make sure we can read back the things we want to read back
        module.texture_Color_usageFlags = module.texture_Color_usageFlags | GPUTextureUsage.COPY_SRC;

        // Load the vertex buffer
        let stencilVBUnpadded;
        if (!await Shared.LoadBin("StencilVB.bin", (data) => stencilVBUnpadded = data ))
        {
            Shared.LogError("Could not load StencilVB.bin");
        }
        const stencilVB = Shared.CopyBufferByFieldNames(new DataView(stencilVBUnpadded), module.constructor.StructOffsets_VertexBuffer_Unpadded, module.constructor.StructOffsets_VertexBuffer);

        // Create the buffer
        module.buffer_Vertex_Buffer = device.createBuffer({
            label: "Stencil.buffer_Vertex_Buffer",
            size: stencilVB.byteLength,
            usage: module.buffer_Vertex_Buffer_usageFlags,
        });

        // Write the data to the buffer
        device.queue.writeBuffer(module.buffer_Vertex_Buffer, 0, stencilVB);

        // Set the vertex buffer attributes
        module.buffer_Vertex_Buffer_stride = module.constructor.StructOffsets_VertexBuffer._size;
        module.buffer_Vertex_Buffer_count = Math.floor(stencilVB.byteLength / module.buffer_Vertex_Buffer_stride);
        module.buffer_Vertex_Buffer_vertexBufferAttributes = module.constructor.StructVertexBufferAttributes_VertexBuffer;

        // Set the camera to be the exact binary values used by the pother platforms
        let viewProjMtxSrc = [
            0x401A8279, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x401A8279, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0xB8D1BC76, 0x3DCBCBDF,
            0x00000000, 0x00000000, 0x3F800000, 0x40A00000
        ];
        const viewProjMtx = new ArrayBuffer(64);
        const viewProjMtxView = new DataView(viewProjMtx);
        for (let i = 0; i < 16; ++i)
        {
            viewProjMtxView.setUint32(i*4, viewProjMtxSrc[i], true);
            module.variable_ViewProjMtx[i] = viewProjMtxView.getFloat32(i*4, true);
        }
        module.variable_ViewProjMtx = Shared.MatTranspose(module.variable_ViewProjMtx);

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Color_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Color);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Color_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Color, "../../../../../../Techniques/UnitTests/_GoldImages/Raster/Stencil/0.png");
            }
        );
    }
};
export var test_Raster_Stencil = new Test_Raster_Stencil();

class Test_Raster_YesVertexStruct_NoIndex_NoInstance
{
    async Init(device, module)
    {
        // make sure we can read back the things we want to read back
        module.texture_Color_Buffer_usageFlags = module.texture_Color_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;
        module.texture_Depth_Buffer_usageFlags = module.texture_Depth_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;

        // Load the vertex buffer
        let stencilVBUnpadded;
        if (!await Shared.LoadBin("StencilVB.bin", (data) => stencilVBUnpadded = data ))
        {
            Shared.LogError("Could not load StencilVB.bin");
        }
        const stencilVB = Shared.CopyBufferByFieldNames(new DataView(stencilVBUnpadded), module.constructor.StructOffsets_VertexFormat_Unpadded, module.constructor.StructOffsets_VertexFormat);

        // Create the buffer
        module.buffer_Vertex_Buffer = device.createBuffer({
            label: "YesVertexStruct_NoIndex_NoInstance.buffer_Vertex_Buffer",
            size: stencilVB.byteLength,
            usage: module.buffer_Vertex_Buffer_usageFlags,
        });

        // Write the data to the buffer
        device.queue.writeBuffer(module.buffer_Vertex_Buffer, 0, stencilVB);

        // Set the vertex buffer attributes
        module.buffer_Vertex_Buffer_stride = module.constructor.StructOffsets_VertexFormat._size;
        module.buffer_Vertex_Buffer_count = Math.floor(stencilVB.byteLength / module.buffer_Vertex_Buffer_stride);
        module.buffer_Vertex_Buffer_vertexBufferAttributes = module.constructor.StructVertexBufferAttributes_VertexFormat;

        // Set the camera to be the exact binary values used by the pother platforms
        let viewProjMtxSrc = [
            0x401A8279, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x401A8279, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0xB8D1BC76, 0x3DCAC5B4,
            0x00000000, 0x00000000, 0x3F800000, 0x41200000
        ];
        const viewProjMtx = new ArrayBuffer(64);
        const viewProjMtxView = new DataView(viewProjMtx);
        for (let i = 0; i < 16; ++i)
        {
            viewProjMtxView.setUint32(i*4, viewProjMtxSrc[i], true);
            module.variable_ViewProjMtx[i] = viewProjMtxView.getFloat32(i*4, true);
        }
        module.variable_ViewProjMtx = Shared.MatTranspose(module.variable_ViewProjMtx);

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Depth_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Depth_Buffer);
        this.texture_Color_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Color_Buffer);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Depth_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsBinaryFile(view, "Depth_Buffer", "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Raster/YesVertexStruct_NoIndex_NoInstance/0.bin");
            }
        );

        await Shared.ReadbackBuffer(this.texture_Color_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Color_Buffer, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Raster/YesVertexStruct_NoIndex_NoInstance/1.png");
            }
        );
    }
};
export var test_Raster_YesVertexStruct_NoIndex_NoInstance = new Test_Raster_YesVertexStruct_NoIndex_NoInstance();

class Test_Raster_YesVertexStruct_NoIndex_YesInstanceStruct
{
    async Init(device, module)
    {
        // make sure we can read back the things we want to read back
        module.texture_Color_Buffer_usageFlags = module.texture_Color_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;
        module.texture_Depth_Buffer_usageFlags = module.texture_Depth_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;

        // Load the vertex buffer
        let stencilVBUnpadded;
        if (!await Shared.LoadBin("StencilVB.bin", (data) => stencilVBUnpadded = data ))
        {
            Shared.LogError("Could not load StencilVB.bin");
        }
        const stencilVB = Shared.CopyBufferByFieldNames(new DataView(stencilVBUnpadded), module.constructor.StructOffsets_VertexFormat_Unpadded, module.constructor.StructOffsets_VertexFormat);

        // Create the buffer
        module.buffer_Vertex_Buffer = device.createBuffer({
            label: "YesVertexStruct_NoIndex_YesInstanceStruct.buffer_Vertex_Buffer",
            size: stencilVB.byteLength,
            usage: module.buffer_Vertex_Buffer_usageFlags,
        });

        // Write the data to the buffer
        device.queue.writeBuffer(module.buffer_Vertex_Buffer, 0, stencilVB);

        // Set the vertex buffer attributes
        module.buffer_Vertex_Buffer_stride = module.constructor.StructOffsets_VertexFormat._size;
        module.buffer_Vertex_Buffer_count = Math.floor(stencilVB.byteLength / module.buffer_Vertex_Buffer_stride);
        module.buffer_Vertex_Buffer_vertexBufferAttributes = module.constructor.StructVertexBufferAttributes_VertexFormat;

        // Load the instance buffer
        let instanceBufferData;
        if (!await Shared.LoadBin("YesVertexStruct_NoIndex_YesInstanceStruct_InstanceBuffer.bin", (data) => instanceBufferData = data ))
        {
            Shared.LogError("Could not load YesVertexStruct_NoIndex_YesInstanceStruct_InstanceBuffer.bin");
        }

        // Create the buffer
        module.buffer_Instance_Buffer = device.createBuffer({
            label: "YesVertexStruct_NoIndex_YesInstanceStruct.buffer_Vertex_Buffer",
            size: instanceBufferData.byteLength,
            usage: module.buffer_Instance_Buffer_usageFlags,
        });

        // Write the data to the buffer
        device.queue.writeBuffer(module.buffer_Instance_Buffer, 0, instanceBufferData);

        // Set the instance buffer attributes
        module.buffer_Instance_Buffer_stride = module.constructor.StructOffsets_InstanceBufferFormat._size;
        module.buffer_Instance_Buffer_count = Math.floor(instanceBufferData.byteLength / module.buffer_Instance_Buffer_stride);
        module.buffer_Instance_Buffer_vertexBufferAttributes = module.constructor.StructVertexBufferAttributes_InstanceBufferFormat;

        // Set the camera to be the exact binary values used by the pother platforms
        let viewProjMtxSrc = [
            0x401A8279, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x401A8279, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0xB8D1BC76, 0x3DCAC5B4,
            0x00000000, 0x00000000, 0x3F800000, 0x41200000
        ];
        const viewProjMtx = new ArrayBuffer(64);
        const viewProjMtxView = new DataView(viewProjMtx);
        for (let i = 0; i < 16; ++i)
        {
            viewProjMtxView.setUint32(i*4, viewProjMtxSrc[i], true);
            module.variable_ViewProjMtx[i] = viewProjMtxView.getFloat32(i*4, true);
        }
        module.variable_ViewProjMtx = Shared.MatTranspose(module.variable_ViewProjMtx);

        // manually fix up the @location of the vertex and index buffers to match what the vertex shader says they should be
        module.buffer_Vertex_Buffer_vertexBufferAttributes[0].shaderLocation = 0;  // position
        module.buffer_Vertex_Buffer_vertexBufferAttributes[1].shaderLocation = 3;  // normal
        module.buffer_Vertex_Buffer_vertexBufferAttributes[2].shaderLocation = 4;  // uv
        module.buffer_Instance_Buffer_vertexBufferAttributes[0].shaderLocation = 1; // offset
        module.buffer_Instance_Buffer_vertexBufferAttributes[1].shaderLocation = 2; // scale

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Depth_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Depth_Buffer);
        this.texture_Color_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Color_Buffer);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Depth_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsBinaryFile(view, "Depth_Buffer", "../../../../../../Techniques/UnitTests/_GoldImages/Raster/YesVertexStruct_NoIndex_YesInstanceStruct/0.bin");
            }
        );

        await Shared.ReadbackBuffer(this.texture_Color_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Color_Buffer, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Raster/YesVertexStruct_NoIndex_YesInstanceStruct/1.png");
            }
        );
    }
};
export var test_Raster_YesVertexStruct_NoIndex_YesInstanceStruct = new Test_Raster_YesVertexStruct_NoIndex_YesInstanceStruct();

class Test_Raster_YesVertexStruct_NoIndex_YesInstanceType
{
    async Init(device, module)
    {
        // make sure we can read back the things we want to read back
        module.texture_Color_Buffer_usageFlags = module.texture_Color_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;
        module.texture_Depth_Buffer_usageFlags = module.texture_Depth_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;

        // Load the vertex buffer
        let stencilVBUnpadded;
        if (!await Shared.LoadBin("StencilVB.bin", (data) => stencilVBUnpadded = data ))
        {
            Shared.LogError("Could not load StencilVB.bin");
        }
        const stencilVB = Shared.CopyBufferByFieldNames(new DataView(stencilVBUnpadded), module.constructor.StructOffsets_VertexFormat_Unpadded, module.constructor.StructOffsets_VertexFormat);

        // Create the buffer
        module.buffer_Vertex_Buffer = device.createBuffer({
            label: "YesVertexStruct_NoIndex_YesInstanceType.buffer_Vertex_Buffer",
            size: stencilVB.byteLength,
            usage: module.buffer_Vertex_Buffer_usageFlags,
        });

        // Write the data to the buffer
        device.queue.writeBuffer(module.buffer_Vertex_Buffer, 0, stencilVB);

        // Set the vertex buffer attributes
        module.buffer_Vertex_Buffer_stride = module.constructor.StructOffsets_VertexFormat._size;
        module.buffer_Vertex_Buffer_count = Math.floor(stencilVB.byteLength / module.buffer_Vertex_Buffer_stride);
        module.buffer_Vertex_Buffer_vertexBufferAttributes = module.constructor.StructVertexBufferAttributes_VertexFormat;

        // Load the instance buffer
        let instanceBufferData;
        if (!await Shared.LoadBin("YesVertexStruct_NoIndex_YesInstanceType_InstanceBuffer.bin", (data) => instanceBufferData = data ))
        {
            Shared.LogError("Could not load YesVertexStruct_NoIndex_YesInstanceType_InstanceBuffer.bin");
        }

        // Create the buffer
        module.buffer_Instance_Buffer = device.createBuffer({
            label: "YesVertexStruct_NoIndex_YesInstanceType.buffer_Vertex_Buffer",
            size: instanceBufferData.byteLength,
            usage: module.buffer_Instance_Buffer_usageFlags,
        });

        // Write the data to the buffer
        device.queue.writeBuffer(module.buffer_Instance_Buffer, 0, instanceBufferData);

        // Set the instance buffer attributes
        module.buffer_Instance_Buffer_stride = 16
        module.buffer_Instance_Buffer_count = 3; //Math.floor(instanceBufferData.byteLength / module.buffer_Instance_Buffer_stride);
        module.buffer_Instance_Buffer_vertexBufferAttributes = 
        [
            // offsetScale
            {
                format: "float32x4",
                offset: 0,
                shaderLocation: 0,
            },
        ];

        // Set the camera to be the exact binary values used by the pother platforms
        let viewProjMtxSrc = [
            0x401A8279, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x401A8279, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0xB8D1BC76, 0x3DCAC5B4,
            0x00000000, 0x00000000, 0x3F800000, 0x41200000
        ];
        const viewProjMtx = new ArrayBuffer(64);
        const viewProjMtxView = new DataView(viewProjMtx);
        for (let i = 0; i < 16; ++i)
        {
            viewProjMtxView.setUint32(i*4, viewProjMtxSrc[i], true);
            module.variable_ViewProjMtx[i] = viewProjMtxView.getFloat32(i*4, true);
        }
        module.variable_ViewProjMtx = Shared.MatTranspose(module.variable_ViewProjMtx);

        // manually fix up the @location of the vertex and index buffers to match what the vertex shader says they should be
        module.buffer_Vertex_Buffer_vertexBufferAttributes[0].shaderLocation = 0;  // position
        module.buffer_Vertex_Buffer_vertexBufferAttributes[1].shaderLocation = 2;  // normal
        module.buffer_Vertex_Buffer_vertexBufferAttributes[2].shaderLocation = 3;  // uv
        module.buffer_Instance_Buffer_vertexBufferAttributes[0].shaderLocation = 1; // offsetScale_0

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Depth_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Depth_Buffer);
        this.texture_Color_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Color_Buffer);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Depth_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsBinaryFile(view, "Depth_Buffer", "../../../../../../Techniques/UnitTests/_GoldImages/Raster/YesVertexStruct_NoIndex_YesInstanceType/0.bin");
            }
        );

        await Shared.ReadbackBuffer(this.texture_Color_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Color_Buffer, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Raster/YesVertexStruct_NoIndex_YesInstanceType/1.png");
            }
        );
    }
};
export var test_Raster_YesVertexStruct_NoIndex_YesInstanceType = new Test_Raster_YesVertexStruct_NoIndex_YesInstanceType();

class Test_Raster_YesVertexStruct_YesIndex_NoInstance
{
    async Init(device, module)
    {
        // make sure we can read back the things we want to read back
        module.texture_Color_Buffer_usageFlags = module.texture_Color_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;
        module.texture_Depth_Buffer_usageFlags = module.texture_Depth_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;

        // Load the vertex buffer
        const VBCPU = new Float32Array((await Shared.LoadCSV('YesVertexStruct_YesIndex_NoInstance_VertexBuffer.csv', item => [ item["PosX"], item["PosY"], item["PosZ"], item["UVX"], item["UVY"] ] )).flat());

        // Create the buffer
        module.buffer_Vertex_Buffer = device.createBuffer({
            label: "YesVertexStruct_YesIndex_NoInstance.buffer_Vertex_Buffer",
            size: VBCPU.length * 4 * 5,
            usage: module.buffer_Vertex_Buffer_usageFlags,
        });

        // Write the data to the buffer
        device.queue.writeBuffer(module.buffer_Vertex_Buffer, 0, VBCPU);

        // Set the vertex buffer attributes
        module.buffer_Vertex_Buffer_stride = 20;
        module.buffer_Vertex_Buffer_count = VBCPU.length / 5;
        module.buffer_Vertex_Buffer_vertexBufferAttributes =
        [
            // Position
            {
                format: "float32x3",
                offset: 0,
                shaderLocation: 0,
            },
            // UV
            {
                format: "float32x2",
                offset: 12,
                shaderLocation: 1,
            },
        ];

        // Load the index buffer
        const IndexBCPU = new Uint32Array((await Shared.LoadCSV('YesVertexStruct_YesIndex_NoInstance_IndexBuffer.csv', item => [ item["index1"], item["index2"], item["index3"] ] )).flat());

        // Create the buffer
        module.buffer_Index_Buffer = device.createBuffer({
            label: "YesVertexStruct_YesIndex_NoInstance.buffer_Index_Buffer",
            size: IndexBCPU.length * 4 * 3,
            usage: module.buffer_Index_Buffer_usageFlags,
        });

        // Write the data to the buffer
        device.queue.writeBuffer(module.buffer_Index_Buffer, 0, IndexBCPU);

        // Set the index buffer attributes
        module.buffer_Index_Buffer_stride = 4;
        module.buffer_Index_Buffer_count = IndexBCPU.length;
        module.buffer_Index_Buffer_indexFormat = "uint32";

        // Set the camera to be the exact binary values used by the pother platforms
        let viewProjMtxSrc = [
            0x401A8279, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x401A8279, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0xB8D1BC76, 0x3DCAC5B4,
            0x00000000, 0x00000000, 0x3F800000, 0x41200000
        ];
        const viewProjMtx = new ArrayBuffer(64);
        const viewProjMtxView = new DataView(viewProjMtx);
        for (let i = 0; i < 16; ++i)
        {
            viewProjMtxView.setUint32(i*4, viewProjMtxSrc[i], true);
            module.variable_ViewProjMtx[i] = viewProjMtxView.getFloat32(i*4, true);
        }
        module.variable_ViewProjMtx = Shared.MatTranspose(module.variable_ViewProjMtx);

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Depth_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Depth_Buffer);
        this.texture_Color_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Color_Buffer);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Depth_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsBinaryFile(view, "Depth_Buffer", "../../../../../../Techniques/UnitTests/_GoldImages/Raster/YesVertexStruct_YesIndex_NoInstance/0.bin");
            }
        );

        await Shared.ReadbackBuffer(this.texture_Color_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Color_Buffer, "../../../../../../Techniques/UnitTests/_GoldImages/Raster/YesVertexStruct_YesIndex_NoInstance/1.png");
            }
        );
    }
};
export var test_Raster_YesVertexStruct_YesIndex_NoInstance = new Test_Raster_YesVertexStruct_YesIndex_NoInstance();

class Test_Raster_YesVertexType_NoIndex_NoInstance
{
    async Init(device, module)
    {
        // make sure we can read back the things we want to read back
        module.texture_Color_Buffer_usageFlags = module.texture_Color_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;
        module.texture_Depth_Buffer_usageFlags = module.texture_Depth_Buffer_usageFlags | GPUTextureUsage.COPY_SRC;

        // Load the vertex buffer
        const VBCPU = new Float32Array((await Shared.LoadCSV('YesVertexType_NoIndex_NoInstance_VertexBuffer.csv', item => [ item["value0"], item["value1"], item["value2"] ] )).flat());

        // Create the buffer
        module.buffer_Vertex_Buffer = device.createBuffer({
            label: "YesVertexType_NoIndex_NoInstance.buffer_Vertex_Buffer",
            size: VBCPU.length * 4 * 3,
            usage: module.buffer_Vertex_Buffer_usageFlags,
        });

        // Write the data to the buffer
        device.queue.writeBuffer(module.buffer_Vertex_Buffer, 0, VBCPU);

        // Set the vertex buffer attributes
        module.buffer_Vertex_Buffer_stride = 12;
        module.buffer_Vertex_Buffer_count = VBCPU.length / 3;
        module.buffer_Vertex_Buffer_vertexBufferAttributes =
        [
            // Position
            {
                format: "float32x3",
                offset: 0,
                shaderLocation: 0,
            },
        ];

        // Set the camera to be the exact binary values used by the pother platforms
        let viewProjMtxSrc = [
            0x401A8279, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x401A8279, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0xB8D1BC76, 0x3DCAC5B4,
            0x00000000, 0x00000000, 0x3F800000, 0x41200000
        ];
        const viewProjMtx = new ArrayBuffer(64);
        const viewProjMtxView = new DataView(viewProjMtx);
        for (let i = 0; i < 16; ++i)
        {
            viewProjMtxView.setUint32(i*4, viewProjMtxSrc[i], true);
            module.variable_ViewProjMtx[i] = viewProjMtxView.getFloat32(i*4, true);
        }
        module.variable_ViewProjMtx = Shared.MatTranspose(module.variable_ViewProjMtx);

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Depth_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Depth_Buffer);
        this.texture_Color_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Color_Buffer);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Depth_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsBinaryFile(view, "Depth_Buffer", "../../../../../../Techniques/UnitTests/_GoldImages/Raster/YesVertexType_NoIndex_NoInstance/0.bin");
            }
        );

        await Shared.ReadbackBuffer(this.texture_Color_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Color_Buffer, "../../../../../../Techniques/UnitTests/_GoldImages/Raster/YesVertexType_NoIndex_NoInstance/1.png");
            }
        );
    }
};
export var test_Raster_YesVertexType_NoIndex_NoInstance = new Test_Raster_YesVertexType_NoIndex_NoInstance();

class Test_SubGraph_SubGraphTest
{
    async Init(device, module)
    {
        // make sure we can read back the things we want to read back
        module.texture_Inner_Exported_Tex_usageFlags = module.texture_Inner_Exported_Tex_usageFlags | GPUTextureUsage.COPY_SRC;

        this.cabinTex = await Shared.CreateTextureWithPNG(device, "cabinsmall.png", module.texture_Test_usageFlags);

        module.texture_Test_size = this.cabinTex.size;
        module.texture_Test_format = this.cabinTex.format;
        module.texture_Test = device.createTexture({
            label: "texture Test",
            size: module.texture_Test_size,
            format: Shared.GetNonSRGBFormat(module.texture_Test_format),
            usage: module.texture_Test_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
        encoder.copyTextureToTexture(
            { texture: this.cabinTex.texture },
            { texture: module.texture_Test },
            {
                width: Math.min(module.texture_Test.width, this.cabinTex.texture.width),
                height: Math.min(module.texture_Test.height, this.cabinTex.texture.height),
                depthOrArrayLayers: Math.min(module.texture_Test.depthOrArrayLayers, this.cabinTex.texture.depthOrArrayLayers)
            }
        );
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Inner_Exported_Tex_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Inner_Exported_Tex);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Inner_Exported_Tex_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Inner_Exported_Tex, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/SubGraph/SubGraphTest/0.png");
            }
        );
    }
};
export var test_SubGraph_SubGraphTest = new Test_SubGraph_SubGraphTest();

class Test_RayTrace_AnyHit
{
    async Init(device, module)
    {
        // Load the BVH
        let bufferSceneBVH;
        if (!await Shared.LoadBin("AnyHit.combined.bvh", (data) => bufferSceneBVH = data ))
        {
            Shared.LogError("Could not load AnyHit.combined.bvh");
        }

        // Load the vertex buffer
        let VBUnpadded;
        if (!await Shared.LoadBin("AnyHitVB.bin", (data) => VBUnpadded = data ))
        {
            Shared.LogError("Could not load AnyHitVB.bin");
        }
        const VB = Shared.CopyBufferByFieldNames(new DataView(VBUnpadded), module.constructor.StructOffsets_VertexBuffer_Unpadded, module.constructor.StructOffsets_VertexBuffer);

        // BVH
        {
            // Create the Scene buffer
            this.buffer_Scene_BVH = device.createBuffer({
                label: "AnyHit.buffer_Scene",
                size: bufferSceneBVH.byteLength,
                usage: module.buffer_Scene_usageFlags,
            });

            // Write the data to the buffer
            device.queue.writeBuffer(this.buffer_Scene_BVH, 0, bufferSceneBVH);
        }

        // NonBVH
        {
            // Create the Scene buffer
            this.buffer_Scene_NonBVH = device.createBuffer({
                label: "AnyHit.buffer_Scene",
                size: VB.byteLength,
                usage: module.buffer_Scene_usageFlags,
            });

            // Write the data to the buffer
            device.queue.writeBuffer(this.buffer_Scene_NonBVH, 0, VB);
        }

        // Set the vertex buffer attributes
        module.buffer_Scene_stride = module.constructor.StructOffsets_VertexBuffer._size;
        module.buffer_Scene_count = Math.floor(VB.byteLength / module.buffer_Scene_stride);
        module.buffer_Scene_vertexBufferAttributes = module.constructor.StructVertexBufferAttributes_VertexBuffer;

        // Create the SceneVB buffer
        module.buffer_SceneVB = device.createBuffer({
            label: "AnyHit.buffer_SceneVB",
            size: VB.byteLength,
            usage: module.buffer_SceneVB_usageFlags,
        });

        // Write the data to the buffer
        device.queue.writeBuffer(module.buffer_SceneVB, 0, VB);

        // Set the vertex buffer attributes
        module.buffer_SceneVB_stride = module.constructor.StructOffsets_VertexBuffer._size;
        module.buffer_SceneVB_count = Math.floor(VB.byteLength / module.buffer_SceneVB_stride);
        module.buffer_SceneVB_vertexBufferAttributes = module.constructor.StructVertexBufferAttributes_VertexBuffer;

        // Set the camera to be the exact binary values used by the other platforms
        {
            let invViewProjMtxSrc = [
                0x3ED2C305, 0x3BF14D57, 0xBEF2279C, 0xBDE0EDC9,
                0x80000000, 0x3ED15999, 0x404BAC28, 0x3E24058C,
                0x3D3CAEF3, 0xBD86C4CE, 0xC2148A7A, 0x3F7A307E,
                0x80000000, 0x80000000, 0x411FFBE7, 0x3A83126E
            ];
            const rawBytes = new ArrayBuffer(4);
            const rawBytesView = new DataView(rawBytes);
            for (let i = 0; i < 16; ++i)
            {
                rawBytesView.setUint32(0, invViewProjMtxSrc[i], true);
                module.variable_InvViewProjMtx[i] = rawBytesView.getFloat32(0, true);
            }
            module.variable_InvViewProjMtx = Shared.MatTranspose(module.variable_InvViewProjMtx);

            let cameraPosSrc = [ 0xBD41BE50, 0x3EA2F44F, 0xC06DB074 ];
            for (let i = 0; i < 3; ++i)
            {
                rawBytesView.setUint32(0, cameraPosSrc[i], true);
                module.variable_CameraPos[i] = rawBytesView.getFloat32(0, true);
            }
        }

        module.variable_depthNearPlane = 0.0;

        // Use BVH to start out
        module.buffer_Scene = this.buffer_Scene_BVH;
        module.buffer_Scene_isBVH = true;

        return 2; // Execute once for BVH, once for non BVH
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Output_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Output);

        // Switch to Non BVH
        module.buffer_Scene = this.buffer_Scene_NonBVH;
        module.buffer_Scene_isBVH = false;
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Output_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Output, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/RayTrace/AnyHit/" + frameIndex + ".png");
            }
        );
    }
};
export var test_RayTrace_AnyHit = new Test_RayTrace_AnyHit();

class Test_RayTrace_AnyHitSimple
{
    async Init(device, module)
    {
        // This is AnyHit.StructOffsets_VertexBuffer_Unpadded, which is the format of the data in AnyHitVB.bin
        const StructOffsets_VertexBuffer_Unpadded =
        {
            position_0: 0,
            position_1: 4,
            position_2: 8,
            normal_0: 12,
            normal_1: 16,
            normal_2: 20,
            uv_0: 24,
            uv_1: 28,
            materialId: 32,
            _size: 36,
        }

        const StructOffsets_VertexBuffer_Float3 =
        {
            position_0: 0,
            position_1: 4,
            position_2: 8,
            _padding: 12,
            _size: 16,
        }

        const StructVertexBufferAttributes_VertexBuffer_Float3 =
        [
            // position
            {
                format: "float32x3",
                offset: 0,
                shaderLocation: 0,
            },
        ];

        // Load the vertex buffer
        let VBUnpadded;
        if (!await Shared.LoadBin("AnyHitVB.bin", (data) => VBUnpadded = data ))
        {
            Shared.LogError("Could not load AnyHitVB.bin");
        }

        // Convert from AnyHit.StructOffsets_VertexBuffer_Unpadded, to a padded float3 position buffer
        const VB = Shared.CopyBufferByFieldNames(new DataView(VBUnpadded), StructOffsets_VertexBuffer_Unpadded, StructOffsets_VertexBuffer_Float3);

        // Create the Scene buffer
        module.buffer_Scene = device.createBuffer({
            label: "AnyHitSimple.buffer_Scene",
            size: VB.byteLength,
            usage: module.buffer_Scene_usageFlags,
        });

        // Write the data to the buffer
        device.queue.writeBuffer(module.buffer_Scene, 0, VB);

        // Set the vertex buffer attributes
        module.buffer_Scene_stride = StructOffsets_VertexBuffer_Float3._size;
        module.buffer_Scene_count = Math.floor(VB.byteLength / module.buffer_Scene_stride);
        module.buffer_Scene_vertexBufferAttributes = StructVertexBufferAttributes_VertexBuffer_Float3;

        // Set the camera to be the exact binary values used by the other platforms
        {
            let invViewProjMtxSrc = [
                0x3ED2C305, 0x3BF14D57, 0xBEF2279C, 0xBDE0EDC9,
                0x80000000, 0x3ED15999, 0x404BAC28, 0x3E24058C,
                0x3D3CAEF3, 0xBD86C4CE, 0xC2148A7A, 0x3F7A307E,
                0x80000000, 0x80000000, 0x411FFBE7, 0x3A83126E
            ];
            const rawBytes = new ArrayBuffer(4);
            const rawBytesView = new DataView(rawBytes);
            for (let i = 0; i < 16; ++i)
            {
                rawBytesView.setUint32(0, invViewProjMtxSrc[i], true);
                module.variable_InvViewProjMtx[i] = rawBytesView.getFloat32(0, true);
            }
            module.variable_InvViewProjMtx = Shared.MatTranspose(module.variable_InvViewProjMtx);

            let cameraPosSrc = [ 0xBD41BE50, 0x3EA2F44F, 0xC06DB074 ];
            for (let i = 0; i < 3; ++i)
            {
                rawBytesView.setUint32(0, cameraPosSrc[i], true);
                module.variable_CameraPos[i] = rawBytesView.getFloat32(0, true);
            }
        }

        module.variable_depthNearPlane = 0.0;

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Output_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Output);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Output_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Output, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/RayTrace/AnyHitSimple/0.png");
            }
        );
    }
};
export var test_RayTrace_AnyHitSimple = new Test_RayTrace_AnyHitSimple();

class Test_RayTrace_IntersectionShader
{
    async Init(device, module)
    {
        // Create buffer AABBs
        const csvData = await Shared.LoadCSV('IntersectionShader.csv',
            item => [ parseFloat(item["MinX"]), parseFloat(item["MinY"]), parseFloat(item["MinZ"]), parseFloat(item["MaxX"]), parseFloat(item["MaxY"]), parseFloat(item["MaxZ"]) ]
        );

        module.buffer_AABBs_count = csvData.length*6;
        module.buffer_AABBs_stride = 4;
        module.buffer_AABBs_isAABBs = true;

        const alignedBufferSize = Shared.Align(16, module.buffer_AABBs_count * module.buffer_AABBs_stride);

        module.buffer_AABBs = device.createBuffer({
            label: "IntersectionShader.AABBs",
            size: alignedBufferSize,
            usage: module.buffer_AABBs_usageFlags,
        });

        const bufferCPU = new ArrayBuffer(alignedBufferSize);
        const view = new DataView(bufferCPU);

        csvData.forEach((value, index) =>
            {
                const baseOffset = index * 4 * 6;
                view.setFloat32(baseOffset + 0, value[0], true);
                view.setFloat32(baseOffset + 4, value[1], true);
                view.setFloat32(baseOffset + 8, value[2], true);
                view.setFloat32(baseOffset + 12, value[3], true);
                view.setFloat32(baseOffset + 16, value[4], true);
                view.setFloat32(baseOffset + 20, value[5], true);
            }
        );
        device.queue.writeBuffer(module.buffer_AABBs, 0, bufferCPU);

        // Create buffer AABBsSRV
        module.buffer_AABBsSRV_count = module.buffer_AABBs_count;
        module.buffer_AABBsSRV_stride = module.buffer_AABBs_stride;

        module.buffer_AABBsSRV = device.createBuffer({
            label: "IntersectionShader.AABBsSRV",
            size: alignedBufferSize,
            usage: module.buffer_AABBsSRV_usageFlags,
        });

        device.queue.writeBuffer(module.buffer_AABBsSRV, 0, bufferCPU);

        // Set the camera to be the exact binary values used by the other platforms
        {
            let invViewProjMtxSrc = [
                0x3ED413CD, 0x80000000, 0x409FFBE7, 0x3A03126F,
                0x80000000, 0x3ED413CD, 0x409FFBE7, 0x3A03126F,
                0x80000000, 0x80000000, 0xC1EFF9DB, 0x3F7F3B65,
                0x80000000, 0x80000000, 0x411FFBE7, 0x3A83126F
            ];
            const rawBytes = new ArrayBuffer(4);
            const rawBytesView = new DataView(rawBytes);
            for (let i = 0; i < 16; ++i)
            {
                rawBytesView.setUint32(0, invViewProjMtxSrc[i], true);
                module.variable_InvViewProjMtx[i] = rawBytesView.getFloat32(0, true);
            }
            module.variable_InvViewProjMtx = Shared.MatTranspose(module.variable_InvViewProjMtx);

            let cameraPosSrc = [ 0x3F000000, 0x3F000000, 0xC0400000 ];
            for (let i = 0; i < 3; ++i)
            {
                rawBytesView.setUint32(0, cameraPosSrc[i], true);
                module.variable_CameraPos[i] = rawBytesView.getFloat32(0, true);
            }
        }

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Output_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Output);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Output_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Output, "../../../../../../Techniques/UnitTests/_GoldImages/RayTrace/IntersectionShader/0.png");
            }
        );
    }
};
export var test_RayTrace_IntersectionShader = new Test_RayTrace_IntersectionShader();

class Test_RayTrace_simpleRT
{
    async Init(device, module)
    {
        // Create the scene buffer
        {
            const csvData = await Shared.LoadCSV('SimpleRT_VertexBuffer.csv',
                item => [ parseFloat(item["ColorR"]), parseFloat(item["colorG"]), parseFloat(item["colorB"]), parseFloat(item["PositionX"]), parseFloat(item["PositionY"]), parseFloat(item["PositionZ"]) ]
            );

            module.buffer_Scene_count = csvData.length;
            module.buffer_Scene_stride = module.constructor.StructOffsets_VertexBuffer._size;

            const alignedBufferSize = Shared.Align(16, module.buffer_Scene_count * module.buffer_Scene_stride);

            module.buffer_Scene = device.createBuffer({
                label: "simpleRT.Scene",
                size: alignedBufferSize,
                usage: module.buffer_Scene_usageFlags,
            });

            const bufferCPU = new ArrayBuffer(alignedBufferSize);
            const view = new DataView(bufferCPU);

            csvData.forEach((value, index) =>
                {
                    const baseOffset = index * module.buffer_Scene_stride;
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBuffer.Color_0, value[0], true);
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBuffer.Color_1, value[1], true);
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBuffer.Color_2, value[2], true);
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBuffer.Position_0, value[3], true);
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBuffer.Position_1, value[4], true);
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBuffer.Position_2, value[5], true);
                }
            );
            device.queue.writeBuffer(module.buffer_Scene, 0, bufferCPU);
        }

        // Set the camera to be the exact binary values used by the pother platforms
        {
            let invViewProjMtxSrc = [
                0x3F3C8362, 0x80000000, 0x80000000, 0x00000000,
                0x80000000, 0x3ED413CF, 0x80000000, 0x80000000,
                0x80000000, 0x80000000, 0xC2C7FAE2, 0x3F7D70A5,
                0x80000000, 0x80000000, 0x411FFBE8, 0x3A83126F
            ];
            const invViewProjMtx = new ArrayBuffer(64);
            const invViewProjMtxView = new DataView(invViewProjMtx);
            for (let i = 0; i < 16; ++i)
            {
                invViewProjMtxView.setUint32(i*4, invViewProjMtxSrc[i], true);
                module.variable_clipToWorld[i] = invViewProjMtxView.getFloat32(i*4, true);
            }
            module.variable_clipToWorld = Shared.MatTranspose(module.variable_clipToWorld);
        }

        module.variable_cameraPos[0] = 0.0;
        module.variable_cameraPos[1] = 0.0;
        module.variable_cameraPos[2] = -10.0;

        module.variable_depthNearPlane = 0.1;

        module.variable_hitColor[0] = 0.0;
        module.variable_hitColor[1] = 1.0;
        module.variable_hitColor[2] = 0.0;

        module.variable_missColor[0] = 1.0;
        module.variable_missColor[1] = 0.0;
        module.variable_missColor[2] = 0.0;

        module.variable_enabled = true;

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Texture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Texture);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Texture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Texture, "../../../../../../Techniques/UnitTests/_GoldImages/RayTrace/simpleRT/1.png");
            }
        );
    }
};
export var test_RayTrace_simpleRT = new Test_RayTrace_simpleRT();

class Test_RayTrace_simpleRTDynamic
{
    async Init(device, module)
    {
        // Create the scene buffer
        {
            const csvData = await Shared.LoadCSV('SimpleRT_VertexBuffer.csv',
                item => [ parseFloat(item["ColorR"]), parseFloat(item["colorG"]), parseFloat(item["colorB"]), parseFloat(item["PositionX"]), parseFloat(item["PositionY"]), parseFloat(item["PositionZ"]) ]
            );

            module.buffer_Scene_count = csvData.length;
            module.buffer_Scene_stride = module.constructor.StructOffsets_VertexBuffer._size;

            const alignedBufferSize = Shared.Align(16, module.buffer_Scene_count * module.buffer_Scene_stride);

            module.buffer_Scene = device.createBuffer({
                label: "simpleRT.Scene",
                size: alignedBufferSize,
                usage: module.buffer_Scene_usageFlags,
            });

            const bufferCPU = new ArrayBuffer(alignedBufferSize);
            const view = new DataView(bufferCPU);

            csvData.forEach((value, index) =>
                {
                    const baseOffset = index * module.buffer_Scene_stride;
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBuffer.Color_0, value[0], true);
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBuffer.Color_1, value[1], true);
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBuffer.Color_2, value[2], true);
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBuffer.Position_0, value[3], true);
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBuffer.Position_1, value[4], true);
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBuffer.Position_2, value[5], true);
                }
            );

            this.buffer_scene_initialState = device.createBuffer({
                label: "simpleRT.Scene Initial State",
                size: alignedBufferSize,
                usage: GPUBufferUsage.COPY_DST | GPUBufferUsage.COPY_SRC,
            });

            device.queue.writeBuffer(this.buffer_scene_initialState, 0, bufferCPU);
        }

        // Set the camera to be the exact binary values used by the pother platforms
        {
            let invViewProjMtxSrc = [
                0x3F3C8362, 0x80000000, 0x80000000, 0x00000000,
                0x80000000, 0x3ED413CF, 0x80000000, 0x80000000,
                0x80000000, 0x80000000, 0xC2C7FAE2, 0x3F7D70A5,
                0x80000000, 0x80000000, 0x411FFBE8, 0x3A83126F
            ];
            const invViewProjMtx = new ArrayBuffer(64);
            const invViewProjMtxView = new DataView(invViewProjMtx);
            for (let i = 0; i < 16; ++i)
            {
                invViewProjMtxView.setUint32(i*4, invViewProjMtxSrc[i], true);
                module.variable_clipToWorld[i] = invViewProjMtxView.getFloat32(i*4, true);
            }
            module.variable_clipToWorld = Shared.MatTranspose(module.variable_clipToWorld);
        }

        module.variable_cameraPos[0] = 0.0;
        module.variable_cameraPos[1] = 0.0;
        module.variable_cameraPos[2] = -10.0;

        module.variable_depthNearPlane = 0.1;

        module.variable_hitColor[0] = 0.0;
        module.variable_hitColor[1] = 1.0;
        module.variable_hitColor[2] = 0.0;

        module.variable_missColor[0] = 1.0;
        module.variable_missColor[1] = 0.0;
        module.variable_missColor[2] = 0.0;

        module.variable_enabled = true;

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
        encoder.copyBufferToBuffer(this.buffer_scene_initialState, 0, module.buffer_Scene, 0, this.buffer_scene_initialState.size);
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Texture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Texture);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Texture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Texture, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/RayTrace/simpleRTDynamic/1.png");
            }
        );
    }
};
export var test_RayTrace_simpleRTDynamic = new Test_RayTrace_simpleRTDynamic();

class Test_RayTrace_simpleRT_inline
{
    async Init(device, module)
    {
        // Load the BVH
        let bufferSceneBVH;
        if (!await Shared.LoadBin("simpleRT_inlineVB.combined.bvh", (data) => bufferSceneBVH = data ))
        {
            Shared.LogError("Could not load simpleRT_inlineVB.combined.bvh");
        }

        // Load the vertex buffer
        let bufferSceneVBUnpadded;
        if (!await Shared.LoadBin("simpleRT_inlineVB.bin", (data) => bufferSceneVBUnpadded = data ))
        {
            Shared.LogError("Could not load simpleRT_inlineVB.bin");
        }
        const bufferSceneVB = Shared.CopyBufferByFieldNames(new DataView(bufferSceneVBUnpadded), module.constructor.StructOffsets_VertexBuffer_Unpadded, module.constructor.StructOffsets_VertexBuffer);

        // BVH
        {
            // Create the buffer Scene
            this.buffer_Scene_BVH = device.createBuffer({
                label: "simpleRT_inline.buffer_Scene",
                size: bufferSceneBVH.byteLength,
                usage: module.buffer_Scene_usageFlags,
            });

            // Write the data to the buffer
            device.queue.writeBuffer(this.buffer_Scene_BVH, 0, bufferSceneBVH);
        }

        // Non BVH
        {
            // Create the buffer Scene
            this.buffer_Scene_NonBVH = device.createBuffer({
                label: "simpleRT_inline.buffer_Scene",
                size: bufferSceneVB.byteLength,
                usage: module.buffer_Scene_usageFlags,
            });

            // Write the data to the buffer
            device.queue.writeBuffer(this.buffer_Scene_NonBVH, 0, bufferSceneVB);
        }

        // Create the buffer Scene_VB
        module.buffer_Scene_VB = device.createBuffer({
            label: "simpleRT_inline.buffer_Scene_VB",
            size: bufferSceneVB.byteLength,
            usage: module.buffer_Scene_usageFlags,
        });

        // Write the data to the buffer
        device.queue.writeBuffer(module.buffer_Scene_VB, 0, bufferSceneVB);

        // Set the camera to be the exact binary values used by the other platforms
        {
            let invViewProjMtxSrc = [
                0xBEADC77F, 0xBC6560B9, 0xC1AFFB7F, 0x3F11EBC7,
                0xB124B189, 0x3ED3B556, 0x40AC7676, 0x3D73BB46,
                0xBE732041, 0x3CA3F3D5, 0x41CBE64D, 0xBF50C0AA,
                0x2D8476D4, 0xAC1295F9, 0x411FFBE8, 0x3A83126F
            ];
            const rawBytes = new ArrayBuffer(4);
            const rawBytesView = new DataView(rawBytes);
            for (let i = 0; i < 16; ++i)
            {
                rawBytesView.setUint32(0, invViewProjMtxSrc[i], true);
                module.variable_clipToWorld[i] = rawBytesView.getFloat32(0, true);
            }
            module.variable_clipToWorld = Shared.MatTranspose(module.variable_clipToWorld);

            let cameraPosSrc = [ 0xC00CCCCD, 0x3F09FBE7, 0x402322D1 ];
            for (let i = 0; i < 3; ++i)
            {
                rawBytesView.setUint32(0, cameraPosSrc[i], true);
                module.variable_cameraPos[i] = rawBytesView.getFloat32(0, true);
            }
        }

        module.variable_depthNearPlane = 0.0;
        module.variable_hitColor = [0, 1, 0];
        module.variable_missColor = [0.2, 0.2, 0.2];
        module.variable_enabled = true;

        // Use BVH to start out
        module.buffer_Scene = this.buffer_Scene_BVH;
        module.buffer_Scene_isBVH = true;

        return 2; // Execute once for BVH, once for non BVH
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Texture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Texture);

        // Switch to Non BVH
        module.buffer_Scene = this.buffer_Scene_NonBVH;
        module.buffer_Scene_isBVH = false;
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Texture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Texture, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/RayTrace/simpleRT_inline/" + frameIndex.toString() + ".png");
            }
        );
    }
};
export var test_RayTrace_simpleRT_inline = new Test_RayTrace_simpleRT_inline();

class Test_RayTrace_TwoRayGens
{
    async Init(device, module)
    {
        // Create the scene buffer
        {
            const csvData = await Shared.LoadCSV('SimpleRT_VertexBufferSimple.csv',
                item => [ parseFloat(item["PositionX"]), parseFloat(item["PositionY"]), parseFloat(item["PositionZ"]) ]
            );

            module.buffer_Scene_count = csvData.length;
            module.buffer_Scene_stride = module.constructor.StructOffsets_VertexBufferSimple._size;

            const alignedBufferSize = Shared.Align(16, module.buffer_Scene_count * module.buffer_Scene_stride);

            module.buffer_Scene = device.createBuffer({
                label: "simpleRT.Scene",
                size: alignedBufferSize,
                usage: module.buffer_Scene_usageFlags,
            });

            const bufferCPU = new ArrayBuffer(alignedBufferSize);
            const view = new DataView(bufferCPU);

            csvData.forEach((value, index) =>
                {
                    const baseOffset = index * module.buffer_Scene_stride;
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBufferSimple.Position_0, value[0], true);
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBufferSimple.Position_1, value[1], true);
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBufferSimple.Position_2, value[2], true);
                }
            );

            this.buffer_scene_initialState = device.createBuffer({
                label: "simpleRT.Scene Initial State",
                size: alignedBufferSize,
                usage: GPUBufferUsage.COPY_DST | GPUBufferUsage.COPY_SRC,
            });

            device.queue.writeBuffer(this.buffer_scene_initialState, 0, bufferCPU);
        }

        this.cabinTex = await Shared.CreateTextureWithPNG(device, "cabinsmall.png", module.texture_Input_usageFlags | GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC);

        module.texture_BlueChannel_size = this.cabinTex.size;
        module.texture_BlueChannel_format = this.cabinTex.format;
        module.texture_BlueChannel = device.createTexture({
            label: "texture BlueChannel",
            size: module.texture_BlueChannel_size,
            format: Shared.GetNonSRGBFormat(module.texture_BlueChannel_format),
            usage: module.texture_BlueChannel_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        // Set the camera to be the exact binary values used by the pother platforms
        {
            let invViewProjMtxSrc = [
                0x3ED413CD, 0x80000000, 0x80000000, 0x00000000,
                0x80000000, 0x3ED413CD, 0x80000000, 0x80000000,
                0x80000000, 0x80000000, 0xC2C7FAE1, 0x3F7D70A3,
                0x80000000, 0x80000000, 0x411FFBE7, 0x3A83126F
            ];
            const invViewProjMtx = new ArrayBuffer(64);
            const invViewProjMtxView = new DataView(invViewProjMtx);
            for (let i = 0; i < 16; ++i)
            {
                invViewProjMtxView.setUint32(i*4, invViewProjMtxSrc[i], true);
                module.variable_clipToWorld[i] = invViewProjMtxView.getFloat32(i*4, true);
            }
            module.variable_clipToWorld = Shared.MatTranspose(module.variable_clipToWorld);
        }

        module.variable_cameraPos[0] = 0.0;
        module.variable_cameraPos[1] = 0.0;
        module.variable_cameraPos[2] = -10.0;

        module.variable_depthNearPlane = 0.0;


        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
        encoder.copyBufferToBuffer(this.buffer_scene_initialState, 0, module.buffer_Scene, 0, this.buffer_scene_initialState.size);

        encoder.copyTextureToTexture(
            { texture: this.cabinTex.texture },
            { texture: module.texture_BlueChannel },
            {
                width: Math.min(module.texture_BlueChannel.width, this.cabinTex.texture.width),
                height: Math.min(module.texture_BlueChannel.height, this.cabinTex.texture.height),
                depthOrArrayLayers: Math.min(module.texture_BlueChannel.depthOrArrayLayers, this.cabinTex.texture.depthOrArrayLayers)
            }
        );
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Texture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Texture);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Texture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Texture, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/RayTrace/TwoRayGens/0.png");
            }
        );
    }
};
export var test_RayTrace_TwoRayGens = new Test_RayTrace_TwoRayGens();

class Test_RayTrace_TwoRayGensSubgraph
{
    async Init(device, module)
    {
        // Create the scene buffer
        {
            const csvData = await Shared.LoadCSV('SimpleRT_VertexBufferSimple.csv',
                item => [ parseFloat(item["PositionX"]), parseFloat(item["PositionY"]), parseFloat(item["PositionZ"]) ]
            );

            module.buffer_Scene_count = csvData.length;
            module.buffer_Scene_stride = module.constructor.StructOffsets_VertexBufferSimple._size;

            const alignedBufferSize = Shared.Align(16, module.buffer_Scene_count * module.buffer_Scene_stride);

            module.buffer_Scene = device.createBuffer({
                label: "simpleRT.Scene",
                size: alignedBufferSize,
                usage: module.buffer_Scene_usageFlags,
            });

            const bufferCPU = new ArrayBuffer(alignedBufferSize);
            const view = new DataView(bufferCPU);

            csvData.forEach((value, index) =>
                {
                    const baseOffset = index * module.buffer_Scene_stride;
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBufferSimple.Position_0, value[0], true);
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBufferSimple.Position_1, value[1], true);
                    view.setFloat32(baseOffset + module.constructor.StructOffsets_VertexBufferSimple.Position_2, value[2], true);
                }
            );

            this.buffer_scene_initialState = device.createBuffer({
                label: "simpleRT.Scene Initial State",
                size: alignedBufferSize,
                usage: GPUBufferUsage.COPY_DST | GPUBufferUsage.COPY_SRC,
            });

            device.queue.writeBuffer(this.buffer_scene_initialState, 0, bufferCPU);
        }

        this.cabinTex = await Shared.CreateTextureWithPNG(device, "cabinsmall.png", module.texture_Input_usageFlags | GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC);

        module.texture_BlueChannel_size = this.cabinTex.size;
        module.texture_BlueChannel_format = this.cabinTex.format;
        module.texture_BlueChannel = device.createTexture({
            label: "texture BlueChannel",
            size: module.texture_BlueChannel_size,
            format: Shared.GetNonSRGBFormat(module.texture_BlueChannel_format),
            usage: module.texture_BlueChannel_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        // Set the camera to be the exact binary values used by the pother platforms
        {
            let invViewProjMtxSrc = [
                0x3ED413CD, 0x80000000, 0x80000000, 0x00000000,
                0x80000000, 0x3ED413CD, 0x80000000, 0x80000000,
                0x80000000, 0x80000000, 0xC2C7FAE1, 0x3F7D70A3,
                0x80000000, 0x80000000, 0x411FFBE7, 0x3A83126F
            ];
            const invViewProjMtx = new ArrayBuffer(64);
            const invViewProjMtxView = new DataView(invViewProjMtx);
            for (let i = 0; i < 16; ++i)
            {
                invViewProjMtxView.setUint32(i*4, invViewProjMtxSrc[i], true);
                module.variable_clipToWorld[i] = invViewProjMtxView.getFloat32(i*4, true);
            }
            module.variable_clipToWorld = Shared.MatTranspose(module.variable_clipToWorld);
        }

        module.variable_cameraPos[0] = 0.0;
        module.variable_cameraPos[1] = 0.0;
        module.variable_cameraPos[2] = -10.0;

        module.variable_depthNearPlane = 0.0;


        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
        encoder.copyBufferToBuffer(this.buffer_scene_initialState, 0, module.buffer_Scene, 0, this.buffer_scene_initialState.size);

        encoder.copyTextureToTexture(
            { texture: this.cabinTex.texture },
            { texture: module.texture_BlueChannel },
            {
                width: Math.min(module.texture_BlueChannel.width, this.cabinTex.texture.width),
                height: Math.min(module.texture_BlueChannel.height, this.cabinTex.texture.height),
                depthOrArrayLayers: Math.min(module.texture_BlueChannel.depthOrArrayLayers, this.cabinTex.texture.depthOrArrayLayers)
            }
        );
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Texture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Texture);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Texture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Texture, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/RayTrace/TwoRayGensSubgraph/0.png");
            }
        );
    }
};
export var test_RayTrace_TwoRayGensSubgraph = new Test_RayTrace_TwoRayGensSubgraph();

class Test_SubGraph_ConstOverride
{
    async Init(device, module)
    {
        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Texture_Output = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Output);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Texture_Output,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Output, "../../../../../../Techniques/UnitTests/_GoldImages/SubGraph/ConstOverride/0.png");
            }
        );
    }
};
export var test_SubGraph_ConstOverride = new Test_SubGraph_ConstOverride();

class Test_SubGraph_SubGraphLoops
{
    async Init(device, module)
    {
        // make sure we can read back the things we want to read back1
        module.texture_Input_usageFlags = module.texture_Input_usageFlags | GPUTextureUsage.COPY_SRC;

        this.cabinTex = await Shared.CreateTextureWithPNG(device, "cabinsmall.png", module.texture_Input_usageFlags);

        module.texture_Input_size = this.cabinTex.size;
        module.texture_Input_format = this.cabinTex.format;
        module.texture_Input = device.createTexture({
            label: "texture Input",
            size: module.texture_Input_size,
            format: Shared.GetNonSRGBFormat(module.texture_Input_format),
            usage: module.texture_Input_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
        encoder.copyTextureToTexture(
            { texture: this.cabinTex.texture },
            { texture: module.texture_Input },
            {
                width: Math.min(module.texture_Input.width, this.cabinTex.texture.width),
                height: Math.min(module.texture_Input.height, this.cabinTex.texture.height),
                depthOrArrayLayers: Math.min(module.texture_Input.depthOrArrayLayers, this.cabinTex.texture.depthOrArrayLayers)
            }
        );
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_FilterSub_Iteration_4_Output_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_FilterSub_Iteration_4_Output);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_FilterSub_Iteration_4_Output_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_FilterSub_Iteration_4_Output, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/SubGraph/SubGraphLoops/0.png");
            }
        );
    }
};
export var test_SubGraph_SubGraphLoops = new Test_SubGraph_SubGraphLoops();

class Test_SubGraph_SubInSub
{
    async Init(device, module)
    {
        // make sure we can read back the things we want to read back1
        module.texture_Input_usageFlags = module.texture_Input_usageFlags | GPUTextureUsage.COPY_SRC;

        this.cabinTex = await Shared.CreateTextureWithPNG(device, "cabinsmall.png", module.texture_Input_usageFlags);

        module.texture_Input_size = this.cabinTex.size;
        module.texture_Input_format = this.cabinTex.format;
        module.texture_Input = device.createTexture({
            label: "texture Input",
            size: module.texture_Input_size,
            format: Shared.GetNonSRGBFormat(module.texture_Input_format),
            usage: module.texture_Input_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
        encoder.copyTextureToTexture(
            { texture: this.cabinTex.texture },
            { texture: module.texture_Input },
            {
                width: Math.min(module.texture_Input.width, this.cabinTex.texture.width),
                height: Math.min(module.texture_Input.height, this.cabinTex.texture.height),
                depthOrArrayLayers: Math.min(module.texture_Input.depthOrArrayLayers, this.cabinTex.texture.depthOrArrayLayers)
            }
        );
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Input_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Input);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Input_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Input, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/SubGraph/SubInSub/0.png");
            }
        );
    }
};
export var test_SubGraph_SubInSub = new Test_SubGraph_SubInSub();

class Test_Textures_Mips_CS_2D
{
    async Init(device, module)
    {
        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Output_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Output, 3);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Output_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Output, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Mips_CS_2D/0.png", 3);
            }
        );
    }
};
export var test_Textures_Mips_CS_2D = new Test_Textures_Mips_CS_2D();

class Test_Textures_Mips_CS_2DArray
{
    async Init(device, module)
    {
        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Output_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Output, 3);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Output_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Output, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Mips_CS_2DArray/0.png", 3, 0);
                VerifyDataViewEqualsPNGFile(view, module.texture_Output, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Mips_CS_2DArray/1.png", 3, 1);
            }
        );
    }
};
export var test_Textures_Mips_CS_2DArray = new Test_Textures_Mips_CS_2DArray();

class Test_Textures_Mips_CS_3D
{
    async Init(device, module)
    {
        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Output_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Output, 3);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Output_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Output, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Mips_CS_3D/0.0.png", 3);
            }
        );
    }
};
export var test_Textures_Mips_CS_3D = new Test_Textures_Mips_CS_3D();

class Test_Textures_Mips_DrawCall
{
    async Init(device, module)
    {
        module.texture_Final_Color_Buffer_usageFlags |= GPUTextureUsage.COPY_SRC;

        // Bunny
        {
            // Load the vertex buffer
            let VBUnpadded;
            if (!await Shared.LoadBin("Mips_DrawCall_Bunny.bin", (data) => VBUnpadded = data ))
            {
                Shared.LogError("Could not load Mips_DrawCall_Bunny.bin");
            }
            const VBPadded = Shared.CopyBufferByFieldNames(new DataView(VBUnpadded), module.constructor.StructOffsets_VertexFormat_Unpadded, module.constructor.StructOffsets_VertexFormat);

            // Create the buffer
            module.buffer_VertexBuffer = device.createBuffer({
                label: "Mips_CS_3D.buffer_VertexBuffer",
                size: VBPadded.byteLength,
                usage: module.buffer_VertexBuffer_usageFlags,
            });

            // Write the data to the buffer
            device.queue.writeBuffer(module.buffer_VertexBuffer, 0, VBPadded);

            // Set the vertex buffer attributes
            module.buffer_VertexBuffer_stride = module.constructor.StructOffsets_VertexFormat._size;
            module.buffer_VertexBuffer_count = Math.floor(VBPadded.byteLength / module.buffer_VertexBuffer_stride);
            module.buffer_VertexBuffer_vertexBufferAttributes = module.constructor.StructVertexBufferAttributes_VertexFormat;
        }

        // Sphere
        {
            // Load the vertex buffer
            let VBUnpadded;
            if (!await Shared.LoadBin("Mips_DrawCall_Sphere.bin", (data) => VBUnpadded = data ))
            {
                Shared.LogError("Could not load Mips_DrawCall_Sphere.bin");
            }
            const VBPadded = Shared.CopyBufferByFieldNames(new DataView(VBUnpadded), module.constructor.StructOffsets_VertexFormat_Unpadded, module.constructor.StructOffsets_VertexFormat);

            // Create the buffer
            module.buffer_SphereVB = device.createBuffer({
                label: "Mips_CS_3D.buffer_SphereVB",
                size: VBPadded.byteLength,
                usage: module.buffer_SphereVB_usageFlags,
            });

            // Write the data to the buffer
            device.queue.writeBuffer(module.buffer_SphereVB, 0, VBPadded);

            // Set the vertex buffer attributes
            module.buffer_SphereVB_stride = module.constructor.StructOffsets_VertexFormat._size;
            module.buffer_SphereVB_count = Math.floor(VBPadded.byteLength / module.buffer_SphereVB_stride);
            module.buffer_SphereVB_vertexBufferAttributes = module.constructor.StructVertexBufferAttributes_VertexFormat;
        }

        // Set the camera to be the exact binary values used by the pother platforms
        let viewProjMtxSrc = [
            0x3F7CDFB5, 0x00000000, 0x400CFC0F, 0xBE27456E,
            0xBF4D915C, 0x400FE0EC, 0x3EB85B26, 0xBF255AC1,
            0x38B2358B, 0x3818E824, 0xB81FD206, 0x3DCC5EFD,
            0xBF5984CA, 0xBEBAA298, 0x3EC312E2, 0x400C6EEC
        ];
        const viewProjMtx = new ArrayBuffer(64);
        const viewProjMtxView = new DataView(viewProjMtx);
        for (let i = 0; i < 16; ++i)
        {
            viewProjMtxView.setUint32(i*4, viewProjMtxSrc[i], true);
            module.variable_ViewProjMtx[i] = viewProjMtxView.getFloat32(i*4, true);
        }
        module.variable_ViewProjMtx = Shared.MatTranspose(module.variable_ViewProjMtx);

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_Final_Color_Buffer_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Final_Color_Buffer);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_Final_Color_Buffer_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_Final_Color_Buffer, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Textures/Mips_DrawCall/0.png");
            }
        );
    }
};
export var test_Textures_Mips_DrawCall = new Test_Textures_Mips_DrawCall();

class Test_Textures_Mips_VSPS_2D_WebGPU
{
    async Init(device, module)
    {
        module.texture_Output_usageFlags |= GPUTextureUsage.COPY_SRC;
        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_MipTex_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_MipTex, 1);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_MipTex_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_MipTex, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Mips_VSPS_2D/0.png", 1);
            }
        );
    }
};
export var test_Textures_Mips_VSPS_2D_WebGPU = new Test_Textures_Mips_VSPS_2D_WebGPU();

class Test_Textures_Texture2DArrayRW_CS
{
    async Init(device, module)
    {
        this.importedTexture = await Shared.CreateTextureWithPNG(device, "Image%i.png", module.texture_ImportedTexture_usageFlags);

        // Create ImportedTexture
        module.texture_ImportedTexture_size = this.importedTexture.size;
        module.texture_ImportedTexture_format = this.importedTexture.format;
        module.texture_ImportedTexture = device.createTexture({
            label: "texture Test_Textures_Texture2DArrayRW_CS.texture_ImportedTexture",
            size: module.texture_ImportedTexture_size,
            format: Shared.GetNonSRGBFormat(module.texture_ImportedTexture_format),
            usage: module.texture_ImportedTexture_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        // Create ImportedColor
        module.texture_ImportedColor_size = [64, 64, 3];
        module.texture_ImportedColor_format = "rgba8unorm";
        module.texture_ImportedColor = device.createTexture({
            label: "texture Test_Textures_Texture2DArrayRW_CS.texture_ImportedColor",
            size: module.texture_ImportedColor_size,
            format: Shared.GetNonSRGBFormat(module.texture_ImportedColor_format),
            usage: module.texture_ImportedColor_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        // fill with [ 128, 128, 128, 255 ]
        {
            let textureData = new Uint8Array(module.texture_ImportedColor_size[0]*module.texture_ImportedColor_size[1]*4);
            for (let i = 0; i < module.texture_ImportedColor_size[0]*module.texture_ImportedColor_size[1]; ++i)
            {
                textureData[i*4+0] = 128;
                textureData[i*4+1] = 128;
                textureData[i*4+2] = 128;
                textureData[i*4+3] = 255;
            }

            for (let i = 0; i < module.texture_ImportedColor_size[2]; i++)
            {
                device.queue.writeTexture(
                    { texture: module.texture_ImportedColor, origin: [0, 0, i] },
                    textureData,
                    { bytesPerRow: module.texture_ImportedColor_size[0]*4 },
                    { width: module.texture_ImportedColor_size[0], height: module.texture_ImportedColor_size[1] },
                );
            }
        }

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
        encoder.copyTextureToTexture(
            { texture: this.importedTexture.texture },
            { texture: module.texture_ImportedTexture },
            {
                width: Math.min(module.texture_ImportedTexture.width, this.importedTexture.texture.width),
                height: Math.min(module.texture_ImportedTexture.height, this.importedTexture.texture.height),
                depthOrArrayLayers: Math.min(module.texture_ImportedTexture.depthOrArrayLayers, this.importedTexture.texture.depthOrArrayLayers)
            }
        );
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        // Copy output to a readback buffer
        this.texture_ImportedTexture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_ImportedTexture);
        this.texture_NodeTexture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_NodeTexture);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        // Validate readback data
        await Shared.ReadbackBuffer(this.texture_ImportedTexture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_CS/0_0.png", 0, 0);
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_CS/1_0.png", 0, 1);
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_CS/2_0.png", 0, 2);
            }
        );

        await Shared.ReadbackBuffer(this.texture_NodeTexture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_CS/0_1.png", 0, 0);
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_CS/1_1.png", 0, 1);
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_CS/2_1.png", 0, 2);
            }
        );
    }
};
export var test_Textures_Texture2DArrayRW_CS = new Test_Textures_Texture2DArrayRW_CS();

class Test_Textures_Texture2DArrayRW_PS
{
    async Init(device, module)
    {
        this.importedTexture = await Shared.CreateTextureWithPNG(device, "Image%i.png", module.texture_ImportedTexture_usageFlags);

        // Create ImportedTexture
        module.texture_ImportedTexture_size = this.importedTexture.size;
        module.texture_ImportedTexture_format = this.importedTexture.format;
        module.texture_ImportedTexture = device.createTexture({
            label: "texture Test_Textures_Texture2DArrayRW_PS.texture_ImportedTexture",
            size: module.texture_ImportedTexture_size,
            format: Shared.GetNonSRGBFormat(module.texture_ImportedTexture_format),
            usage: module.texture_ImportedTexture_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        // Create ImportedColor
        module.texture_ImportedColor_size = [64, 64, 3];
        module.texture_ImportedColor_format = "rgba8unorm";
        module.texture_ImportedColor = device.createTexture({
            label: "texture Test_Textures_Texture2DArrayRW_PS.texture_ImportedColor",
            size: module.texture_ImportedColor_size,
            format: Shared.GetNonSRGBFormat(module.texture_ImportedColor_format),
            usage: module.texture_ImportedColor_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        // fill with [ 128, 128, 128, 255 ]
        {
            let textureData = new Uint8Array(module.texture_ImportedColor_size[0]*module.texture_ImportedColor_size[1]*4);
            for (let i = 0; i < module.texture_ImportedColor_size[0]*module.texture_ImportedColor_size[1]; ++i)
            {
                textureData[i*4+0] = 128;
                textureData[i*4+1] = 128;
                textureData[i*4+2] = 128;
                textureData[i*4+3] = 255;
            }

            for (let i = 0; i < module.texture_ImportedColor_size[2]; i++)
            {
                device.queue.writeTexture(
                    { texture: module.texture_ImportedColor, origin: [0, 0, i] },
                    textureData,
                    { bytesPerRow: module.texture_ImportedColor_size[0]*4 },
                    { width: module.texture_ImportedColor_size[0], height: module.texture_ImportedColor_size[1] },
                );
            }
        }

        // Create Color
        module.texture_Color_size = [64, 64, 3];
        module.texture_Color_format = "rgba8unorm";
        module.texture_Color = device.createTexture({
            label: "texture Test_Textures_Texture2DArrayRW_PS.texture_Color",
            size: module.texture_Color_size,
            format: Shared.GetNonSRGBFormat(module.texture_Color_format),
            usage: module.texture_Color_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        // fill with [ 255, 255, 255, 255 ]
        {
            let textureData = new Uint8Array(module.texture_Color_size[0]*module.texture_Color_size[1]*4);
            for (let i = 0; i < module.texture_Color_size[0]*module.texture_Color_size[1]; ++i)
            {
                textureData[i*4+0] = 255;
                textureData[i*4+1] = 255;
                textureData[i*4+2] = 255;
                textureData[i*4+3] = 255;
            }

            for (let i = 0; i < module.texture_Color_size[2]; i++)
            {
                device.queue.writeTexture(
                    { texture: module.texture_Color, origin: [0, 0, i] },
                    textureData,
                    { bytesPerRow: module.texture_Color_size[0]*4 },
                    { width: module.texture_Color_size[0], height: module.texture_Color_size[1] },
                );
            }
        }

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
        encoder.copyTextureToTexture(
            { texture: this.importedTexture.texture },
            { texture: module.texture_ImportedTexture },
            {
                width: Math.min(module.texture_ImportedTexture.width, this.importedTexture.texture.width),
                height: Math.min(module.texture_ImportedTexture.height, this.importedTexture.texture.height),
                depthOrArrayLayers: Math.min(module.texture_ImportedTexture.depthOrArrayLayers, this.importedTexture.texture.depthOrArrayLayers)
            }
        );
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        // Copy output to a readback buffer
        this.texture_ImportedTexture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_ImportedTexture);
        this.texture_NodeTexture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_NodeTexture);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        // Validate readback data
        await Shared.ReadbackBuffer(this.texture_ImportedTexture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_PS/0_0.png", 0, 0);
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_PS/1_0.png", 0, 1);
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_PS/2_0.png", 0, 2);
            }
        );

        await Shared.ReadbackBuffer(this.texture_NodeTexture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_PS/0_1.png", 0, 0);
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_PS/1_1.png", 0, 1);
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_PS/2_1.png", 0, 2);
            }
        );
    }
};
export var test_Textures_Texture2DArrayRW_PS = new Test_Textures_Texture2DArrayRW_PS();

class Test_Textures_Texture2DArrayRW_RGS
{
    async Init(device, module)
    {
        this.importedTexture = await Shared.CreateTextureWithPNG(device, "Image%i.png", module.texture_ImportedTexture_usageFlags);

        // Create ImportedTexture
        module.texture_ImportedTexture_size = this.importedTexture.size;
        module.texture_ImportedTexture_format = this.importedTexture.format;
        module.texture_ImportedTexture = device.createTexture({
            label: "texture Test_Textures_Texture2DArrayRW_RGS.texture_ImportedTexture",
            size: module.texture_ImportedTexture_size,
            format: Shared.GetNonSRGBFormat(module.texture_ImportedTexture_format),
            usage: module.texture_ImportedTexture_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        // Create ImportedColor
        module.texture_ImportedColor_size = [64, 64, 3];
        module.texture_ImportedColor_format = "rgba8unorm";
        module.texture_ImportedColor = device.createTexture({
            label: "texture Test_Textures_Texture2DArrayRW_RGS.texture_ImportedColor",
            size: module.texture_ImportedColor_size,
            format: Shared.GetNonSRGBFormat(module.texture_ImportedColor_format),
            usage: module.texture_ImportedColor_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        // fill with [ 255, 64, 128, 255 ]
        {
            let textureData = new Uint8Array(module.texture_ImportedColor_size[0]*module.texture_ImportedColor_size[1]*4);
            for (let i = 0; i < module.texture_ImportedColor_size[0]*module.texture_ImportedColor_size[1]; ++i)
            {
                textureData[i*4+0] = 255;
                textureData[i*4+1] = 64;
                textureData[i*4+2] = 128;
                textureData[i*4+3] = 255;
            }

            for (let i = 0; i < module.texture_ImportedColor_size[2]; i++)
            {
                device.queue.writeTexture(
                    { texture: module.texture_ImportedColor, origin: [0, 0, i] },
                    textureData,
                    { bytesPerRow: module.texture_ImportedColor_size[0]*4 },
                    { width: module.texture_ImportedColor_size[0], height: module.texture_ImportedColor_size[1] },
                );
            }
        }

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
        encoder.copyTextureToTexture(
            { texture: this.importedTexture.texture },
            { texture: module.texture_ImportedTexture },
            {
                width: Math.min(module.texture_ImportedTexture.width, this.importedTexture.texture.width),
                height: Math.min(module.texture_ImportedTexture.height, this.importedTexture.texture.height),
                depthOrArrayLayers: Math.min(module.texture_ImportedTexture.depthOrArrayLayers, this.importedTexture.texture.depthOrArrayLayers)
            }
        );
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        // Copy output to a readback buffer
        this.texture_ImportedTexture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_ImportedTexture);
        this.texture_NodeTexture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_NodeTexture);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        // Validate readback data
        await Shared.ReadbackBuffer(this.texture_ImportedTexture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_RGS/0_0.png", 0, 0);
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_RGS/1_0.png", 0, 1);
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_RGS/2_0.png", 0, 2);
            }
        );

        await Shared.ReadbackBuffer(this.texture_NodeTexture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_RGS/0_1.png", 0, 0);
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_RGS/1_1.png", 0, 1);
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DArrayRW_RGS/2_1.png", 0, 2);
            }
        );
    }
};
export var test_Textures_Texture2DArrayRW_RGS = new Test_Textures_Texture2DArrayRW_RGS();

class Test_Textures_Texture2DRW_CS
{
    async Init(device, module)
    {
        // Create ImportedColor
        module.texture_ImportedColor_size = [512, 512, 1];
        module.texture_ImportedColor_format = "rgba8unorm";
        module.texture_ImportedColor = device.createTexture({
            label: "texture Texture2DRW_CS.texture_ImportedColor",
            size: module.texture_ImportedColor_size,
            format: Shared.GetNonSRGBFormat(module.texture_ImportedColor_format),
            usage: module.texture_ImportedColor_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        // fill with [ 255, 64, 128, 255 ]
        {
            let textureData = new Uint8Array(module.texture_ImportedColor_size[0]*module.texture_ImportedColor_size[1]*4);
            for (let i = 0; i < module.texture_ImportedColor_size[0]*module.texture_ImportedColor_size[1]; ++i)
            {
                textureData[i*4+0] = 255;
                textureData[i*4+1] = 64;
                textureData[i*4+2] = 128;
                textureData[i*4+3] = 255;
            }

            device.queue.writeTexture(
                { texture: module.texture_ImportedColor },
                textureData,
                { bytesPerRow: module.texture_ImportedColor_size[0]*4 },
                { width: module.texture_ImportedColor_size[0], height: module.texture_ImportedColor_size[1] },
            );
        }

        // Create ImportedTexture
        module.texture_ImportedTexture_size = [512, 512, 1];
        module.texture_ImportedTexture_format = "rgba8unorm";
        module.texture_ImportedTexture = device.createTexture({
            label: "texture Texture2DRW_CS.texture_ImportedTexture",
            size: module.texture_ImportedTexture_size,
            format: Shared.GetNonSRGBFormat(module.texture_ImportedTexture_format),
            usage: module.texture_ImportedTexture_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        // fill with [ 255, 255, 255, 255 ]
        {
            let textureData = new Uint8Array(module.texture_ImportedTexture_size[0]*module.texture_ImportedTexture_size[1]*4);
            for (let i = 0; i < module.texture_ImportedTexture_size[0]*module.texture_ImportedTexture_size[1]; ++i)
            {
                textureData[i*4+0] = 255;
                textureData[i*4+1] = 255;
                textureData[i*4+2] = 255;
                textureData[i*4+3] = 255;
            }

            device.queue.writeTexture(
                { texture: module.texture_ImportedTexture },
                textureData,
                { bytesPerRow: module.texture_ImportedTexture_size[0]*4 },
                { width: module.texture_ImportedTexture_size[0], height: module.texture_ImportedTexture_size[1] },
            );
        }

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_ImportedTexture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_ImportedTexture);
        this.texture_NodeTexture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_NodeTexture);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_ImportedTexture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DRW_CS/0.png");
            }
        );

        await Shared.ReadbackBuffer(this.texture_NodeTexture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DRW_CS/1.png");
            }
        );
    }
};
export var test_Textures_Texture2DRW_CS = new Test_Textures_Texture2DRW_CS();

class Test_Textures_Texture2DRW_PS
{
    async Init(device, module)
    {
        // Create Color
        module.texture_Color_size = [512, 512, 1];
        module.texture_Color_format = "rgba8unorm";
        module.texture_Color = device.createTexture({
            label: "texture Texture2DRW_PS.texture_Color",
            size: module.texture_Color_size,
            format: Shared.GetNonSRGBFormat(module.texture_Color_format),
            usage: module.texture_Color_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        // fill with [ 255, 64, 128, 255 ]
        {
            let textureData = new Uint8Array(module.texture_Color_size[0]*module.texture_Color_size[1]*4);
            for (let i = 0; i < module.texture_Color_size[0]*module.texture_Color_size[1]; ++i)
            {
                textureData[i*4+0] = 255;
                textureData[i*4+1] = 64;
                textureData[i*4+2] = 128;
                textureData[i*4+3] = 255;
            }

            device.queue.writeTexture(
                { texture: module.texture_Color },
                textureData,
                { bytesPerRow: module.texture_Color_size[0]*4 },
                { width: module.texture_Color_size[0], height: module.texture_Color_size[1] },
            );
        }

        // Create ImportedColor
        module.texture_ImportedColor_size = [512, 512, 1];
        module.texture_ImportedColor_format = "rgba8unorm";
        module.texture_ImportedColor = device.createTexture({
            label: "texture Texture2DRW_PS.texture_ImportedColor",
            size: module.texture_ImportedColor_size,
            format: Shared.GetNonSRGBFormat(module.texture_ImportedColor_format),
            usage: module.texture_ImportedColor_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        // fill with [ 255, 64, 128, 255 ]
        {
            let textureData = new Uint8Array(module.texture_ImportedColor_size[0]*module.texture_ImportedColor_size[1]*4);
            for (let i = 0; i < module.texture_ImportedColor_size[0]*module.texture_ImportedColor_size[1]; ++i)
            {
                textureData[i*4+0] = 255;
                textureData[i*4+1] = 64;
                textureData[i*4+2] = 128;
                textureData[i*4+3] = 255;
            }

            device.queue.writeTexture(
                { texture: module.texture_ImportedColor },
                textureData,
                { bytesPerRow: module.texture_ImportedColor_size[0]*4 },
                { width: module.texture_ImportedColor_size[0], height: module.texture_ImportedColor_size[1] },
            );
        }

        // Create ImportedTexture
        module.texture_ImportedTexture_size = [512, 512, 1];
        module.texture_ImportedTexture_format = "rgba8unorm";
        module.texture_ImportedTexture = device.createTexture({
            label: "texture Texture2DRW_PS.texture_ImportedTexture",
            size: module.texture_ImportedTexture_size,
            format: Shared.GetNonSRGBFormat(module.texture_ImportedTexture_format),
            usage: module.texture_ImportedTexture_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        // fill with [ 255, 255, 255, 255 ]
        {
            let textureData = new Uint8Array(module.texture_ImportedTexture_size[0]*module.texture_ImportedTexture_size[1]*4);
            for (let i = 0; i < module.texture_ImportedTexture_size[0]*module.texture_ImportedTexture_size[1]; ++i)
            {
                textureData[i*4+0] = 255;
                textureData[i*4+1] = 255;
                textureData[i*4+2] = 255;
                textureData[i*4+3] = 255;
            }

            device.queue.writeTexture(
                { texture: module.texture_ImportedTexture },
                textureData,
                { bytesPerRow: module.texture_ImportedTexture_size[0]*4 },
                { width: module.texture_ImportedTexture_size[0], height: module.texture_ImportedTexture_size[1] },
            );
        }

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_ImportedTexture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_ImportedTexture);
        this.texture_NodeTexture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_NodeTexture);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_ImportedTexture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Textures/Texture2DRW_PS/0.png");
            }
        );

        await Shared.ReadbackBuffer(this.texture_NodeTexture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages_WebGPU/Textures/Texture2DRW_PS/1.png");
            }
        );
    }
};
export var test_Textures_Texture2DRW_PS = new Test_Textures_Texture2DRW_PS();

class Test_Textures_Texture2DRW_RGS
{
    async Init(device, module)
    {
        // Create ImportedColor
        module.texture_ImportedColor_size = [512, 512, 1];
        module.texture_ImportedColor_format = "rgba8unorm";
        module.texture_ImportedColor = device.createTexture({
            label: "texture Texture2DRW_CS.texture_ImportedColor",
            size: module.texture_ImportedColor_size,
            format: Shared.GetNonSRGBFormat(module.texture_ImportedColor_format),
            usage: module.texture_ImportedColor_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        // fill with [ 255, 64, 128, 255 ]
        {
            let textureData = new Uint8Array(module.texture_ImportedColor_size[0]*module.texture_ImportedColor_size[1]*4);
            for (let i = 0; i < module.texture_ImportedColor_size[0]*module.texture_ImportedColor_size[1]; ++i)
            {
                textureData[i*4+0] = 255;
                textureData[i*4+1] = 64;
                textureData[i*4+2] = 128;
                textureData[i*4+3] = 255;
            }

            device.queue.writeTexture(
                { texture: module.texture_ImportedColor },
                textureData,
                { bytesPerRow: module.texture_ImportedColor_size[0]*4 },
                { width: module.texture_ImportedColor_size[0], height: module.texture_ImportedColor_size[1] },
            );
        }

        // Create ImportedTexture
        module.texture_ImportedTexture_size = [512, 512, 1];
        module.texture_ImportedTexture_format = "rgba8unorm";
        module.texture_ImportedTexture = device.createTexture({
            label: "texture Texture2DRW_CS.texture_ImportedTexture",
            size: module.texture_ImportedTexture_size,
            format: Shared.GetNonSRGBFormat(module.texture_ImportedTexture_format),
            usage: module.texture_ImportedTexture_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
        });

        // fill with [ 255, 255, 255, 255 ]
        {
            let textureData = new Uint8Array(module.texture_ImportedTexture_size[0]*module.texture_ImportedTexture_size[1]*4);
            for (let i = 0; i < module.texture_ImportedTexture_size[0]*module.texture_ImportedTexture_size[1]; ++i)
            {
                textureData[i*4+0] = 255;
                textureData[i*4+1] = 255;
                textureData[i*4+2] = 255;
                textureData[i*4+3] = 255;
            }

            device.queue.writeTexture(
                { texture: module.texture_ImportedTexture },
                textureData,
                { bytesPerRow: module.texture_ImportedTexture_size[0]*4 },
                { width: module.texture_ImportedTexture_size[0], height: module.texture_ImportedTexture_size[1] },
            );
        }

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        this.texture_ImportedTexture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_ImportedTexture);
        this.texture_NodeTexture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_NodeTexture);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        await Shared.ReadbackBuffer(this.texture_ImportedTexture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DRW_RGS/0.png");
            }
        );

        await Shared.ReadbackBuffer(this.texture_NodeTexture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture2DRW_RGS/1.png");
            }
        );
    }
};
export var test_Textures_Texture2DRW_RGS = new Test_Textures_Texture2DRW_RGS();

class Test_Textures_Texture3DRW_CS
{
    async Init(device, module)
    {
        this.importedTexture = await Shared.CreateTextureWithPNG(device, "Image%i.png", module.texture_ImportedTexture_usageFlags, "3d");

        // Create ImportedTexture
        module.texture_ImportedTexture_size = this.importedTexture.size;
        module.texture_ImportedTexture_format = this.importedTexture.format;
        module.texture_ImportedTexture = device.createTexture({
            label: "texture Texture3DRW_CS.texture_ImportedTexture",
            size: module.texture_ImportedTexture_size,
            format: Shared.GetNonSRGBFormat(module.texture_ImportedTexture_format),
            usage: module.texture_ImportedTexture_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
            dimension: "3d",
        });

        // Create ImportedColor
        module.texture_ImportedColor_size = [64, 64, 3];
        module.texture_ImportedColor_format = "rgba8unorm";
        module.texture_ImportedColor = device.createTexture({
            label: "texture Texture3DRW_CS.texture_ImportedColor",
            size: module.texture_ImportedColor_size,
            format: Shared.GetNonSRGBFormat(module.texture_ImportedColor_format),
            usage: module.texture_ImportedColor_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
            dimension: "3d",
        });

        // fill with [ 128, 128, 128, 255 ]
        {
            let textureData = new Uint8Array(module.texture_ImportedColor_size[0]*module.texture_ImportedColor_size[1]*4);
            for (let i = 0; i < module.texture_ImportedColor_size[0]*module.texture_ImportedColor_size[1]; ++i)
            {
                textureData[i*4+0] = 128;
                textureData[i*4+1] = 128;
                textureData[i*4+2] = 128;
                textureData[i*4+3] = 255;
            }

            for (let i = 0; i < module.texture_ImportedColor_size[2]; i++)
            {
                device.queue.writeTexture(
                    { texture: module.texture_ImportedColor, origin: [0, 0, i] },
                    textureData,
                    { bytesPerRow: module.texture_ImportedColor_size[0]*4 },
                    { width: module.texture_ImportedColor_size[0], height: module.texture_ImportedColor_size[1] },
                );
            }
        }

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
        encoder.copyTextureToTexture(
            { texture: this.importedTexture.texture },
            { texture: module.texture_ImportedTexture },
            {
                width: Math.min(module.texture_ImportedTexture.width, this.importedTexture.texture.width),
                height: Math.min(module.texture_ImportedTexture.height, this.importedTexture.texture.height),
                depthOrArrayLayers: Math.min(module.texture_ImportedTexture.depthOrArrayLayers, this.importedTexture.texture.depthOrArrayLayers)
            }
        );
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        // Copy output to a readback buffer
        this.texture_ImportedTexture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_ImportedTexture);
        this.texture_NodeTexture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_NodeTexture);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        // Validate readback data
        await Shared.ReadbackBuffer(this.texture_ImportedTexture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture3DRW_CS/0_0.png", 0, 0);
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture3DRW_CS/0_1.png", 0, 1);
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture3DRW_CS/0_2.png", 0, 2);
            }
        );

        await Shared.ReadbackBuffer(this.texture_NodeTexture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture3DRW_CS/1_0.png", 0, 0);
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture3DRW_CS/1_1.png", 0, 1);
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture3DRW_CS/1_2.png", 0, 2);
            }
        );
    }
};
export var test_Textures_Texture3DRW_CS = new Test_Textures_Texture3DRW_CS();

class Test_Textures_Texture3DRW_RGS
{
    async Init(device, module)
    {
        this.importedTexture = await Shared.CreateTextureWithPNG(device, "Image%i.png", module.texture_ImportedTexture_usageFlags, "3d");

        // Create ImportedTexture
        module.texture_ImportedTexture_size = this.importedTexture.size;
        module.texture_ImportedTexture_format = this.importedTexture.format;
        module.texture_ImportedTexture = device.createTexture({
            label: "texture Texture3DRW_RGS.texture_ImportedTexture",
            size: module.texture_ImportedTexture_size,
            format: Shared.GetNonSRGBFormat(module.texture_ImportedTexture_format),
            usage: module.texture_ImportedTexture_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
            dimension: "3d",
        });

        // Create ImportedColor
        module.texture_ImportedColor_size = [64, 64, 3];
        module.texture_ImportedColor_format = "rgba8unorm";
        module.texture_ImportedColor = device.createTexture({
            label: "texture Texture3DRW_RGS.texture_ImportedColor",
            size: module.texture_ImportedColor_size,
            format: Shared.GetNonSRGBFormat(module.texture_ImportedColor_format),
            usage: module.texture_ImportedColor_usageFlags,
            viewFormats: ["rgba8unorm-srgb"],
            dimension: "3d",
        });

        // fill with [ 128, 128, 128, 255 ]
        {
            let textureData = new Uint8Array(module.texture_ImportedColor_size[0]*module.texture_ImportedColor_size[1]*4);
            for (let i = 0; i < module.texture_ImportedColor_size[0]*module.texture_ImportedColor_size[1]; ++i)
            {
                textureData[i*4+0] = 128;
                textureData[i*4+1] = 128;
                textureData[i*4+2] = 128;
                textureData[i*4+3] = 255;
            }

            for (let i = 0; i < module.texture_ImportedColor_size[2]; i++)
            {
                device.queue.writeTexture(
                    { texture: module.texture_ImportedColor, origin: [0, 0, i] },
                    textureData,
                    { bytesPerRow: module.texture_ImportedColor_size[0]*4 },
                    { width: module.texture_ImportedColor_size[0], height: module.texture_ImportedColor_size[1] },
                );
            }
        }

        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
        encoder.copyTextureToTexture(
            { texture: this.importedTexture.texture },
            { texture: module.texture_ImportedTexture },
            {
                width: Math.min(module.texture_ImportedTexture.width, this.importedTexture.texture.width),
                height: Math.min(module.texture_ImportedTexture.height, this.importedTexture.texture.height),
                depthOrArrayLayers: Math.min(module.texture_ImportedTexture.depthOrArrayLayers, this.importedTexture.texture.depthOrArrayLayers)
            }
        );
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        // Copy output to a readback buffer
        this.texture_ImportedTexture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_ImportedTexture);
        this.texture_NodeTexture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_NodeTexture);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        // Validate readback data
        await Shared.ReadbackBuffer(this.texture_ImportedTexture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture3DRW_RGS/0_0.png", 0, 0);
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture3DRW_RGS/0_1.png", 0, 1);
                VerifyDataViewEqualsPNGFile(view, module.texture_ImportedTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture3DRW_RGS/0_2.png", 0, 2);
            }
        );

        await Shared.ReadbackBuffer(this.texture_NodeTexture_Readback,
            (view) =>
            {
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture3DRW_RGS/1_0.png", 0, 0);
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture3DRW_RGS/1_1.png", 0, 1);
                VerifyDataViewEqualsPNGFile(view, module.texture_NodeTexture, "../../../../../../Techniques/UnitTests/_GoldImages/Textures/Texture3DRW_RGS/1_2.png", 0, 2);
            }
        );
    }
};
export var test_Textures_Texture3DRW_RGS = new Test_Textures_Texture3DRW_RGS();

class Test_Textures_TextureFormats
{
    // WebGPU texture formats listed: https://www.w3.org/TR/webgpu/#texture-formats
    // Limitations of texture formats for different uses are shown here: https://gpuweb.github.io/gpuweb/#plain-color-formats
    // Gigi turns read/write access into read only access and write only access, to give access to more formats.
    // * Read/write access:
    //   * r32 (uint, sint, float)
    // * Read only / write only access:
    //   * rgba8 (unorm, snorm, uint, sint)
    //   * rgba16 (uint, sint, float)
    //   * r32 (uint, sint, float)
    //   * rg32 (uint, sint, float)
    //   * rgba32 (uint, sint, float)
    // The formats commented out below are not supported for writes from compute shaders
    // Note: Wgsl wants to write 4 channels, even for formats with fewer channels. The extra channels are ignored.
    // WebGPU has a proposal for increasing the formats supported for read/write, through an extension: https://github.com/gpuweb/gpuweb/issues/3838
    // A pull request seems to be close to being accepted, which would let us open things up to more formats! https://github.com/gpuweb/gpuweb/pull/5160
    formats =
    [
        // rgba8 (unorm, snorm, uint, sint)
        [ "rgba8unorm", "RGBA8_Unorm", "vec4<f32>(0.25f, 0.75f, 1.0f, 4.0f)" ],
        [ "rgba8uint", "RGBA8_Uint", "vec4<u32>(255, 255, 255, 255)" ],
        [ "rgba8sint", "RGBA8_Sint", "vec4<i32>(127,127,127,127)" ],

        // rgba16 (uint, sint, float)
        [ "rgba16float", "RGBA16_Float", "vec4<f32>(0.25f, 0.75f, 1.0f, 4.0f)" ],

        // r32 (uint, sint, float)
        [ "r32float", "R32_Float", "vec4<f32>(0.25f, 0.75f, 1.0f, 4.0f)" ],
        [ "r32uint", "R32_Uint", "vec4<u32>(0x3e800000, 0, 0, 0)" ],

        // rg32 (uint, sint, float)
        [ "rg32float", "RG32_Float", "vec4<f32>(0.25f, 0.75f, 1.0f, 4.0f)" ],
        [ "rg32uint", "RG32_Uint", "vec4<u32>(0x3e800000, 0x3f400000, 0, 0)" ],

        // rgba32 (uint, sint, float)
        [ "rgba32float", "RGBA32_Float", "vec4<f32>(0.25f, 0.75f, 1.0f, 4.0f)" ],
        [ "rgba32uint", "RGBA32_Uint", "vec4<u32>(0x3e800000, 0x3f400000, 0x3f800000, 0x40800000)" ],

        // Unsupported types
        //[ "r8unorm", "R8_Unorm", "vec4<f32>(0.25f, 0.75f, 1.0f, 4.0f)" ],
        //[ "rg8unorm", "RG8_Unorm" ],
        //[ "rgba8unorm-srgb", "RGBA8_Unorm_sRGB" ],
        //[ "bgra8unorm", "BGRA8_Unorm" ],
        //[ "r8sint", "R8_Snorm" ],
        //[ "rg8sint", "RG8_Snorm" ],
        //[ "r8uint", "R8_Uint" ],
        //[ "rg8uint", "RG8_Uint" ],
        //[ "r8sint", "R8_Sint" ],
        //[ "rg8sint", "RG8_Sint" ],
        //[ "r16float", "R16_Float" ],
        //[ "rg16float", "RG16_Float" ],
        //[ "", "RGBA16_Unorm" ],  // no such type. closest is rgba16uint or rgba16float
        //[ "", "RGBA16_Snorm" ],  // no such type. closest is rgba16sint or rgba16float
        //[ "rg16uint", "RG16_Uint" ],
        //[ "rg11b10ufloat", "R11G11B10_Float" ],

        // Depth stencil formats can't be written to from compute shaders
        //[ "depth32float", "D32_Float" ],
        //[ "depth16unorm", "D16_Unorm" ],
        //[ "depth32float-stencil8", "D32_Float_S8" ],
        //[ "depth24plus-stencil8", "D24_Unorm_S8" ],

        // Block compressed formats can't be written to from compute shaders
        //[ "bc4-r-unorm", "BC4_Unorm" ],
        //[ "bc4-r-snorm", "BC4_Snorm" ],
        //[ "bc5-rg-unorm", "BC5_Unorm" ],
        //[ "bc5-rg-snorm", "BC5_Snorm" ],
        //[ "bc7-rgba-unorm", "BC7_Unorm" ],
        //[ "bc7-rgba-unorm-srgb", "BC7_Unorm_sRGB" ],
        //[ "bc6h-rgb-ufloat", "BC6_UF16" ],
        //[ "bc6h-rgb-float", "BC6_SF16" ],
    ];

    async Init(device, module)
    {
        return this.formats.length;
    }

    PreExecute(device, encoder, module, frameIndex)
    {
        if (frameIndex >= this.formats.length)
            return;

        const formatInfo = this.formats[frameIndex];

        module.valueToWrite = formatInfo[2];

        //console.log(formatInfo[0]);

        // Create Texture
        module.texture_Texture_size = [16, 16, 1];
        module.texture_Texture_format = formatInfo[0];
        module.texture_Texture = device.createTexture({
            label: "texture TextureFormats.texture_Texture: " + module.texture_Texture_format,
            size: module.texture_Texture_size,
            format: module.texture_Texture_format,
            usage: module.texture_Texture_usageFlags,
            dimension: "2d",
        });
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
        if (frameIndex >= this.formats.length)
            return;

        this.texture_Texture_Readback = Shared.GetReadbackBuffer_FromTexture(device, encoder, module.texture_Texture);
    }

    async NodeOnly_Validate(module, frameIndex)
    {
        if (frameIndex >= this.formats.length)
            return;

        const formatInfo = this.formats[frameIndex];

        await Shared.ReadbackBuffer(this.texture_Texture_Readback,
            (view) =>
            {
                let unpaddedView = UnpadTextureDataView(view, module.texture_Texture);
                VerifyDataViewEqualsBinaryFile(unpaddedView, formatInfo[0], "../../../../../../Techniques/UnitTests/_GoldImages/Textures/TextureFormats/" + formatInfo[1] + ".bin");
            }
        );
    }
};
export var test_Textures_TextureFormats = new Test_Textures_TextureFormats();

/*

Blank test to copy/paste/implement:

class Test_Compute_ReadbackSequence
{
    async Init(device, module)
    {
        Shared.LogError("Not yet implemented!");
        return 1; // Execute once
    }

    PreExecute(device, encoder, module, frameIndex)
    {
    }

    NodeOnly_PostExecute(device, encoder, module, frameIndex)
    {
    }

    async NodeOnly_Validate(module, frameIndex)
    {
    }
};
export var test_Compute_ReadbackSequence = new Test_Compute_ReadbackSequence();

*/
