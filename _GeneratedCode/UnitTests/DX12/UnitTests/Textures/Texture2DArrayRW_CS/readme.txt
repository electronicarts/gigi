Here are the instructions for how to use this package in a dx12 application

=====================================================================================================
1) Add the code files to your project: private, public, DX12Utils
=====================================================================================================

=====================================================================================================
2) Include the public/technique.h file where you want to interact with the technique from
as well as the public/imgui.h and public/pythoninterface.h file if you want that functionality.
=====================================================================================================

=====================================================================================================
3) Create 0 or more contexts at initialization or other times. Member variables are a good place to
store them.
=====================================================================================================

Texture2DArrayRW_CS::Context *m_Texture2DArrayRW_CSContext = nullptr;

m_Texture2DArrayRW_CSContext = Texture2DArrayRW_CS::CreateContext(device);

=====================================================================================================
4) Call OnNewFrame at the beginning of your frame.
=====================================================================================================

Texture2DArrayRW_CS::OnNewFrame(FramesInFlight)

FramesInFlight is the number of buffered frames in your application, which the technique uses to
know when temporary resources are no longer used and can safely be released.

=====================================================================================================
5) Destroy each context at some point before application exit.
=====================================================================================================

Texture2DArrayRW_CS::DestroyContext(m_Texture2DArrayRW_CSContext);

=====================================================================================================
6) Ensure that the technique has the right path to the root folder of the technique, to find
assets and shaders.
=====================================================================================================

// By default this is set to L"./", so only need to set it if that is not right
Texture2DArrayRW_CS::Context::s_techniqueLocation = L"./"

=====================================================================================================
7) Call execute on each context 0 or more times per frame, giving inputs and using outputs.
=====================================================================================================

// You should fill out everything in this struct before calling Execute
m_Texture2DArrayRW_CS.m_input.

Texture2DArrayRW_CS::Execute(m_Texture2DArrayRW_CS, m_device, m_commandList);

=====================================================================================================
8) Handle Assets
=====================================================================================================

The assets folder contains any assets the technique needs, such as textures.

Those need to be loaded and provided to the technique via the context input structure as appropriate.

Resources loaded by the technique (such as those declared in shaders) will happen within the generated
technique code and do not need to be loaded manually.

=====================================================================================================
9) Optional: Hook up a custom log function to get notifications of internal errors and warnings
=====================================================================================================

void CustomLogFn(LogLevel level, const char* msg, ...)
{
}

Texture2DArrayRW_CS::Context::LogFn = &CustomLogFn;

you can do similar for perf markers (like for pix) by overriding these:

Texture2DArrayRW_CS::Context::PerfEventBeginFn
Texture2DArrayRW_CS::Context::PerfEventEndFn

=====================================================================================================
10) Optional: Hook up imgui for automatic UI of technique inputs
=====================================================================================================

if (m_Texture2DArrayRW_CS && ImGui::CollapsingHeader("Texture2DArrayRW_CS"))
    outline::MakeUI(m_Texture2DArrayRW_CS);

=====================================================================================================
11) Optional: Hook up python
=====================================================================================================

PyImport_AppendInittab("Texture2DArrayRW_CS", Texture2DArrayRW_CS::CreateModule);

in the python module, you will need to "import Texture2DArrayRW_CS", and when will be able to set user exposed
variables like:

Texture2DArrayRW_CS.Set_<variable name>(<value>);
