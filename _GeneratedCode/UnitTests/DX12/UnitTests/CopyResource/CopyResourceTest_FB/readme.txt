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

CopyResourceTest_FB::Context *m_CopyResourceTest_FBContext = nullptr;

m_CopyResourceTest_FBContext = CopyResourceTest_FB::CreateContext(device);

=====================================================================================================
4) Call OnNewFrame at the beginning of your frame.
=====================================================================================================

CopyResourceTest_FB::OnNewFrame(FramesInFlight)

FramesInFlight is the number of buffered frames in your application, which the technique uses to
know when temporary resources are no longer used and can safely be released.

=====================================================================================================
5) Destroy each context at some point before application exit.
=====================================================================================================

CopyResourceTest_FB::DestroyContext(m_CopyResourceTest_FBContext);

=====================================================================================================
6) Ensure that the technique has the right path to the root folder of the technique, to find
assets and shaders.
=====================================================================================================

// By default this is set to L"./", so only need to set it if that is not right
CopyResourceTest_FB::Context::s_techniqueLocation = L"./"

=====================================================================================================
7) Call execute on each context 0 or more times per frame, giving inputs and using outputs.
=====================================================================================================

// You should fill out everything in this struct before calling Execute
m_CopyResourceTest_FB.m_input.

CopyResourceTest_FB::Execute(m_CopyResourceTest_FB, m_device, m_commandList);

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

CopyResourceTest_FB::Context::LogFn = &CustomLogFn;

you can do similar for perf markers (like for pix) by overriding these:

CopyResourceTest_FB::Context::PerfEventBeginFn
CopyResourceTest_FB::Context::PerfEventEndFn

=====================================================================================================
10) Optional: Hook up imgui for automatic UI of technique inputs
=====================================================================================================

if (m_CopyResourceTest_FB && ImGui::CollapsingHeader("CopyResourceTest_FB"))
    outline::MakeUI(m_CopyResourceTest_FB);

=====================================================================================================
11) Optional: Hook up python
=====================================================================================================

PyImport_AppendInittab("CopyResourceTest_FB", CopyResourceTest_FB::CreateModule);

in the python module, you will need to "import CopyResourceTest_FB", and when will be able to set user exposed
variables like:

CopyResourceTest_FB.Set_<variable name>(<value>);
