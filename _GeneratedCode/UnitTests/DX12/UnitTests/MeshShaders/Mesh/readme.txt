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

Mesh::Context *m_MeshContext = nullptr;

m_MeshContext = Mesh::CreateContext(device);

=====================================================================================================
4) Call OnNewFrame at the beginning of your frame.
=====================================================================================================

Mesh::OnNewFrame(FramesInFlight)

FramesInFlight is the number of buffered frames in your application, which the technique uses to
know when temporary resources are no longer used and can safely be released.

=====================================================================================================
5) Destroy each context at some point before application exit.
=====================================================================================================

Mesh::DestroyContext(m_MeshContext);

=====================================================================================================
6) Ensure that the technique has the right path to the root folder of the technique, to find
assets and shaders.
=====================================================================================================

// By default this is set to L"./", so only need to set it if that is not right
Mesh::Context::s_techniqueLocation = L"./"

=====================================================================================================
7) Call execute on each context 0 or more times per frame, giving inputs and using outputs.
=====================================================================================================

// You should fill out everything in this struct before calling Execute
m_Mesh.m_input.

Mesh::Execute(m_Mesh, m_device, m_commandList);

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

Mesh::Context::LogFn = &CustomLogFn;

you can do similar for perf markers (like for pix) by overriding these:

Mesh::Context::PerfEventBeginFn
Mesh::Context::PerfEventEndFn

=====================================================================================================
10) Optional: Hook up imgui for automatic UI of technique inputs
=====================================================================================================

if (m_Mesh && ImGui::CollapsingHeader("Mesh"))
    outline::MakeUI(m_Mesh);

=====================================================================================================
11) Optional: Hook up python
=====================================================================================================

PyImport_AppendInittab("Mesh", Mesh::CreateModule);

in the python module, you will need to "import Mesh", and when will be able to set user exposed
variables like:

Mesh.Set_<variable name>(<value>);

=====================================================================================================
12) Hook up Agility SDK
=====================================================================================================

The Agility SDK is needed for this technique, which is supplied in the AgilitySDK folder.

The agility SDK is Microsoft's solution to being able to update directx more quickly than the windows SDK.

To set it up, you need to add "AgilitySDK\include" to the include path, so it finds the "d3d12.h" in
there instead of in the usual windows SDK location.

You also need to add this to a single cpp file (such as where your main function exists):

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 616; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\AgilitySDK\\bin\\"; }
