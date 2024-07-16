///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "GraphViz.h"
#include "GigiAssert.h"
#include "GigiCompilerLib/Backends/Shared.h"
#include "Nodes/nodes.h"
#include <unordered_set>

#define TEST_FOR_DUPLICATE_ACTION_OUTPUT() false

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static void LaunchExe(const char* applicationName, const char* commandLine)
{
    /*
    // Run the command in a way that won't block, and will allow the program to exit without waiting.
    // Only run it if dot.exe is found though, to avoid the error message box.
    {
        DWORD size = SearchPathA(nullptr, applicationName, ".exe", 0, nullptr, nullptr);
        if (size > 0)
        {
            std::vector<char> buff(size);
            size = SearchPathA(nullptr, applicationName, ".exe", size, buff.data(), nullptr);
            buff[size] = 0;

            char command[1024];
            sprintf_s(command, "start /min %s %s", applicationName, commandLine);
            system(command);
        }
    }
    */

    // ghetto solution that blocks, and pops up a cmd window, but at least doesn't make a message box if it errors
    std::string command = std::string(applicationName) + " " + std::string(commandLine);
    system(command.c_str());

    // Uses SearchPathA() to find the path to the exe to use for CreateProcess.
    // Mysteriously doesn't generate output images though ):
    /*
    DWORD size = SearchPathA(nullptr, applicationName, ".exe", 0, nullptr, nullptr);
    std::vector<char> buff(size);
    size = SearchPathA(nullptr, applicationName, ".exe", size, buff.data(), nullptr);
    buff[size] = 0;

    // additional information
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    // set the size of the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    size = GetCurrentDirectoryA(0, nullptr);
    std::vector<char> currentDirectory(size);
    size = GetCurrentDirectoryA(size, currentDirectory.data());
    currentDirectory[size] = 0;

    // start the program up
    bool result = CreateProcessA(buff.data(),   // the path
        (LPSTR)commandLine,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        currentDirectory.data(),           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
    );

    if (!result)
    {
        DWORD error = GetLastError();
        HRESULT hr = HRESULT_FROM_WIN32(error);
        int ijkl = 0;
    }

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    */

    // This solution works well, except if dot.exe isn't found, it makes an error messagebox.
    /*
    // run the command in a way that won't block, and will allow the program to exit without waiting
    sprintf_s(command, "start /min dot -Tpng %s -o %s", outFileDot.c_str(), outFilePng.c_str());
    system(command);
    */

    // I keep getting file not found for some reason. I think it isn't searching the path??
    /*
    // additional information
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    // set the size of the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // start the program up
    bool result = CreateProcessA((LPSTR)applicationName,   // the path
        (LPSTR)commandLine,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
    );

    if (!result)
    {
        DWORD error = GetLastError();
        HRESULT hr = HRESULT_FROM_WIN32(error);
        int ijkl = 0;
    }

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    */
}

// This generic function can be overridden for specific node types
template <typename T>
static void WriteSpecificNodeInfo(RenderGraph& renderGraph, RenderGraphNode& variant, T& node, std::ostringstream& out)
{
}

static void WriteSpecificNodeInfo(RenderGraph& renderGraph, RenderGraphNode& variant, RenderGraphNode_Resource_Texture& node, std::ostringstream& out)
{
    if (node.loadFileName.empty())
        return;

    out << "\\n" << node.loadFileName;
}

static void WriteSpecificNodeInfo(RenderGraph& renderGraph, RenderGraphNode& variant, RenderGraphNode_ActionBase& node, std::ostringstream& out)
{
    WriteSpecificNodeInfo(renderGraph, variant, node.GetBaseType(), out);

    if (node.condition.comparison == ConditionComparison::Count)
        return;

    out << "\n            <tr><td bgcolor=\"yellow\">Condition: ";

    if (node.condition.alwaysFalse)
    {
        out << "false";
    }
    else
    {
        if (node.condition.variable1Index != -1)
            out << renderGraph.variables[node.condition.variable1Index].name;

        bool showRHS = true;
        switch (node.condition.comparison)
        {
            case ConditionComparison::IsFalse: showRHS = false; out << " is False"; break;
            case ConditionComparison::IsTrue: showRHS = false; out << " is True"; break;
            case ConditionComparison::Equals: showRHS = true; out << " == "; break;
            case ConditionComparison::NotEquals: showRHS = true; out << " != "; break;
            case ConditionComparison::LT: showRHS = true; out << " &lt; "; break;
            case ConditionComparison::LTE: showRHS = true; out << " &lt;= "; break;
            case ConditionComparison::GT: showRHS = true; out << " &gt; "; break;
            case ConditionComparison::GTE: showRHS = true; out << " &gt;= "; break;
        }

        if (showRHS)
        {
            if (node.condition.variable2Index != -1)
                out << renderGraph.variables[node.condition.variable2Index].name;
            else
                out << node.condition.value2;
        }
    }

    out << "</td></tr>";
}

static void WriteSpecificNodeInfo(RenderGraph& renderGraph, RenderGraphNode& variant, RenderGraphNode_Action_ComputeShader& node, std::ostringstream& out)
{
    WriteSpecificNodeInfo(renderGraph, variant, node.GetBaseType(), out);

    out <<
        "\n            <tr><td bgcolor=\"thistle\">" << node.shader.shader->fileName << " : " << (node.entryPoint.empty() ? node.shader.shader->entryPoint : node.entryPoint) << "()</td></tr>"
        "\n            <tr><td bgcolor=\"thistle\">Dispatch: ";

    if (node.dispatchSize.indirectBuffer.nodeIndex != -1)
    {
        out << "&lt;Indirect&gt;";
    }
    else
    {
        if (node.dispatchSize.preAdd[0] != 0 || node.dispatchSize.preAdd[1] != 0 || node.dispatchSize.preAdd[2] != 0)
            out << "(";

        if (node.dispatchSize.variable.variableIndex != -1)
            out << renderGraph.variables[node.dispatchSize.variable.variableIndex].name;
        else if (node.dispatchSize.node.nodeIndex != -1)
            out << GetNodeName(renderGraph, node.dispatchSize.node.nodeIndex) << ".size";
        else
            out << "(1,1,1)";

        if (node.dispatchSize.preAdd[0] != 0 || node.dispatchSize.preAdd[1] != 0 || node.dispatchSize.preAdd[2] != 0)
            out << " + (" << node.dispatchSize.preAdd[0] << ", " << node.dispatchSize.preAdd[1] << ", " << node.dispatchSize.preAdd[2] << "))";

        if (node.dispatchSize.multiply[0] != 1 || node.dispatchSize.multiply[1] != 1 || node.dispatchSize.multiply[2] != 1)
            out << " * (" << node.dispatchSize.multiply[0] << ", " << node.dispatchSize.multiply[1] << ", " << node.dispatchSize.multiply[2] << ")";

        if (node.dispatchSize.divide[0] != 1 || node.dispatchSize.divide[1] != 1 || node.dispatchSize.divide[2] != 1)
            out << " / (" << node.dispatchSize.divide[0] << ", " << node.dispatchSize.divide[1] << ", " << node.dispatchSize.divide[2] << ")";

        if (node.dispatchSize.postAdd[0] != 0 || node.dispatchSize.postAdd[1] != 0 || node.dispatchSize.postAdd[2] != 0)
            out << " + (" << node.dispatchSize.postAdd[0] << ", " << node.dispatchSize.postAdd[1] << ", " << node.dispatchSize.postAdd[2] << ")";
    }

    out << "</td></tr>";
}

static void WriteSpecificNodeInfo(RenderGraph& renderGraph, RenderGraphNode& variant, RenderGraphNode_Action_RayShader& node, std::ostringstream& out)
{
    WriteSpecificNodeInfo(renderGraph, variant, node.GetBaseType(), out);

    out <<
        "\n            <tr><td bgcolor=\"thistle\">" << node.shader.shader->fileName << " : " << (node.entryPoint.empty() ? node.shader.shader->entryPoint : node.entryPoint) << "()</td></tr>"
        "\n            <tr><td bgcolor=\"thistle\">Dispatch: ";

    if (node.dispatchSize.preAdd[0] != 0 || node.dispatchSize.preAdd[1] != 0 || node.dispatchSize.preAdd[2] != 0)
        out << "(";

    if (node.dispatchSize.variable.variableIndex != -1)
        out << renderGraph.variables[node.dispatchSize.variable.variableIndex].name;
    else if (node.dispatchSize.node.nodeIndex != -1)
        out << GetNodeName(renderGraph, node.dispatchSize.node.nodeIndex) << ".size";
    else
        out << "(1,1,1)";

    if (node.dispatchSize.preAdd[0] != 0 || node.dispatchSize.preAdd[1] != 0 || node.dispatchSize.preAdd[2] != 0)
        out << " + (" << node.dispatchSize.preAdd[0] << ", " << node.dispatchSize.preAdd[1] << ", " << node.dispatchSize.preAdd[2] << "))";

    if (node.dispatchSize.multiply[0] != 1 || node.dispatchSize.multiply[1] != 1 || node.dispatchSize.multiply[2] != 1)
        out << " * (" << node.dispatchSize.multiply[0] << ", " << node.dispatchSize.multiply[1] << ", " << node.dispatchSize.multiply[2] << ")";

    if (node.dispatchSize.divide[0] != 1 || node.dispatchSize.divide[1] != 1 || node.dispatchSize.divide[2] != 1)
        out << " / (" << node.dispatchSize.divide[0] << ", " << node.dispatchSize.divide[1] << ", " << node.dispatchSize.divide[2] << ")";

    if (node.dispatchSize.postAdd[0] != 0 || node.dispatchSize.postAdd[1] != 0 || node.dispatchSize.postAdd[2] != 0)
        out << " + (" << node.dispatchSize.postAdd[0] << ", " << node.dispatchSize.postAdd[1] << ", " << node.dispatchSize.postAdd[2] << ")";

    out << "</td></tr>";
}

template <typename T>
static void WriteNodeInfo(RenderGraph& renderGraph, RenderGraphNode& variant, T& node_, std::ostringstream& out, bool resourcesInline, const char* nodeIndexSuffix)
{
    RenderGraphNode_Base& node = node_; // this just helps auto complete

    if (GetNodeIsResourceNode(variant))
    {
        std::string color = "";
        switch (GetNodeResourceVisibility(variant))
        {
            case ResourceVisibility::Imported: color = ", color = green"; break;
            case ResourceVisibility::Internal: break;
            case ResourceVisibility::Exported: color = ", color = red"; break;
        }

        out << "\n    Node" << node.nodeIndex << nodeIndexSuffix << " [label=\"" << node.name << "(" << GetNodeTypeString(variant) << ")";

        WriteSpecificNodeInfo(renderGraph, variant, node_, out);

        out << "\", shape=ellipse" << color << "];";
        return;
    }

    out <<
        "\n    Node" << node.nodeIndex << nodeIndexSuffix << " [shape=none, margin=0, label=<"
        "\n        <table border=\"0\" cellborder=\"1\" cellspacing=\"0\" cellpadding=\"4\">"
        "\n            <tr><td bgcolor=\"lightyellow\" port=\"-1\">" << node.name << " (" << GetNodeTypeString(variant) << ")</td></tr>"
        ;

    WriteSpecificNodeInfo(renderGraph, variant, node_, out);

    int pinCount = GetNodePinCount(variant);
    for (int pinIndex = 0; pinIndex < pinCount; ++pinIndex)
    {
        InputNodeInfo inputInfo = GetNodePinInputNodeInfo(variant, pinIndex);
        if (inputInfo.nodeIndex == -1)
            continue;

        RenderGraphNode& destVariant = renderGraph.nodes[inputInfo.nodeIndex];

        out << "\n            <tr><td bgcolor=\"lightslategray\" port=\"" << pinIndex << "\">";
        out << GetNodePinName(variant, pinIndex) << "(" << EnumToString(inputInfo.access) << ")";

        if (resourcesInline)
        {
            int resourceNodeIndex = GetResourceNodeForPin(renderGraph, variant, pinIndex);
            RenderGraphNode& resourceNode = renderGraph.nodes[resourceNodeIndex];
            out << " : " << GetNodeName(resourceNode) << "(" << GetNodeTypeString(resourceNode) << ")";
        }

        out << "</td></tr>";
    }

    out << "\n        </table>>];";
}

// returns true if it found any variables to write
bool WriteVariableInfo(const RenderGraph& renderGraph, std::ostringstream& out, bool includeInternal)
{
    std::ostringstream varsOut[(int)VariableVisibility::Count];

    bool printedAVariable = false;
    for (const Variable& variable : renderGraph.variables)
    {
        if (!includeInternal && variable.visibility == VariableVisibility::Internal)
            continue;
        printedAVariable = true;

        std::ostringstream& vout = varsOut[(int)variable.visibility];

        if (vout.str().empty())
            vout << "\n            <tr><td bgcolor=\"lightyellow\" port=\"-1\">" << EnumToString(variable.visibility) << " Variables</td></tr>";

        vout <<
            "\n            <tr><td bgcolor=\"lightslategray\" port=\"-1\">" <<
            (variable.Const ? "const " : " ") <<
            EnumToString(variable.type) << " " <<
            variable.name;

        if (!variable.dflt.empty())
            vout << " = " << variable.dflt;
        else
            vout << " = {}";

        vout << ";";

        //if (!variable.comment.empty())
            //vout << " // " << variable.comment;

        vout <<
            "</td></tr>";
    }

    for (int i = 0; i < (int)VariableVisibility::Count; ++i)
        out << varsOut[i].str();

    return printedAVariable;
}

void MakeRenderGraphGraphViz(RenderGraph& renderGraph, const char* outFolder)
{
    // write the header
    std::ostringstream out;
    out <<
        "// This is a graphviz file"
        "\ndigraph G {"
        "\n    rankdir = LR;"
        "\n"
        ;

    // write out info for each node
    for (RenderGraphNode& node : renderGraph.nodes)
        ExecuteOnNode(node, [&](auto& node_) { WriteNodeInfo(renderGraph, node, node_, out, false, ""); });

    struct Link
    {
        int pinIndex;
        int otherNodeIndex;
    };

    // write the links between nodes
    std::vector<std::vector<Link>> links(renderGraph.nodes.size());
    out << "\n";
    for (RenderGraphNode& node : renderGraph.nodes)
    {
        int pinCount = GetNodePinCount(node);
        for (int pinIndex = 0; pinIndex < pinCount; ++pinIndex)
        {
            InputNodeInfo inputInfo = GetNodePinInputNodeInfo(node, pinIndex);
            if (inputInfo.nodeIndex == -1)
                continue;

            RenderGraphNode& destVariant = renderGraph.nodes[inputInfo.nodeIndex];

            int otherPinIndex = -1;
            if (GetNodeIsResourceNode(destVariant))
            {
                out << "\n    Node" << GetNodeIndex(destVariant) << " -> Node" << GetNodeIndex(node) << ":" << pinIndex;
            }
            else
            {
                otherPinIndex = GetNodeIndex(node);
                out << "\n    Node" << GetNodeIndex(destVariant) << ":" << inputInfo.pinIndex << " -> Node" << GetNodeIndex(node) << ":" << pinIndex;
            }

            Link newLink;
            newLink.pinIndex = inputInfo.pinIndex;
            newLink.otherNodeIndex = otherPinIndex;
            links[GetNodeIndex(destVariant)].push_back(newLink);
        }
    }

#if TEST_FOR_DUPLICATE_ACTION_OUTPUT()
    for (size_t nodeIndex = 0; nodeIndex < renderGraph.nodes.size(); ++nodeIndex)
    {
        // Ignore 
        if (GetNodeIsResourceNode(renderGraph.nodes[nodeIndex]))
            continue;

        std::unordered_set<int> pinsUsed;
        for (auto link : links[nodeIndex])
        {
            if (pinsUsed.count(link.pinIndex) > 0)
            {
                Assert(false, "Action node %s pin %i output goes to multiple places. This is not necesarily an error, but is often not intended", GetNodeName(renderGraph.nodes[nodeIndex]).c_str(), link.pinIndex);
            }
            pinsUsed.insert(link.pinIndex);
        }
    }
#endif

    // write out the variables and constants
    {
        std::ostringstream varsOut;
        if (WriteVariableInfo(renderGraph, varsOut, true))
        {
            out <<
                "\n    VariableNode [shape=none, margin=0, label=<"
                "\n        <table border=\"0\" cellborder=\"1\" cellspacing=\"0\" cellpadding=\"4\">" <<
                varsOut.str() <<
                "\n        </table>>];";
        }
    }

    // write the footer
    out <<
        "\n}"
        ;

    // make the output file names
    std::string outFileDot = std::string(outFolder) + renderGraph.name + ".dot";
    std::string outFilePng = std::string(outFolder) + renderGraph.name + ".png";

    // write the file if it's changed
    WriteFileIfDifferent(outFileDot, out.str());

    // make the rendered png
    char command[1024];
    sprintf_s(command, "-Tpng %s -o %s", outFileDot.c_str(), outFilePng.c_str());
    LaunchExe("dot", command);
}

void MakeFlattenedRenderGraphGraphViz(RenderGraph& renderGraph, const char* outFolder)
{
    // write the header
    std::ostringstream out;
    out <<
        "// This is a graphviz file"
        "\ndigraph G {"
        "\n    rankdir = LR;"
        "\n"
        ;

    // write out info for each node
    for (RenderGraphNode& node : renderGraph.nodes)
    {
        if (GetNodeIsResourceNode(node))
            continue;
        ExecuteOnNode(node, [&](auto& node_) { WriteNodeInfo(renderGraph, node, node_, out, true, ""); });
    }

    // write out the variables and constants
    {
        std::ostringstream varsOut;
        if (WriteVariableInfo(renderGraph, varsOut, true))
        {
            out <<
                "\n    VariableNode [shape=none, margin=0, label=<"
                "\n        <table border=\"0\" cellborder=\"1\" cellspacing=\"0\" cellpadding=\"4\">" <<
                varsOut.str() <<
                "\n        </table>>];";
        }
    }

    // write the links between nodes
    out << "\n";
    int lastNodeIndex = -1;
    for (int nodeIndex :renderGraph.flattenedNodeList)
    {
        RenderGraphNode& node = renderGraph.nodes[nodeIndex];
        if (GetNodeIsResourceNode(node))
            continue;     

        /*
        int pinCount = GetNodePinCount(node);
        for (int pinIndex = 0; pinIndex < pinCount; ++pinIndex)
        {
            int resourceNodeIndex = GetResourceNodeForPin(renderGraph, node, pinIndex);
            RenderGraphNode& resourceNode = renderGraph.nodes[resourceNodeIndex];

            char nodeIndexSuffix[1024];
            sprintf_s(nodeIndexSuffix, "_%i_%i", nodeIndex, pinIndex);

            ExecuteOnNode(resourceNode, [&](auto& resourceNode_) { WriteNodeInfo(renderGraph, resourceNode, resourceNode_, out, nodeIndexSuffix); });
            out << "\n    Node" << resourceNodeIndex << "_" << nodeIndex << "_" << pinIndex << " ->Node" << nodeIndex << " : " << pinIndex;
        }
        */

        if (lastNodeIndex != -1)
            out << "\n    Node" << lastNodeIndex << ":-1 -> Node" << nodeIndex << ":-1";
        else
            out << "\n    VariableNode:-1 -> Node" << nodeIndex << ":-1";

        lastNodeIndex = nodeIndex;
    }

    // write the footer
    out <<
        "\n}"
        ;

    // make the output file names
    std::string outFileDot = std::string(outFolder) + renderGraph.name + ".flat.dot";
    std::string outFilePng = std::string(outFolder) + renderGraph.name + ".flat.png";

    // write the file if it's changed
    WriteFileIfDifferent(outFileDot, out.str());

    // make the rendered png
    char command[1024];
    sprintf_s(command, "-Tpng %s -o %s", outFileDot.c_str(), outFilePng.c_str());
    LaunchExe("dot", command);
}

void MakeSummaryRenderGraphGraphViz(RenderGraph& renderGraph, const char* outFolder)
{
    // write the header
    std::ostringstream out;
    out <<
        "// This is a graphviz file"
        "\ndigraph G {"
        "\n    rankdir = LR;"
        "\n"
        ;

    // write out the main node header
    out <<
        "\n    MainNode [shape=none, margin=0, label=<"
        "\n        <table border=\"0\" cellborder=\"1\" cellspacing=\"0\" cellpadding=\"4\">"
        "\n            <tr><td bgcolor=\"lightyellow\" port=\"-1\">" << renderGraph.name << "</td></tr>"
        ;

    // write out the main node pins
    int pinIndex = 0;
    for (RenderGraphNode& node : renderGraph.nodes)
    {
        if (!GetNodeIsResourceNode(node))
            continue;

        ResourceVisibility visibility = GetNodeResourceVisibility(node);
        if (visibility == ResourceVisibility::Internal)
            continue;

        out << "\n            <tr><td bgcolor=\"lightslategray\" port=\"" << pinIndex << "\">" << GetNodeName(node) << "</td></tr>";
        pinIndex++;
    }

    // write out the exposed variables
    {
        std::ostringstream varsOut;
        if (WriteVariableInfo(renderGraph, varsOut, false))
        {
            out <<
                varsOut.str() <<
                "\n        </table>>];";
        }
        else
        {
            out <<
                "\n        </table>>];";
        }
    }

    // write out info for each node
    pinIndex = 0;
    for (RenderGraphNode& node : renderGraph.nodes)
    {
        if (!GetNodeIsResourceNode(node))
            continue;

        ResourceVisibility visibility = GetNodeResourceVisibility(node);
        if (visibility == ResourceVisibility::Internal)
            continue;

        ExecuteOnNode(node,
            [&](auto& node_)
            {
                WriteNodeInfo(renderGraph, node, node_, out, false, "");
            }
        );

        switch (visibility)
        {
            case ResourceVisibility::Imported:
            {
                out << "\n    Node" << GetNodeIndex(node) << " -> MainNode:" << pinIndex;
                break;
            }
            case ResourceVisibility::Exported:
            {
                out << "\n    MainNode:" << pinIndex << " ->  Node" << GetNodeIndex(node);
                break;
            }
        }
        pinIndex++;
    }

    // write the footer
    out <<
        "\n}"
        ;

    // make the output file names
    std::string outFileDot = std::string(outFolder) + renderGraph.name + ".summary.dot";
    std::string outFilePng = std::string(outFolder) + renderGraph.name + ".summary.png";

    // write the file if it's changed
    WriteFileIfDifferent(outFileDot, out.str());

    // make the rendered png
    char command[1024];
    sprintf_s(command, "-Tpng %s -o %s", outFileDot.c_str(), outFilePng.c_str());
    LaunchExe("dot", command);
}
