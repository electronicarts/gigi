# Gigi Overview

Gigi aims to help graphics practitioners work more closely to the speed of thought and is meant for use by professionals, researchers, students, and hobbyists.

Gigi is made of four components:

1. Editor - Author a rendering technique
2. Viewer - Debug and profile the technique
3. Compiler - Generate code for the technique like a human would write
4. Browser - View a library of Gigi techniques, and submit your own

## Gigi Editor

The editor allows you to concisely but completely describe rendering techniques as a graph of nodes.

[![A screenshot of the Gigi Editor](editor.png)](editor.png)

Blue nodes are resource nodes and include:

* Textures - 2D, 2D Array, 3D, Cube, etc.
* Buffers - Structured, typed, raw.

Resources can be of various types and sizes, and can be marked as transient or persistent across frames.  Resources also have a visibility setting:

* **Imported** - The resource will be provided from outside of the technique, by the host application.  The technique does not own this resource or manage its lifetime.
* **Internal** - The resource is internal to the technique.  The technique owns the resource and manages its lifetime.  The host does not have access to read or write this resource
* **Exported** - The resource is created by the technique, which manages its lifetime, but the resource is exported to the host application after execution, allowing it to be read or written.

Orange nodes are action nodes and include:

* Dispatch Compute Shader
* Dispatch Ray Gen Shader
* Draw Call (rasterize)
* Copy Resource
* Sub Graph (call into another Gigi technique)

Shader source code must be provided, but there are special tokens you can put into the shader file for Gigi specific functionality.

One notable piece of functionality available is reading textures from disk:

    float3 color = /*$(Image2D:cabinsmall.png:RGBA8_Unorm_sRGB:float4:true)*/[px].rgb;

When that shader file is processed by Gigi, it will see that the shader wants to read "cabinsmall.png", stored in a texture format RGBA8_Unorm_sRGB, read in the shader as a float4 texture, and the boolean parameter on the end says whether or not the source file should be treated as sRGB.  Gigi will handle loading that texture into memory, creating a texture resource to hold it, passing it to the shader as an SRV, and then doing string replacement to the shader before it gets to the shader compiler. This happens in the viewer, as well as in the generated code packages, but the details of the mechanisms and shader replacement will vary from platform to platform.  Here is how the processed shader looks in the DX12 viewer:

    // Inserted at the top of the shader
    Texture2D<float4> _loadedTexture_0 : register(t0);
    
    ...
    
    // String replacement where the texture read occurs
    float3 color = _loadedTexture_0[px].rgb;

Another useful feature is that you can define Gigi variables in the editor and then read them from shaders.  Gigi variables are typed (int, uint, float, float4), they are given default values, and they are given a visibility scope. Variables can be used to control the size of resources, the size of shader dispatches, and a handful of other things.  Per frame logic can also be executed to conditionally modify variables each frame.  Variables can also be read by shaders using special shader tokens:

    ret.position = mul(float4(input.position, 1.0f), /*$(Variable:ViewProjMtx)*/);

When Gigi processes the shader file, it will make note of which variables want to be accessed by a shader, it will make a constant buffer for those values, send it through to the shader, and will do string replacement before the shader gets to the shader compiler.

    // Inserted at the top of the shader
    struct Struct__Draw1VSCB
    {
        float4x4 ViewProjMtx;
    };
    ConstantBuffer<Struct__Draw1VSCB> _Draw1VSCB : register(b0);
    
    ...
    
    // String replacement where the variable read occurs
    ret.position = mul(float4(input.position, 1.0f), _Draw1VSCB.ViewProjMtx);

User visible Gigi variables can be thought of as user settings, and will become UI, console variables, or exposed to scripts, depending on the target platform.  Host visible variables are meant to be things that the technique needs but should not be exposed to the user. A good example of this is a camera's view matrix.  Lastly are internal variables, which cannot be accessed from outside the generated code.  These are useful for things like tracking frame index, or calculating sizes of buffers that are based on the size of resources that are provided by the host.

Please see **UserDocumentation/GigiShaders_Documentation.docx** for more information about shaders in Gigi.

Subgraph nodes are a way of having "Data Defined Nodes" in Gigi.  When you create a subgraph node, you specify which .gg file you want to run.  Any imported resources of that technique become input/output pins on the node, and any exported resource become output pins.  Subgraph nodes allow you to override the visibility of variables within the subgraph node, and even allow you to replace a variable inside of a subgraph node with a variable from the parent graph.  Subgraphs also allow you to specify a loop count.  When a Gigi graph is processed for the viewer, or code generation, subgraphs currently have their loops unrolled and the nodes inlined into the parent graph to be able to be optimized holistically.

In short, subgraphs are a pain free way to re-use modular rendering functionality defined within Gigi.

For examples of how to do things, check out the **Techniques/** folder, especially the **Techniques/UnitTests/** folder which aims to exercise every possible piece of functionality.

## Gigi Viewer

The viewer allows you to view the described rendering technique as it runs in real time.

[![A screenshot of the Gigi Viewer](viewer.png)](viewer.png)

You can modify variables, specify imported resources, you can view the CPU and GPU time of each node in the profiler window and you can even pause and single step the technique.

While all of this is going on, you can view each resource at each step in the render technique to help diagnose problems - such as investigating which pass is generating mysterious black pixels (NaNs!) in the final result.

The viewer supports hot reloading of the .gg file being viewed, shader files, asset files, and any other file that is read by your technique, which allows rapid iteration.

A variety of formats are supported for loading data into textures and buffers, including standard image formats, binary files, common mesh formats, and csv.  The viewer also allows you to save resources out as a variety of formats.

When more intensive debugging of a technique is needed, a single button click takes a pix capture and opens pix.  Gigi uses perf markers for each node, using the names you gave the nodes, so navigating the pix capture is very easy and convenient.

Another important feature of the Viewer is that it can be scripted by python scripts. The option to run a python script is under the file menu.  This is used to run the viewer unit tests (**Techniques/UnitTests/RunTests.py**), but has also been used at EA / SEED for gathering data and making diagrams for published research, such as [Filter-Adapted Spatio-Temporal Sampling for Real-Time Rendering](https://www.ea.com/seed/news/spatio-temporal-sampling).  We've found this extremely useful in research work and importantly, Gigi is deterministic by default.  An example of how you'd break determinism, is by adding a variable for the frame time, and using that variable in your shaders.

Please see **UserDocumentation/GigiViewerDX12_Documentation.docx** for more information about the python interface, or have a look at the scripts in the unit tests folder.

Lastly, there is a "System variables" tab in the viewer where you can specify things like details of the camera.  You can also specify which variables in your Gigi technique should receive values that the viewer makes available.  As an example, many techniques want a view projection matrix for use in their rendering.  In the "System Variables" tab, you can specify which of your variables should receive the view projection matrix provided by the viewer's camera, and then the camera will function within your technique.

The systems variables also allows you to receive mouse and keyboard data to make interactive demos.  The viewer has a "Hide UI" option under the view menu which helps it behave as a standalone demo.  To make the viewer function as a standalone demo, you can run it with a python file on the command line, where the python file loads the .gg file, hides the UI, and sets up the demo so that it's ready for a user to interact with.  The instructions to the user would just be to run the batch file to start the demo.

## Compiler

The compiler generates code for your technique, towards whatever platform and language you've chosen. At the time of this writing, the public Gigi repo only supports generating code for C++ / DX12, but we intend to add more as time goes on.

It's very important to us that the compiler generates good code, like a human would write, and that it should pass a reasonable code review.  If not, we consider that a bug.

We also feel it's important not to add foreign abstractions to the code generated. We believe "Nobody wants your abstraction". When writing code generation in Gigi, we ask ourselves "how would a human write this, on this platform, in this language?" and have it generate code that does it the same way.  This is fully true when writing towards an engine, which already has an abstraction for resource lifetime management etc, but when writing towards a raw API, we've found that ***light weight*** abstraction helps make more readable, and maintainable code.

The generated code shouldn't just be readable generated code, it should be maintainable too, just as if it actually was written by a human.

Here is a snippet of C++ / DX12 code generated for the **Techniques/UnitTests/Compute/boxblur.gg** technique:

[![A screenshot of the Gigi Generated C++ / DX12 code](code.png)](code.png)

We have already released Gigi generated code to the wild! Perhaps the only give away that the code wasn't written by a human, is that it's very consistent.

1. The Demo of the "Machine Learning For Game Devs" blog posts and video were code generated by Gigi: https://github.com/electronicarts/cpp-ml-intro/
2. The command line utility that generates FAST noise was code generated by Gigi: https://github.com/electronicarts/fastnoise

The compiler works by loading up a .gg file and having a "front end" process it.  The front end is responsible for things like inlining subgraphs, and flattening the DAG into a serial list of work to send to a command list.  It then passes the flattened / processed rendering technique to the code generator for the chosen platform.

The compiler does perform some optimizations on the graph while flattening.  For instance, any "read only" resource access will search backwards in the graph and be dependent on the last write to a resource.  This prevents "false ordering constraints" caused by read only access.  Another optimization it does is reorder work to minimize resource transitions. Other optimizations are planned for the future, such as being able to de-duplicate transient resources and reduce memory used, or reducing resource copies via "register renaming".  These things would be needed as subgraphs are used heavily for modularity, and would help break up any overhead added by those abstractions.

Each backend code generator has code that is isolated from the other code generators.  This is important because if you are working on the Vulkan backend for instance, you don't want to have to worry if your work is going to break the Metal back end or not, and you may not even be able to easily verify that it hasn't.  Keeping the code generators completely separate from each other allows people to work on one platform without having to worry about other platforms.  This problem is the challenge faced by traditional HALs and is why I believe the abstraction level Gigi works at is more appropriate for cross platform development.  Gigi allows you to work with concepts that are familiar to the GPU (run CS, copy resource, etc) and its up to each backend to determine how to best interpret that for their platform.  This leaves open the possibility of a platform doing things like finding opportunities to use tiled memory / shader operations by combining nodes together.

Finally, calling the Gigi compiler a compiler makes it sound a lot fancier than it is.  In reality, the compiler is more of a string replacement engine. If you look at the code, you'll see what I mean.

## Browser

The browser allows you to view a library of Gigi techniques ranging from playable games, to implementations of research papers and modern rendering techniques, to image processing utilities.

You may also submit your own techniques for sharing in the browser: [Sharing Techniques In The Gigi Browser](BrowserSharing.md).

[![A screenshot of the Gigi Browser](browser.png)](browser.png)

## Conclusion

Gigi lets you turn your ideas into reality more quickly.  It removes the need for error prone boilerplate of raw APIs and long compile times of huge engines. When you are finished however, it allows you to deploy your work to those very same places to share with others, where the rubber actually hits the road.

By reducing the effort and time needed to do these things, our hope is that people will be able to learn more rapidly, find better results more quickly, and most importantly be able to more easily share work with fellow practitioners across the landscape of modern engines and APIs.
