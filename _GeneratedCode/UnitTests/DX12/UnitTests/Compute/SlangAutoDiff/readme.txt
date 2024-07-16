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

SlangAutoDiff::Context *m_SlangAutoDiffContext = nullptr;

m_SlangAutoDiffContext = SlangAutoDiff::CreateContext(device);

=====================================================================================================
4) Call OnNewFrame at the beginning of your frame.
=====================================================================================================

SlangAutoDiff::OnNewFrame(FramesInFlight)

FramesInFlight is the number of buffered frames in your application, which the technique uses to
know when temporary resources are no longer used and can safely be released.

=====================================================================================================
5) Destroy each context at some point before application exit.
=====================================================================================================

SlangAutoDiff::DestroyContext(m_SlangAutoDiffContext);

=====================================================================================================
6) Ensure that the technique has the right path to the root folder of the technique, to find
assets and shaders.
=====================================================================================================

// By default this is set to L"./", so only need to set it if that is not right
SlangAutoDiff::Context::s_techniqueLocation = L"./"

=====================================================================================================
7) Call execute on each context 0 or more times per frame, giving inputs and using outputs.
=====================================================================================================

// You should fill out everything in this struct before calling Execute
m_SlangAutoDiff.m_input.

SlangAutoDiff::Execute(m_SlangAutoDiff, m_device, m_commandList);

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

SlangAutoDiff::Context::LogFn = &CustomLogFn;

you can do similar for perf markers (like for pix) by overriding these:

SlangAutoDiff::Context::PerfEventBeginFn
SlangAutoDiff::Context::PerfEventEndFn

=====================================================================================================
10) Optional: Hook up imgui for automatic UI of technique inputs
=====================================================================================================

if (m_SlangAutoDiff && ImGui::CollapsingHeader("SlangAutoDiff"))
    outline::MakeUI(m_SlangAutoDiff);

=====================================================================================================
11) Optional: Hook up python
=====================================================================================================

PyImport_AppendInittab("SlangAutoDiff", SlangAutoDiff::CreateModule);

in the python module, you will need to "import SlangAutoDiff", and when will be able to set user exposed
variables like:

SlangAutoDiff.Set_<variable name>(<value>);
