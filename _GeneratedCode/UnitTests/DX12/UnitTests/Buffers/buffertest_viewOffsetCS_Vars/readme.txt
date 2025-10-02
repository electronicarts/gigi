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

buffertest_viewOffsetCS_Vars::Context *m_buffertest_viewOffsetCS_VarsContext = nullptr;

m_buffertest_viewOffsetCS_VarsContext = buffertest_viewOffsetCS_Vars::CreateContext(device);

=====================================================================================================
4) Call OnNewFrame at the beginning of your frame.
=====================================================================================================

buffertest_viewOffsetCS_Vars::OnNewFrame(FramesInFlight)

FramesInFlight is the number of buffered frames in your application, which the technique uses to
know when temporary resources are no longer used and can safely be released.

=====================================================================================================
5) Destroy each context at some point before application exit.
=====================================================================================================

buffertest_viewOffsetCS_Vars::DestroyContext(m_buffertest_viewOffsetCS_VarsContext);

=====================================================================================================
6) Ensure that the technique has the right path to the root folder of the technique, to find
assets and shaders.
=====================================================================================================

// By default this is set to L"./", so only need to set it if that is not right
buffertest_viewOffsetCS_Vars::Context::s_techniqueLocation = L"./"

=====================================================================================================
7) Call execute on each context 0 or more times per frame, giving inputs and using outputs.
=====================================================================================================

// You should fill out everything in this struct before calling Execute
m_buffertest_viewOffsetCS_Vars.m_input.

buffertest_viewOffsetCS_Vars::Execute(m_buffertest_viewOffsetCS_Vars, m_device, m_commandList);

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

buffertest_viewOffsetCS_Vars::Context::LogFn = &CustomLogFn;

you can do similar for perf markers (like for pix) by overriding these:

buffertest_viewOffsetCS_Vars::Context::PerfEventBeginFn
buffertest_viewOffsetCS_Vars::Context::PerfEventEndFn

=====================================================================================================
10) Optional: Hook up imgui for automatic UI of technique inputs
=====================================================================================================

if (m_buffertest_viewOffsetCS_Vars && ImGui::CollapsingHeader("buffertest_viewOffsetCS_Vars"))
    outline::MakeUI(m_buffertest_viewOffsetCS_Vars);

=====================================================================================================
11) Optional: Hook up python
=====================================================================================================

PyImport_AppendInittab("buffertest_viewOffsetCS_Vars", buffertest_viewOffsetCS_Vars::CreateModule);

in the python module, you will need to "import buffertest_viewOffsetCS_Vars", and when will be able to set user exposed
variables like:

buffertest_viewOffsetCS_Vars.Set_<variable name>(<value>);
