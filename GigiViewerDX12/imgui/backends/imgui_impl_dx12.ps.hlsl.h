static const char* pixelShader =
R"=====(
static const uint ImGuiShaderFlag_HideR = 1 << 0;
static const uint ImGuiShaderFlag_HideG = 1 << 1;
static const uint ImGuiShaderFlag_HideB = 1 << 2;
static const uint ImGuiShaderFlag_HideA = 1 << 3;
static const uint ImGuiShaderFlag_Checker = 1 << 4;
static const uint ImGuiShaderFlag_Nearest = 1 << 5;
static const uint ImGuiShaderFlag_UINTByteCountBit1 = 1 << 6;
static const uint ImGuiShaderFlag_UINTByteCountBit2 = 1 << 7;
static const uint ImGuiShaderFlag_UINTByteCountBit3 = 1 << 8;
static const uint ImGuiShaderFlag_Signed = 1 << 9;
static const uint ImGuiShaderFlag_Clamp = 1 << 10;

static const uint ImGuiShaderFlag_HideBits = ImGuiShaderFlag_HideR | ImGuiShaderFlag_HideG | ImGuiShaderFlag_HideB | ImGuiShaderFlag_HideA;

cbuffer vertexBuffer : register(b0)
{
    uint flags;
    float2 histogramMinMax;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0);

float4 main(PS_INPUT input) : SV_Target
{
    uint flagChannelBits = (flags & ImGuiShaderFlag_HideBits) ^ ImGuiShaderFlag_HideBits;
    bool singleChannel = (flagChannelBits != 0) && ((flagChannelBits & flagChannelBits-1)==0);
    float checker = ((floor(input.pos.x / 10) + floor(input.pos.y / 10)) % 2) ? 0.9f : 0.7f;
    float4 mask = float4(
        (flags & ImGuiShaderFlag_HideR) ? 0.0f : 1.0f,
        (flags & ImGuiShaderFlag_HideG) ? 0.0f : 1.0f,
        (flags & ImGuiShaderFlag_HideB) ? 0.0f : 1.0f,
        (flags & ImGuiShaderFlag_HideA) ? 0.0f : 1.0f);
    float2 uv = input.uv;
    if (flags & ImGuiShaderFlag_Clamp)
    {
        uint w,h;
        texture0.GetDimensions(w, h);
        float2 minUV = float2(0.5f, 0.5f) / float2(w, h);
        float2 maxUV = (float2(w,h) - float2(0.5f, 0.5f)) / float2(w, h);
        uv = clamp(uv, minUV, maxUV);
    }
    if (flags & ImGuiShaderFlag_Nearest)
    {
        uint w,h;
        texture0.GetDimensions(w, h);
        uv = (floor(uv * float2(w,h)) + 0.5f) / float2(w,h);
    }
    uint byteCount = ((flags & ImGuiShaderFlag_UINTByteCountBit1)/ImGuiShaderFlag_UINTByteCountBit1) | ((flags & ImGuiShaderFlag_UINTByteCountBit2)/ImGuiShaderFlag_UINTByteCountBit1) | ((flags & ImGuiShaderFlag_UINTByteCountBit3)/ImGuiShaderFlag_UINTByteCountBit1);
    float4 textureSample = float4(0.0f, 0.0f, 0.0f, 0.0f);
    if (byteCount != 0)
    {
        if (flags & ImGuiShaderFlag_Signed)
            textureSample = float4(asint(texture0.Sample(sampler0, uv)));
        else
            textureSample = float4(asuint(texture0.Sample(sampler0, uv)));
    }
    else
    {
        textureSample = texture0.Sample(sampler0, uv);
    }

    float4 out_col = input.col * textureSample;
    out_col = (out_col - histogramMinMax.xxxx) / (histogramMinMax.yyyy - histogramMinMax.xxxx);
    out_col = clamp(out_col, float4(0,0,0,0), float4(1,1,1,1));
    out_col *= mask;

    if (singleChannel)
    {
        float value = dot(out_col, float4(1.0f, 1.0f, 1.0f, 1.0f));
        out_col = float4(value, value, value, 1.0f);
    }
    else if (flags & ImGuiShaderFlag_HideA)
    {
        out_col.a = 1.0f;
    }

    if (flags & ImGuiShaderFlag_Checker)
    {
        out_col.a = clamp(out_col.a, 0.0f, 1.0f);
        return float4(lerp(float3(checker, checker, checker), out_col.rgb, out_col.a), 1.0f);
    }
    else
    {
        return out_col;
    }
}
)====="
;
