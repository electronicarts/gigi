:: This file is part of the FidelityFX SDK.
:: 
:: Copyright (C) 2025 Advanced Micro Devices, Inc.
:: 
:: Permission is hereby granted, free of charge, to any person obtaining a copy
:: of this software and associated documentation files(the "Software"), to deal
:: in the Software without restriction, including without limitation the rights
:: to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
:: copies of the Software, and to permit persons to whom the Software is
:: furnished to do so, subject to the following conditions :
:: 
:: The above copyright notice and this permission notice shall be included in
:: all copies or substantial portions of the Software.
:: 
:: THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
:: IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
:: FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
:: AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
:: LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
:: OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
:: THE SOFTWARE.

set projectdir=%1
set outputdir=%2
set filename=%3

:: Remove all quotes because arguments can contain them
set projectdir=%projectdir:"=%
set outputdir=%outputdir:"=%
set filename=%filename:"=%

:: Set the absolute path to the FidelityFX directory to avoid issues with long relative path names
pushd %projectdir%..\..\
set sdkdir=%cd%
popd

:: Set additional variables with quotes to support paths with spaces
set outdir="%projectdir%%outputdir%ffx_sc_output"
set FFX_BASE_INCLUDE_ARGS=-I "%sdkdir%\..\api\internal\include\gpu" -I "%sdkdir%\..\framegeneration\fsr3\include\gpu"
set FFX_SC="%sdkdir%\tools\ffx_sc\bin\FidelityFX_SC.exe"
set FFX_API_BASE_ARGS=-embed-arguments -E CS -Wno-for-redefinition -Wno-ambig-lit-shift -DFFX_HLSL=1
set FFX_BASE_ARGS=-reflection -deps=gcc -DFFX_GPU=1 -DFFX_IMPLICIT_SHADER_REGISTER_BINDING_HLSL=0

set HLSL_WAVE64_ARGS=-DFFX_PREFER_WAVE64="[WaveSize(64)]" -DFFX_HLSL_SM=66 -T cs_6_6
set HLSL_WAVE32_ARGS=-DFFX_HLSL_SM=62 -T cs_6_2
set HLSL_16BIT_ARGS=-DFFX_HALF=1 -enable-16bit-types
set FFX_COMPILE_OPTION=-Zs

:: Frame interpolation shaders
set FI_API_BASE_ARGS=%FFX_API_BASE_ARGS% -DFFX_FRAMEINTERPOLATION_EMBED_ROOTSIG=0
set FI_BASE_ARGS=-DFFX_FRAMEINTERPOLATION_OPTION_UPSAMPLE_SAMPLERS_USE_DATA_HALF=0 -DFFX_FRAMEINTERPOLATION_OPTION_ACCUMULATE_SAMPLERS_USE_DATA_HALF=0 -DFFX_FRAMEINTERPOLATION_OPTION_REPROJECT_SAMPLERS_USE_DATA_HALF=1 -DFFX_FRAMEINTERPOLATION_OPTION_POSTPROCESSLOCKSTATUS_SAMPLERS_USE_DATA_HALF=0 -DFFX_FRAMEINTERPOLATION_OPTION_UPSAMPLE_USE_LANCZOS_TYPE=2 %FFX_BASE_ARGS%
set FI_PERMUTATION_ARGS=-DFFX_FRAMEINTERPOLATION_OPTION_LOW_RES_MOTION_VECTORS={0,1} -DFFX_FRAMEINTERPOLATION_OPTION_JITTER_MOTION_VECTORS={0,1} -DFFX_FRAMEINTERPOLATION_OPTION_INVERTED_DEPTH={0,1}
set FI_INCLUDE_ARGS=%FFX_BASE_INCLUDE_ARGS%
set FI_SC_ARGS=%FI_BASE_ARGS% %FI_API_BASE_ARGS% %FI_PERMUTATION_ARGS%

:: Compile all permutations
%FFX_SC% %FFX_COMPILE_OPTION% %FI_SC_ARGS% -name=%filename% -DFFX_HALF=0 %HLSL_WAVE32_ARGS% %FI_INCLUDE_ARGS% -output=%outdir% "%sdkdir%\..\framegeneration\fsr3\internal\shaders\%filename%.hlsl"
%FFX_SC% %FFX_COMPILE_OPTION% %FI_SC_ARGS% -name=%filename%_wave64 -DFFX_HALF=0 %HLSL_WAVE64_ARGS% %FI_INCLUDE_ARGS% -output=%outdir% "%sdkdir%\..\framegeneration\fsr3\internal\shaders\%filename%.hlsl"
%FFX_SC% %FFX_COMPILE_OPTION% %FI_SC_ARGS% -name=%filename%_16bit %HLSL_16BIT_ARGS% %HLSL_WAVE32_ARGS% %FI_INCLUDE_ARGS% -output=%outdir% "%sdkdir%\..\framegeneration\fsr3\internal\shaders\%filename%.hlsl"
%FFX_SC% %FFX_COMPILE_OPTION% %FI_SC_ARGS% -name=%filename%_wave64_16bit %HLSL_16BIT_ARGS% %HLSL_WAVE64_ARGS% %FI_INCLUDE_ARGS% -output=%outdir% "%sdkdir%\..\framegeneration\fsr3\internal\shaders\%filename%.hlsl"

exit /b 0
