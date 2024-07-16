# Gigi

## Build Status

[![pipeline status](https://gitlab.ea.com/seed/gigi/badges/main/pipeline.svg)](https://gitlab.ea.com/seed/gigi/commits/main)

[Download Latest Gigi Version](https://gitlab.ea.com/seed/gigi/-/jobs/artifacts/main/download?job=win64_build_release)

## Downloading and Building

Pulling this repo and then building gigi.sln is all that is needed to build Gigi Editor, Viewer and Compiler.

When pulling down the repo, make sure and also do this to get the objects from lfs:

git lfs fetch --all

## Support

Gigi development is driven by the needs of the users, so please let us know about any problems you encounter, or missing  functionality, to help us prioritize appropriately.

This is still fairly young software so is missing features, has rough edges, and you may hit bugs!

You can log issues in gitlab, or contact me (Alan Wolfe) by email, or on slack.

Slack: [#seed-graphics-gigi](https://electronic-arts.slack.com/archives/C043Y2FK2AF)

## Learning Gigi

A tutorial to introduce Gigi can be found at:
[Gigi Tutorial 1 - Making a blur technique](https://drive.google.com/file/d/1-Ci3ZL5LPBmWEiE8pY29Up0p_BvqFsop/view?usp=sharing)

The Techniques folder contains examples that can be followed, and the UnitTests folder within Techniques contains small examples of how to use each feature.

## Introduction

Gigi is a framework for rapidly prototyping, developing, and researching rendering techniques such as ambient occlusion algorithms, denoisers, or machine learning with synthetic data.  Gigi aims to address the pain points of modern graphics technique development and maintenance.

Gigi is made up of  three applications: The editor, the viewer, and the compiler.

### Gigi Editor

Rendering techniques are described in the Gigi Editor as a set of action and resource nodes, with ordering semi implicitly defined by the read and write access of the resources within the action nodes.

![](readme/editor.png)
*Fig. 1 - The editor with a ray tracing technique loaded. Orange blocks are action nodes. Blue blocks are resource nodes.*

### Gigi Viewer

The Gigi Viewer can be opened from within the editor, or standalone, executing the loaded Gigi rendering technique in real time. The viewer contains functionality to profile and debug the rendering technique, and supports hot reloading of shader and data files for fast iteration times.  The viewer can be controlled with python scripts, allowing automated gathering of data, such as renderings to compare vs ground truths, when making figures or graphs for research results.  You can also do a pix capture from the viewer with a single button click, to more deeply debug techniques when needed.

![](readme/viewer.png)
*Fig. 2 - The viewer with the same ray tracing technique loaded as in figure 1. Gigi variables are exposed in the UI, profiling and debugging functionality is present, and hot reloading of shaders, images and other data files is supported.*

The viewer contains a lot of functionality out of the scope of Gigi techniques themselves, such as loading models, having a built in camera, and providing mouse and keyboard state to shaders on demand.  The Techniques folder contains a variety of functionality, including a UnitTests folder which can be used as examples for doing various things.  Below is shown a playable minesweeper game, and a simple path tracer.

![](readme/minesweeper.png)
*Fig. 3 - A playable minesweeper game, running entirely on the GPU. Techniques/ShaderToy/DemofoxMinesweeper.gg.*

![](readme/pathtracer.png)
*Fig. 4 - A simple path tracer. Techniques/ShaderToy/DemofoxPathTracer.gg.*

### Gigi Compiler

The Gigi compiler is a command line utility which generates code for a rendering technique towards any of the supported target platforms. The code generated is well formatted, well commented, contains friendly variable names, and is free of foreign abstractions which can obfuscate code or cause performance issues.  In short, it makes code that is meant to be what a human would write, and should pass a code review.  It just happens that a machine wrote the code.

![](readme/code.png)
*Fig. 5 - A snippet of code generated for Frostbite from Techniques/boxblur.gg.*

### Project Status

The Gigi Editor and Viewer is ready for general use, for rapid prototyping and research, and has a support chat room on slack at [seed-graphics-gigi](https://electronic-arts.slack.com/archives/C043Y2FK2AF).

For bugs and feature requests: https://gitlab.ea.com/seed/gigi/-/issues

The compiler and code generation is not yet ready for general use, but development is under way for suppport of DX12, Halcyon, and Frostbite.  Other backends, and functionality will be added as needed and requested!

# Gigi User Guide

Please see the "UserDocumentation" folder for more detailed information.

## Important Concepts

**Supported Functionality**

Supported functionality currently includes rasterization (draw calls), compute shaders, the whole suite of ray tracing shaders (ray gen, miss, closest hit, any hit, intersection) and indirect dispatch.

You can read and write textures and buffers from shaders.  Texture types supported are Texture2D, Texture2DArray and Texture3D.  Both typed and structured buffers are supported, as well as raw byte buffer views.

**Nodes**

Nodes are either resource nodes or action nodes.  Action nodes perform actions such as running a compute shader or copying a resource.  A resource node defines a resource, whether that resource is given to the technique by the application (an input), or the resource is internal to the technique, or if it's a resource generated by the technique for use by the application (an output).

Nodes often need sizes defined, such as the size of a buffer or a texture, or the dispatch size of a compute shader.  These sizes can be defined using a "fixed function" equation where you have a source of the size (a resource's size, or a variable), and then can put it into an equation of constants like so:

(((source + preAdd) * multiply) / divide) + postAdd

This allows you to specify constant sizes, or relative sizes, and even allows for division with round up.  You can also do things like set texture formats of internal textures to be the same format as textures provided by the host application at runtime.

Action nodes have resource input and output pins.  When a resource is plugged into an input pin, the output pin is for the same resource and can be plugged into a different action node.

Read and write access of resources by actions defines how the DAG can be flattened into a specific order to submit to the GPU as a command list.  Resource transitions are generated for backends when relevant.

**Variables**

Variables are a concept to help get around the rigidity of a statically analyzable render graph.  Variables have a type, and a visibility.  They can be internal to the technique, they can be read/write visible to the host, or they can be user visible, which means they will show up in UI and scripts, on platforms that support that.

Variables can be used to size resources, be part of the condition for whether action nodes execute, and they can also be read in shaders through constant buffers.  They can also be read and written by custom C++ code.

Enums are also supported for integer typed variables, which give labels to values in generated UI, the C++ code, and also the generated shader code, making it easy and convinient to have meaningful symbols for values instead of magic numbers.

**Constant Buffers**

If you use variables in a shader file, it will automatically make a constant buffer for those variables and handle all the details.

For instance, putting "/\*$(Variable:myVariableName)\*/" in a shader is enough for Gigi to automatically create and maintain a constant buffer for that shader, and will do a string replacement to the field of the constant buffer that the variable lives in.

If a variable is marked as constant, the token string in the shader will be replaced by a literal value instead.

**Loading Textures**

Reading specific textures from a shader is also supported.

Putting "/\*$(Image:myImage.png:R8_Unorm:float)\*/" into a shader tells gigi that you want to read myImage.png in the shader. The generated code will handle all the details of that and replace that token with the SRV of the texture in the shader. In this case, it would make a "Texture2D\<float\>" that was of type R8_Unorm.

You can also load texture arrays like this "/\*$(Image2DArray:flipbook_%i.png:RGBA8_Unorm:float3)\*/" which would make a "Texture2DArray\<float3\>" of type RGBA8_Unorm, but only the RGB channels being readable by the shader.  Texture3D is also supported.

**Custom C++ Code**

Since graphics techniques are often more than just dispatching shaders and managing resource lifetimes, Gigi allows you to run custom C++ code within your technique.  The goal of the C++ code ultimately is to write to variables which are then used by the rest of the technique.

**Shaders**

Shaders go through a processing step and have strings replaced to have them work on the target platform.  Most notably, you don't list any resources in your shader, but Gigi will put them there for you instead.  It looks for the string "/\*$(ShaderResources)\*/" for that and any platform specific includes or other things needed.

**Frontend vs Backends**

The entry point of GigiCompileLib is the front end. It loads the render graph and does any common processing on it, including optimizations and flattening into command list order.  It then passes the result to the chosen backend to generate code for that target.

Gigi and the backends are free to interpret your DAG render graph however they see fit, so long as they don't break your assumptions about read/write ordering or similar. This may happen for optimization purposes (minimize resource transitions, or scratch memory usage), or for other reasons such as limitations or extra abilities of the backend.

# Contributors

Created by Alan Wolfe

Contributors:

Chris Lewin

Mohit Sethi

Martin Mittring

Richard Raquepo

Stephane Levesque

Tristan Calderbank

