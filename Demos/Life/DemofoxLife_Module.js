import * as Shared from './Shared.js';

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

class class_DemofoxLife
{
loadingPromises = new Set();
waitingOnPromises = false;

// -------------------- Shaders
// Shader code for Compute shader "Render", node "Render"
static ShaderCode_Render_Render = `
struct Struct_RenderCB_std140_0
{
    @align(16) FrameIndex_0 : i32,
    @align(4) Grid_0 : u32,
    @align(8) GridSize_0 : vec2<u32>,
    @align(16) RenderSize_0 : vec2<u32>,
    @align(8) _padding0_0 : f32,
    @align(4) _padding1_0 : f32,
};

@binding(3) @group(0) var<uniform> _RenderCB : Struct_RenderCB_std140_0;
@binding(1) @group(0) var Color : texture_storage_2d</*(Color_format)*/, write>;

@binding(0) @group(0) var GridState : texture_2d_array<u32>;

@compute
@workgroup_size(8, 8, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var simReadIndex_0 : i32 = (_RenderCB.FrameIndex_0 + i32(1)) % i32(2);
    var _S1 : vec2<i32> = vec2<i32>(DTid_0.xy);
    var uv_0 : vec2<f32> = vec2<f32>(_S1) / vec2<f32>(_RenderCB.RenderSize_0);
    var cellIndexF_0 : vec2<f32> = uv_0 * vec2<f32>(_RenderCB.GridSize_0);
    var _S2 : vec2<i32> = vec2<i32>(cellIndexF_0);
    var _S3 : vec2<u32> = _RenderCB.RenderSize_0 / _RenderCB.GridSize_0;
    var _S4 : vec2<i32> = vec2<i32>(_S3);
    var _S5 : bool;
    if(bool(_RenderCB.Grid_0))
    {
        _S5 = (_S4.x) >= i32(4);
    }
    else
    {
        _S5 = false;
    }
    if(_S5)
    {
        _S5 = (_S4.y) >= i32(4);
    }
    else
    {
        _S5 = false;
    }
    if(_S5)
    {
        var cellIndexFrac_0 : vec2<f32> = fract(cellIndexF_0);
        var cellIndexFrac_1 : vec2<f32> = min(cellIndexFrac_0, vec2<f32>(1.0f) - cellIndexFrac_0);
        if((min(cellIndexFrac_1.x, cellIndexFrac_1.y)) < 0.00999999977648258f)
        {
            textureStore((Color), (vec2<u32>(_S1)), (vec4<f32>(0.20000000298023224f, 0.20000000298023224f, 0.20000000298023224f, 1.0f)));
            return;
        }
    }
    var _S6 : vec4<i32> = vec4<i32>(vec4<u32>(vec3<u32>(vec2<u32>(_S2), u32(simReadIndex_0)), u32(0)));
    if((textureLoad((GridState), ((_S6)).xy, i32(((_S6)).z), ((_S6)).w).x) == u32(0))
    {
        textureStore((Color), (vec2<u32>(_S1)), (vec4<f32>(0.0f, 0.0f, 0.0f, 1.0f)));
    }
    else
    {
        textureStore((Color), (vec2<u32>(_S1)), (vec4<f32>(uv_0, 1.0f - uv_0.x, 1.0f)));
    }
    return;
}

`;

// Shader code for Compute shader "Simulate", node "Simulate"
static ShaderCode_Simulate_Simulate = `
struct Struct_SimulateCB_std140_0
{
    @align(16) Clear_0 : u32,
    @align(4) FrameIndex_0 : i32,
    @align(8) GridSize_0 : vec2<u32>,
    @align(16) MouseState_0 : vec4<f32>,
    @align(16) Pause_0 : u32,
    @align(4) RandomDensity_0 : f32,
    @align(8) Randomize_0 : u32,
    @align(4) _padding0_0 : f32,
    @align(16) RenderSize_0 : vec2<u32>,
    @align(8) SimPeriod_0 : u32,
    @align(4) SingleStep_0 : u32,
    @align(16) WrapAround_0 : u32,
    @align(4) iTime_0 : f32,
    @align(8) _padding1_0 : f32,
    @align(4) _padding2_0 : f32,
};

@binding(2) @group(0) var<uniform> _SimulateCB : Struct_SimulateCB_std140_0;
@binding(0) @group(0) var GridState : texture_storage_2d_array</*(GridState_format)*/, write>;

@binding(1) @group(0) var GridStateReadOnly : texture_storage_2d_array</*(GridStateReadOnly_format)*/, read>;

fn wang_hash_init_0( seed_0 : vec3<u32>) -> u32
{
    return ((seed_0.x * u32(1973) + seed_0.y * u32(9277) + seed_0.z * u32(26699)) | (u32(1)));
}

fn wang_hash_uint_0( seed_1 : ptr<function, u32>) -> u32
{
    var _S1 : u32 = (((((*seed_1) ^ (u32(61)))) ^ ((((*seed_1) >> (u32(16))))))) * u32(9);
    var _S2 : u32 = ((_S1 ^ (((_S1 >> (u32(4))))))) * u32(668265261);
    var _S3 : u32 = (_S2 ^ (((_S2 >> (u32(15))))));
    (*seed_1) = _S3;
    return _S3;
}

fn wang_hash_float01_0( state_0 : ptr<function, u32>) -> f32
{
    var _S4 : u32 = wang_hash_uint_0(&((*state_0)));
    return f32((_S4 & (u32(16777215)))) / 1.6777216e+07f;
}

@compute
@workgroup_size(8, 8, 1)
fn csmain(@builtin(global_invocation_id) DTid_0 : vec3<u32>)
{
    var _S5 : vec2<u32> = DTid_0.xy;
    var _S6 : vec2<i32> = vec2<i32>(_S5);
    var simReadIndex_0 : i32 = _SimulateCB.FrameIndex_0 % i32(2);
    var simWriteIndex_0 : i32 = (_SimulateCB.FrameIndex_0 + i32(1)) % i32(2);
    var _S7 : vec2<i32> = vec2<i32>(_SimulateCB.GridSize_0);
    var iy_0 : i32;
    if(bool(_SimulateCB.Randomize_0))
    {
        var rng_0 : u32 = wang_hash_init_0(vec3<u32>(_S5, (bitcast<u32>((_SimulateCB.iTime_0)))));
        var _S8 : f32 = wang_hash_float01_0(&(rng_0));
        if(_S8 < (_SimulateCB.RandomDensity_0))
        {
            iy_0 = i32(255);
        }
        else
        {
            iy_0 = i32(0);
        }
        var writeValue_0 : u32 = u32(iy_0);
        var _S9 : vec2<u32> = vec2<u32>(_S6);
        var _S10 : vec3<u32> = vec3<u32>(_S9, u32(0));
        textureStore((GridState), ((_S10)).xy, i32(((_S10)).z), vec4<u32>((writeValue_0), 0, 0, 1));
        var _S11 : vec3<u32> = vec3<u32>(_S9, u32(1));
        textureStore((GridState), ((_S11)).xy, i32(((_S11)).z), vec4<u32>((writeValue_0), 0, 0, 1));
        return;
    }
    var mouseState_0 : vec4<f32> = _SimulateCB.MouseState_0;
    var _S12 : bool = (_SimulateCB.MouseState_0.z) != 0.0f;
    var simulate_0 : bool;
    if(_S12)
    {
        simulate_0 = true;
    }
    else
    {
        simulate_0 = (mouseState_0.w) != 0.0f;
    }
    if(simulate_0)
    {
        if(_S12)
        {
            iy_0 = i32(255);
        }
        else
        {
            iy_0 = i32(0);
        }
        var writeValue_1 : u32 = u32(iy_0);
        var _S13 : vec2<i32> = vec2<i32>(mouseState_0.xy / vec2<f32>(_SimulateCB.RenderSize_0) * vec2<f32>(_S7));
        if((_S6.x) == (_S13.x))
        {
            simulate_0 = (_S6.y) == (_S13.y);
        }
        else
        {
            simulate_0 = false;
        }
        if(simulate_0)
        {
            if(bool(_SimulateCB.Pause_0))
            {
                var _S14 : vec2<u32> = vec2<u32>(_S6);
                var _S15 : vec3<u32> = vec3<u32>(_S14, u32(0));
                textureStore((GridState), ((_S15)).xy, i32(((_S15)).z), vec4<u32>((writeValue_1), 0, 0, 1));
                var _S16 : vec3<u32> = vec3<u32>(_S14, u32(1));
                textureStore((GridState), ((_S16)).xy, i32(((_S16)).z), vec4<u32>((writeValue_1), 0, 0, 1));
            }
            else
            {
                var _S17 : vec3<u32> = vec3<u32>(vec2<u32>(_S6), u32(simWriteIndex_0));
                textureStore((GridState), ((_S17)).xy, i32(((_S17)).z), vec4<u32>((writeValue_1), 0, 0, 1));
            }
            return;
        }
    }
    if((_SimulateCB.FrameIndex_0) < i32(1))
    {
        simulate_0 = true;
    }
    else
    {
        simulate_0 = bool(_SimulateCB.Clear_0);
    }
    if(simulate_0)
    {
        var _S18 : vec2<u32> = vec2<u32>(_S6);
        var _S19 : vec3<u32> = vec3<u32>(_S18, u32(0));
        textureStore((GridState), ((_S19)).xy, i32(((_S19)).z), vec4<u32>((u32(0)), 0, 0, 1));
        var _S20 : vec3<u32> = vec3<u32>(_S18, u32(1));
        textureStore((GridState), ((_S20)).xy, i32(((_S20)).z), vec4<u32>((u32(0)), 0, 0, 1));
        return;
    }
    if(bool(_SimulateCB.Pause_0))
    {
        simulate_0 = !bool(u32(!bool(_SimulateCB.SingleStep_0)));
    }
    else
    {
        var _S21 : u32 = u32(_SimulateCB.FrameIndex_0) % _SimulateCB.SimPeriod_0;
        simulate_0 = !(_S21 != u32(0));
    }
    if(!simulate_0)
    {
        var _S22 : vec2<u32> = vec2<u32>(_S6);
        var _S23 : vec3<u32> = vec3<u32>(_S22, u32(simWriteIndex_0));
        var _S24 : vec3<i32> = vec3<i32>(vec3<u32>(_S22, u32(simReadIndex_0)));
        var _S25 : u32 = (textureLoad((GridStateReadOnly), ((_S24)).xy, i32(((_S24)).z)).x);
        textureStore((GridState), ((_S23)).xy, i32(((_S23)).z), vec4<u32>((_S25), 0, 0, 1));
        return;
    }
    iy_0 = i32(-1);
    var neighborCount_0 : i32 = i32(0);
    for(;;)
    {
        if(iy_0 <= i32(1))
        {
        }
        else
        {
            break;
        }
        var ix_0 : i32 = i32(-1);
        var neighborCount_1 : i32 = neighborCount_0;
        for(;;)
        {
            if(ix_0 <= i32(1))
            {
            }
            else
            {
                break;
            }
            if(ix_0 == i32(0))
            {
                simulate_0 = iy_0 == i32(0);
            }
            else
            {
                simulate_0 = false;
            }
            if(simulate_0)
            {
                ix_0 = ix_0 + i32(1);
                continue;
            }
            var readpx_0 : vec2<i32> = _S6 + vec2<i32>(ix_0, iy_0);
            var readpxWrapped_0 : vec2<i32> = (readpx_0 + _S7) % _S7;
            var _S26 : bool;
            if(bool(u32(!bool(_SimulateCB.WrapAround_0))))
            {
                if((readpx_0.x) != (readpxWrapped_0.x))
                {
                    _S26 = true;
                }
                else
                {
                    _S26 = (readpx_0.y) != (readpxWrapped_0.y);
                }
            }
            else
            {
                _S26 = false;
            }
            if(_S26)
            {
                ix_0 = ix_0 + i32(1);
                continue;
            }
            var _S27 : vec3<i32> = vec3<i32>(vec3<u32>(vec2<u32>(readpxWrapped_0), u32(simReadIndex_0)));
            var _S28 : u32 = (textureLoad((GridStateReadOnly), ((_S27)).xy, i32(((_S27)).z)).x);
            var neighborCount_2 : i32;
            if(_S28 == u32(255))
            {
                neighborCount_2 = neighborCount_1 + i32(1);
            }
            else
            {
                neighborCount_2 = neighborCount_1;
            }
            neighborCount_1 = neighborCount_2;
            ix_0 = ix_0 + i32(1);
        }
        iy_0 = iy_0 + i32(1);
        neighborCount_0 = neighborCount_1;
    }
    switch(neighborCount_0)
    {
    case i32(2), :
        {
            var _S29 : vec2<u32> = vec2<u32>(_S6);
            var _S30 : vec3<u32> = vec3<u32>(_S29, u32(simWriteIndex_0));
            var _S31 : vec3<i32> = vec3<i32>(vec3<u32>(_S29, u32(simReadIndex_0)));
            var _S32 : u32 = (textureLoad((GridStateReadOnly), ((_S31)).xy, i32(((_S31)).z)).x);
            textureStore((GridState), ((_S30)).xy, i32(((_S30)).z), vec4<u32>((_S32), 0, 0, 1));
            break;
        }
    case i32(3), :
        {
            var _S33 : vec3<u32> = vec3<u32>(vec2<u32>(_S6), u32(simWriteIndex_0));
            textureStore((GridState), ((_S33)).xy, i32(((_S33)).z), vec4<u32>((u32(255)), 0, 0, 1));
            break;
        }
    case default, :
        {
            var _S34 : vec3<u32> = vec3<u32>(vec2<u32>(_S6), u32(simWriteIndex_0));
            textureStore((GridState), ((_S34)).xy, i32(((_S34)).z), vec4<u32>((u32(0)), 0, 0, 1));
            break;
        }
    }
    return;
}

`;

// -------------------- Private Members

// Texture GridState
texture_GridState = null;
texture_GridState_size = [0, 0, 0];
texture_GridState_format = "";
texture_GridState_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING | GPUTextureUsage.TEXTURE_BINDING;

// Texture Color
texture_Color = null;
texture_Color_size = [0, 0, 0];
texture_Color_format = "";
texture_Color_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Render_Color_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Render_Color_ReadOnly = null;
texture_Render_Color_ReadOnly_size = [0, 0, 0];
texture_Render_Color_ReadOnly_format = "";
texture_Render_Color_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Texture Simulate_GridState_ReadOnly : This is a read only texture copy to help get around read-write storage texture format limitations.
texture_Simulate_GridState_ReadOnly = null;
texture_Simulate_GridState_ReadOnly_size = [0, 0, 0];
texture_Simulate_GridState_ReadOnly_format = "";
texture_Simulate_GridState_ReadOnly_usageFlags = GPUTextureUsage.COPY_DST | GPUTextureUsage.COPY_SRC | GPUTextureUsage.STORAGE_BINDING;

// Constant buffer _SimulateCB
constantBuffer__SimulateCB = null;
constantBuffer__SimulateCB_size = 80;
constantBuffer__SimulateCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader Simulate
Hash_Compute_Simulate = 0;
ShaderModule_Compute_Simulate = null;
BindGroupLayout_Compute_Simulate = null;
PipelineLayout_Compute_Simulate = null;
Pipeline_Compute_Simulate = null;

// Constant buffer _RenderCB
constantBuffer__RenderCB = null;
constantBuffer__RenderCB_size = 32;
constantBuffer__RenderCB_usageFlags = GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM;

// Compute Shader Render
Hash_Compute_Render = 0;
ShaderModule_Compute_Render = null;
BindGroupLayout_Compute_Render = null;
PipelineLayout_Compute_Render = null;
Pipeline_Compute_Render = null;

// -------------------- Public Variables

variable_Clear = false;
variableDefault_Clear = false;
variableChanged_Clear = false;
variable_GridSize = [ 32, 32 ];  // The size of the simulation grid
variableDefault_GridSize = [ 32, 32 ];  // The size of the simulation grid
variableChanged_GridSize = [ false, false ];
variable_Grid = true;
variableDefault_Grid = true;
variableChanged_Grid = false;
variable_WrapAround = true;  // If true, the right most cells are neighbors to the left most, and the top are neighbors to the bottom.
variableDefault_WrapAround = true;  // If true, the right most cells are neighbors to the left most, and the top are neighbors to the bottom.
variableChanged_WrapAround = false;
variable_RenderSize = [ 512, 512 ];  // The size of the rendered output
variableDefault_RenderSize = [ 512, 512 ];  // The size of the rendered output
variableChanged_RenderSize = [ false, false ];
variable_SimPeriod = 5;  // A frame step is done when this many frames have been rendered. 1 is as fast as possible.
variableDefault_SimPeriod = 5;  // A frame step is done when this many frames have been rendered. 1 is as fast as possible.
variableChanged_SimPeriod = false;
variable_Pause = false;
variableDefault_Pause = false;
variableChanged_Pause = false;
variable_SingleStep = false;
variableDefault_SingleStep = false;
variableChanged_SingleStep = false;
variable_RandomDensity = 0.500000;
variableDefault_RandomDensity = 0.500000;
variableChanged_RandomDensity = false;
variable_Randomize = false;
variableDefault_Randomize = false;
variableChanged_Randomize = false;

// -------------------- Private Variables

variable_MouseState = [ 0.000000, 0.000000, 0.000000, 0.000000 ];
variableDefault_MouseState = [ 0.000000, 0.000000, 0.000000, 0.000000 ];
variableChanged_MouseState = [ false, false, false, false ];
variable_FrameIndex = 0;
variableDefault_FrameIndex = 0;
variableChanged_FrameIndex = false;
variable_iTime = 0.000000;
variableDefault_iTime = 0.000000;
variableChanged_iTime = false;
// -------------------- Structs

static StructOffsets__SimulateCB =
{
    Clear: 0,
    FrameIndex: 4,
    GridSize_0: 8,
    GridSize_1: 12,
    MouseState_0: 16,
    MouseState_1: 20,
    MouseState_2: 24,
    MouseState_3: 28,
    Pause: 32,
    RandomDensity: 36,
    Randomize: 40,
    _padding0: 44,
    RenderSize_0: 48,
    RenderSize_1: 52,
    SimPeriod: 56,
    SingleStep: 60,
    WrapAround: 64,
    iTime: 68,
    _padding1: 72,
    _padding2: 76,
    _size: 80,
}

static StructOffsets__RenderCB =
{
    FrameIndex: 0,
    Grid: 4,
    GridSize_0: 8,
    GridSize_1: 12,
    RenderSize_0: 16,
    RenderSize_1: 20,
    _padding0: 24,
    _padding1: 28,
    _size: 32,
}


async SetVarsBefore()
{
    {
        this.variableChanged_GridSize[0] = true;
        this.variable_GridSize[0] = Math.max(this.variable_GridSize[0], 1);
    }

    {
        this.variableChanged_GridSize[1] = true;
        this.variable_GridSize[1] = Math.max(this.variable_GridSize[1], 1);
    }

    {
        this.variableChanged_RenderSize[0] = true;
        this.variable_RenderSize[0] = Math.max(this.variable_RenderSize[0], 16);
    }

    {
        this.variableChanged_RenderSize[1] = true;
        this.variable_RenderSize[1] = Math.max(this.variable_RenderSize[1], 16);
    }

    {
        this.variableChanged_SimPeriod = true;
        this.variable_SimPeriod = Math.max(this.variable_SimPeriod, 1);
    }


}

async SetVarsAfter()
{
    {
        this.variableChanged_FrameIndex = true;
        this.variable_FrameIndex = this.variable_FrameIndex + 1;
    }


}

async ValidateImports()
{
    return true;
}

async Init(device, encoder, useBlockingAPIs)
{
    // Handle (re)creation of texture GridState
    {
        const baseSize = [ this.variable_GridSize[0], this.variable_GridSize[1], 1];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 2) / 1) + 0
        ];
        const desiredFormat = "r32uint";
        if (this.texture_GridState !== null && (this.texture_GridState_format != desiredFormat || this.texture_GridState_size[0] != desiredSize[0] || this.texture_GridState_size[1] != desiredSize[1] || this.texture_GridState_size[2] != desiredSize[2]))
        {
            this.texture_GridState.destroy();
            this.texture_GridState = null;
        }

        if (this.texture_GridState === null)
        {
            this.texture_GridState_size = desiredSize.slice();
            this.texture_GridState_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_GridState_format))
                viewFormats.push(this.texture_GridState_format);

            this.texture_GridState = device.createTexture({
                label: "texture DemofoxLife.GridState",
                size: this.texture_GridState_size,
                format: Shared.GetNonSRGBFormat(this.texture_GridState_format),
                usage: this.texture_GridState_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Color
    {
        const baseSize = [ this.variable_RenderSize[0], this.variable_RenderSize[1], 1];
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = "rgba8unorm-srgb";
        if (this.texture_Color !== null && (this.texture_Color_format != desiredFormat || this.texture_Color_size[0] != desiredSize[0] || this.texture_Color_size[1] != desiredSize[1] || this.texture_Color_size[2] != desiredSize[2]))
        {
            this.texture_Color.destroy();
            this.texture_Color = null;
        }

        if (this.texture_Color === null)
        {
            this.texture_Color_size = desiredSize.slice();
            this.texture_Color_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Color_format))
                viewFormats.push(this.texture_Color_format);

            this.texture_Color = device.createTexture({
                label: "texture DemofoxLife.Color",
                size: this.texture_Color_size,
                format: Shared.GetNonSRGBFormat(this.texture_Color_format),
                usage: this.texture_Color_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Render_Color_ReadOnly
    {
        const baseSize = this.texture_Color_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_Color_format;
        if (this.texture_Render_Color_ReadOnly !== null && (this.texture_Render_Color_ReadOnly_format != desiredFormat || this.texture_Render_Color_ReadOnly_size[0] != desiredSize[0] || this.texture_Render_Color_ReadOnly_size[1] != desiredSize[1] || this.texture_Render_Color_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Render_Color_ReadOnly.destroy();
            this.texture_Render_Color_ReadOnly = null;
        }

        if (this.texture_Render_Color_ReadOnly === null)
        {
            this.texture_Render_Color_ReadOnly_size = desiredSize.slice();
            this.texture_Render_Color_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Render_Color_ReadOnly_format))
                viewFormats.push(this.texture_Render_Color_ReadOnly_format);

            this.texture_Render_Color_ReadOnly = device.createTexture({
                label: "texture DemofoxLife.Render_Color_ReadOnly",
                size: this.texture_Render_Color_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Render_Color_ReadOnly_format),
                usage: this.texture_Render_Color_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // Handle (re)creation of texture Simulate_GridState_ReadOnly
    {
        const baseSize = this.texture_GridState_size.slice();
        const desiredSize = [
            Math.floor(((parseInt(baseSize[0]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[1]) + 0) * 1) / 1) + 0,
            Math.floor(((parseInt(baseSize[2]) + 0) * 1) / 1) + 0
        ];
        const desiredFormat = this.texture_GridState_format;
        if (this.texture_Simulate_GridState_ReadOnly !== null && (this.texture_Simulate_GridState_ReadOnly_format != desiredFormat || this.texture_Simulate_GridState_ReadOnly_size[0] != desiredSize[0] || this.texture_Simulate_GridState_ReadOnly_size[1] != desiredSize[1] || this.texture_Simulate_GridState_ReadOnly_size[2] != desiredSize[2]))
        {
            this.texture_Simulate_GridState_ReadOnly.destroy();
            this.texture_Simulate_GridState_ReadOnly = null;
        }

        if (this.texture_Simulate_GridState_ReadOnly === null)
        {
            this.texture_Simulate_GridState_ReadOnly_size = desiredSize.slice();
            this.texture_Simulate_GridState_ReadOnly_format = desiredFormat;

            let viewFormats = [];
            if (Shared.IsSRGBFormat(this.texture_Simulate_GridState_ReadOnly_format))
                viewFormats.push(this.texture_Simulate_GridState_ReadOnly_format);

            this.texture_Simulate_GridState_ReadOnly = device.createTexture({
                label: "texture DemofoxLife.Simulate_GridState_ReadOnly",
                size: this.texture_Simulate_GridState_ReadOnly_size,
                format: Shared.GetNonSRGBFormat(this.texture_Simulate_GridState_ReadOnly_format),
                usage: this.texture_Simulate_GridState_ReadOnly_usageFlags,
                viewFormats: viewFormats,
            });
        }
    }

    // (Re)create compute shader Simulate
    {
        const bindGroupEntries =
        [
            {
                // GridState
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_GridState_format), viewDimension: "2d-array" }
            },
            {
                // GridStateReadOnly
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Simulate_GridState_ReadOnly_format), viewDimension: "2d-array" }
            },
            {
                // _SimulateCB
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Simulate === null || newHash !== this.Hash_Compute_Simulate)
        {
            this.Hash_Compute_Simulate = newHash;

            let shaderCode = class_DemofoxLife.ShaderCode_Simulate_Simulate;
            shaderCode = shaderCode.replace("/*(GridState_format)*/", Shared.GetNonSRGBFormat(this.texture_GridState_format));
            shaderCode = shaderCode.replace("/*(GridStateReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Simulate_GridState_ReadOnly_format));

            this.ShaderModule_Compute_Simulate = device.createShaderModule({ code: shaderCode, label: "Compute Shader Simulate"});
            this.BindGroupLayout_Compute_Simulate = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Simulate",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Simulate = device.createPipelineLayout({
                label: "Compute Pipeline Layout Simulate",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Simulate],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Simulate = device.createComputePipeline({
                    label: "Compute Pipeline Simulate",
                    layout: this.PipelineLayout_Compute_Simulate,
                    compute: {
                        module: this.ShaderModule_Compute_Simulate,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Simulate");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Simulate",
                    layout: this.PipelineLayout_Compute_Simulate,
                    compute: {
                        module: this.ShaderModule_Compute_Simulate,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_Simulate = handle; this.loadingPromises.delete("Simulate"); } );
            }
        }
    }

    // (Re)create compute shader Render
    {
        const bindGroupEntries =
        [
            {
                // GridState
                binding: 0,
                visibility: GPUShaderStage.COMPUTE,
                texture : { viewDimension: "2d-array", sampleType: Shared.GetTextureFormatInfo(this.texture_GridState_format).sampleType }
            },
            {
                // Color
                binding: 1,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "write-only", format: Shared.GetNonSRGBFormat(this.texture_Color_format), viewDimension: "2d" }
            },
            {
                // ColorReadOnly
                binding: 2,
                visibility: GPUShaderStage.COMPUTE,
                storageTexture : { access: "read-only", format: Shared.GetNonSRGBFormat(this.texture_Render_Color_ReadOnly_format), viewDimension: "2d" }
            },
            {
                // _RenderCB
                binding: 3,
                visibility: GPUShaderStage.COMPUTE,
                buffer: { type: "uniform" }
            },
        ];

        let hashString = JSON.stringify(bindGroupEntries);
        const newHash = hashString.hashCode();

        if (this.ShaderModule_Compute_Render === null || newHash !== this.Hash_Compute_Render)
        {
            this.Hash_Compute_Render = newHash;

            let shaderCode = class_DemofoxLife.ShaderCode_Render_Render;
            shaderCode = shaderCode.replace("/*(Color_format)*/", Shared.GetNonSRGBFormat(this.texture_Color_format));
            shaderCode = shaderCode.replace("/*(ColorReadOnly_format)*/", Shared.GetNonSRGBFormat(this.texture_Render_Color_ReadOnly_format));

            this.ShaderModule_Compute_Render = device.createShaderModule({ code: shaderCode, label: "Compute Shader Render"});
            this.BindGroupLayout_Compute_Render = device.createBindGroupLayout({
                label: "Compute Bind Group Layout Render",
                entries: bindGroupEntries
            });

            this.PipelineLayout_Compute_Render = device.createPipelineLayout({
                label: "Compute Pipeline Layout Render",
                bindGroupLayouts: [this.BindGroupLayout_Compute_Render],
            });

            if (useBlockingAPIs)
            {
                this.Pipeline_Compute_Render = device.createComputePipeline({
                    label: "Compute Pipeline Render",
                    layout: this.PipelineLayout_Compute_Render,
                    compute: {
                        module: this.ShaderModule_Compute_Render,
                        entryPoint: "csmain",
                    }
                });
            }
            else
            {
                this.loadingPromises.add("Render");

                device.createComputePipelineAsync({
                    label: "Compute Pipeline Render",
                    layout: this.PipelineLayout_Compute_Render,
                    compute: {
                        module: this.ShaderModule_Compute_Render,
                        entryPoint: "csmain",
                    }
                }).then( handle => { this.Pipeline_Compute_Render = handle; this.loadingPromises.delete("Render"); } );
            }
        }
    }

}

async FillEncoder(device, encoder)
{
    encoder.pushDebugGroup("DemofoxLife.GridState");

    encoder.popDebugGroup(); // "DemofoxLife.GridState"

    encoder.pushDebugGroup("DemofoxLife.Color");

    encoder.popDebugGroup(); // "DemofoxLife.Color"

    encoder.pushDebugGroup("DemofoxLife.Render_Color_ReadOnly");

    encoder.popDebugGroup(); // "DemofoxLife.Render_Color_ReadOnly"

    encoder.pushDebugGroup("DemofoxLife.Copy_Render_Color");

    // Copy texture Color to texture Render_Color_ReadOnly
    {
        const numMips = Math.min(this.texture_Color.mipLevelCount, this.texture_Render_Color_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_Color.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_Color.height >> mipIndex, 1);
            let mipDepth = this.texture_Color.depthOrArrayLayers;

            if (this.texture_Color.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_Color, mipLevel: mipIndex },
                { texture: this.texture_Render_Color_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "DemofoxLife.Copy_Render_Color"

    encoder.pushDebugGroup("DemofoxLife.Simulate_GridState_ReadOnly");

    encoder.popDebugGroup(); // "DemofoxLife.Simulate_GridState_ReadOnly"

    encoder.pushDebugGroup("DemofoxLife.Copy_Simulate_GridState");

    // Copy texture GridState to texture Simulate_GridState_ReadOnly
    {
        const numMips = Math.min(this.texture_GridState.mipLevelCount, this.texture_Simulate_GridState_ReadOnly.mipLevelCount);

        for (let mipIndex = 0; mipIndex < numMips; ++mipIndex)
        {
            const mipWidth = Math.max(this.texture_GridState.width >> mipIndex, 1);
            const mipHeight = Math.max(this.texture_GridState.height >> mipIndex, 1);
            let mipDepth = this.texture_GridState.depthOrArrayLayers;

            if (this.texture_GridState.dimension == "3d")
                mipDepth = Math.max(mipDepth >> mipIndex, 1);

            encoder.copyTextureToTexture(
                { texture: this.texture_GridState, mipLevel: mipIndex },
                { texture: this.texture_Simulate_GridState_ReadOnly, mipLevel: mipIndex },
                {
                    width: mipWidth,
                    height: mipHeight,
                    depthOrArrayLayers: mipDepth,
                }
            );
        }
    }

    encoder.popDebugGroup(); // "DemofoxLife.Copy_Simulate_GridState"

    encoder.pushDebugGroup("DemofoxLife._SimulateCB");

    // Create constant buffer _SimulateCB
    if (this.constantBuffer__SimulateCB === null)
    {
        this.constantBuffer__SimulateCB = device.createBuffer({
            label: "DemofoxLife._SimulateCB",
            size: Shared.Align(16, this.constructor.StructOffsets__SimulateCB._size),
            usage: this.constantBuffer__SimulateCB_usageFlags,
        });
    }

    // Upload values to constant buffer _SimulateCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__SimulateCB._size));
        const view = new DataView(bufferCPU);
        view.setUint32(this.constructor.StructOffsets__SimulateCB.Clear, (this.variable_Clear === true ? 1 : 0), true);
        view.setInt32(this.constructor.StructOffsets__SimulateCB.FrameIndex, this.variable_FrameIndex, true);
        view.setUint32(this.constructor.StructOffsets__SimulateCB.GridSize_0, this.variable_GridSize[0], true);
        view.setUint32(this.constructor.StructOffsets__SimulateCB.GridSize_1, this.variable_GridSize[1], true);
        view.setFloat32(this.constructor.StructOffsets__SimulateCB.MouseState_0, this.variable_MouseState[0], true);
        view.setFloat32(this.constructor.StructOffsets__SimulateCB.MouseState_1, this.variable_MouseState[1], true);
        view.setFloat32(this.constructor.StructOffsets__SimulateCB.MouseState_2, this.variable_MouseState[2], true);
        view.setFloat32(this.constructor.StructOffsets__SimulateCB.MouseState_3, this.variable_MouseState[3], true);
        view.setUint32(this.constructor.StructOffsets__SimulateCB.Pause, (this.variable_Pause === true ? 1 : 0), true);
        view.setFloat32(this.constructor.StructOffsets__SimulateCB.RandomDensity, this.variable_RandomDensity, true);
        view.setUint32(this.constructor.StructOffsets__SimulateCB.Randomize, (this.variable_Randomize === true ? 1 : 0), true);
        view.setUint32(this.constructor.StructOffsets__SimulateCB.RenderSize_0, this.variable_RenderSize[0], true);
        view.setUint32(this.constructor.StructOffsets__SimulateCB.RenderSize_1, this.variable_RenderSize[1], true);
        view.setUint32(this.constructor.StructOffsets__SimulateCB.SimPeriod, this.variable_SimPeriod, true);
        view.setUint32(this.constructor.StructOffsets__SimulateCB.SingleStep, (this.variable_SingleStep === true ? 1 : 0), true);
        view.setUint32(this.constructor.StructOffsets__SimulateCB.WrapAround, (this.variable_WrapAround === true ? 1 : 0), true);
        view.setFloat32(this.constructor.StructOffsets__SimulateCB.iTime, this.variable_iTime, true);
        device.queue.writeBuffer(this.constantBuffer__SimulateCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "DemofoxLife._SimulateCB"

    encoder.pushDebugGroup("DemofoxLife.Simulate");

    // Run compute shader Simulate
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Simulate",
            layout: this.BindGroupLayout_Compute_Simulate,
            entries: [
                {
                    // GridState
                    binding: 0,
                    resource: this.texture_GridState.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // GridStateReadOnly
                    binding: 1,
                    resource: this.texture_Simulate_GridState_ReadOnly.createView({ dimension: "2d-array", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _SimulateCB
                    binding: 2,
                    resource: { buffer: this.constantBuffer__SimulateCB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_GridState_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Simulate !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Simulate);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "DemofoxLife.Simulate"

    encoder.pushDebugGroup("DemofoxLife._RenderCB");

    // Create constant buffer _RenderCB
    if (this.constantBuffer__RenderCB === null)
    {
        this.constantBuffer__RenderCB = device.createBuffer({
            label: "DemofoxLife._RenderCB",
            size: Shared.Align(16, this.constructor.StructOffsets__RenderCB._size),
            usage: this.constantBuffer__RenderCB_usageFlags,
        });
    }

    // Upload values to constant buffer _RenderCB
    {
        const bufferCPU = new ArrayBuffer(Shared.Align(16, this.constructor.StructOffsets__RenderCB._size));
        const view = new DataView(bufferCPU);
        view.setInt32(this.constructor.StructOffsets__RenderCB.FrameIndex, this.variable_FrameIndex, true);
        view.setUint32(this.constructor.StructOffsets__RenderCB.Grid, (this.variable_Grid === true ? 1 : 0), true);
        view.setUint32(this.constructor.StructOffsets__RenderCB.GridSize_0, this.variable_GridSize[0], true);
        view.setUint32(this.constructor.StructOffsets__RenderCB.GridSize_1, this.variable_GridSize[1], true);
        view.setUint32(this.constructor.StructOffsets__RenderCB.RenderSize_0, this.variable_RenderSize[0], true);
        view.setUint32(this.constructor.StructOffsets__RenderCB.RenderSize_1, this.variable_RenderSize[1], true);
        device.queue.writeBuffer(this.constantBuffer__RenderCB, 0, bufferCPU);
    }

    encoder.popDebugGroup(); // "DemofoxLife._RenderCB"

    encoder.pushDebugGroup("DemofoxLife.Render");

    // Run compute shader Render
    {
        const bindGroup = device.createBindGroup({
            label: "Compute Bind Group Render",
            layout: this.BindGroupLayout_Compute_Render,
            entries: [
                {
                    // GridState
                    binding: 0,
                    resource: this.texture_GridState.createView({ dimension: "2d-array", format: this.texture_GridState_format, usage: GPUTextureUsage.TEXTURE_BINDING })
                },
                {
                    // Color
                    binding: 1,
                    resource: this.texture_Color.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // ColorReadOnly
                    binding: 2,
                    resource: this.texture_Render_Color_ReadOnly.createView({ dimension: "2d", mipLevelCount: 1, baseMipLevel: 0 })
                },
                {
                    // _RenderCB
                    binding: 3,
                    resource: { buffer: this.constantBuffer__RenderCB }
                },
            ]
        });

        // Calculate dispatch size
        const baseDispatchSize = this.texture_Color_size;
        const dispatchSize = [
            Math.floor((Math.floor(((parseInt(baseDispatchSize[0]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[1]) + 0) * 1) / 1) + 0 + 8 - 1) / 8),
            Math.floor((Math.floor(((parseInt(baseDispatchSize[2]) + 0) * 1) / 1) + 0 + 1 - 1) / 1)
        ];

        if (this.Pipeline_Compute_Render !== null)
        {
            const computePass = encoder.beginComputePass();
                computePass.setPipeline(this.Pipeline_Compute_Render);
                computePass.setBindGroup(0, bindGroup);
                computePass.dispatchWorkgroups(dispatchSize[0], dispatchSize[1], dispatchSize[2]);
            computePass.end();
        }
    }

    encoder.popDebugGroup(); // "DemofoxLife.Render"

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

var DemofoxLife = new class_DemofoxLife;

export default DemofoxLife;
