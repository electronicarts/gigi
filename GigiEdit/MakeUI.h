///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// clang-format off
#include "external/df_serialize/_common.h"
#include "Schemas/Utils.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Shellapi.h"

#include <filesystem>
// clang-format on

// Context for when ShowUIOverride is being called
enum class ShowUIOverrideContext
{
    Field,  // for a field
    Type    // for a type (enum or struct)
};

inline void CaseInsensitiveSort(std::vector<std::string>& strs)
{
    std::sort(
        std::begin(strs),
        std::end(strs),
        [](const std::string& str1, const std::string& str2) {
            return lexicographical_compare(
                begin(str1), end(str1),
                begin(str2), end(str2),
                [](const char& char1, const char& char2) {
                    return tolower(char1) < tolower(char2);
                }
            );
        }
    );
}

enum class UIOverrideResult
{
    Finished,
    Continue
};

inline void ShowUIToolTip(const char* tooltip, bool sameline = true)
{
    if (!tooltip || !tooltip[0])
        return;

    /*
    if (sameline)
        ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
    ImGui::Text("[?]");
    ImGui::PopStyleColor();
    */
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", tooltip);
}

// Enums

#define ENUM_BEGIN(_NAME, _DESCRIPTION) \
    bool ShowUI(RenderGraph& renderGraph, const char* label, const char* tooltip, _NAME& value, TypePathEntry path) \
    { \
        typedef _NAME EnumType; \
        bool ret = false; \
        if (ShowUIOverrideBase(renderGraph, 0, ret, label, tooltip, value, path, ShowUIOverrideContext::Type) == UIOverrideResult::Finished) \
            return ret; \
        std::string labelNameOuter = EnumToString(value); \
        if (labelNameOuter == "Count") \
            labelNameOuter = "<None>"; \
        if (ImGui::BeginCombo(label, labelNameOuter.c_str())) \
        {

#define ENUM_ITEM(_NAME, _DESCRIPTION) \
            { \
                std::string labelName = #_NAME; \
                if (labelName == "Count") \
                    labelName = "<None>"; \
                bool is_selected = (std::string(#_NAME) == EnumToString(value)); \
                if (ImGui::Selectable(labelName.c_str(), is_selected)) \
                { \
                    value = EnumType::_NAME; \
                    ret = true; \
                } \
                if (is_selected) \
                    ImGui::SetItemDefaultFocus(); \
            }

#define ENUM_END() \
            ImGui::EndCombo();\
        } \
        if (ret) \
            OnUIValueChange(label, value, path); \
        ShowUIToolTip(tooltip); \
        return ret; \
    }

// Structs

#define STRUCT_BEGIN(_NAME, _DESCRIPTION) \
    bool ShowUI(RenderGraph& renderGraph, const char* label, const char* tooltip, _NAME& value, TypePathEntry path) \
    { \
        path = TypePathEntry(path, TypePathEntry(#_NAME)); \
        bool ret = false; \
        if (ShowUIOverrideBase(renderGraph, 0, ret, label, tooltip, value, path, ShowUIOverrideContext::Type) == UIOverrideResult::Finished) \
            return ret; \
        if (label && label[0]) \
        { \
            ImGui::Text(label); \
            ImGui::Indent(); \
            ImGui::PushID(label); \
            ShowUIToolTip(_DESCRIPTION); \
        } \

#define STRUCT_INHERIT_BEGIN(_NAME, _BASE, _DESCRIPTION) \
    bool ShowUI(RenderGraph& renderGraph, const char* label, const char* tooltip, _NAME& value, TypePathEntry path) \
    { \
        path = TypePathEntry(path, TypePathEntry(#_NAME)); \
        bool ret = false; \
        if (ShowUIOverrideBase(renderGraph, 0, ret, label, tooltip, value, path, ShowUIOverrideContext::Type) == UIOverrideResult::Finished) \
            return ret; \
        if (label && label[0]) \
        { \
            ImGui::Text(label); \
            ImGui::Indent(); \
            ImGui::PushID(label); \
            ShowUIToolTip(_DESCRIPTION); \
        } \
        ret |= ShowUI(renderGraph, nullptr, nullptr, *(_BASE*)&value, path);

#define STRUCT_FIELD(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS) \
        if (ShowUIOverrideBase(renderGraph, _FLAGS, ret, PrettyLabel(#_NAME).c_str(), _DESCRIPTION, value.##_NAME, TypePathEntry(path, TypePathEntry(#_NAME)), ShowUIOverrideContext::Field) == UIOverrideResult::Continue) \
        { \
            if (_FLAGS & SCHEMA_FLAG_UI_CONST) \
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true); \
            ret |= ShowUI(renderGraph, PrettyLabel(#_NAME).c_str(), _DESCRIPTION, value.##_NAME, TypePathEntry(path, TypePathEntry(#_NAME))); \
            if (_FLAGS & SCHEMA_FLAG_UI_CONST) \
                ImGui::PopItemFlag(); \
        }

#define STRUCT_CONST(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS)

#define STRUCT_DYNAMIC_ARRAY(_TYPE, _NAME, _DESCRIPTION, _FLAGS) \
        if (ShowUIOverrideBase(renderGraph, _FLAGS, ret, PrettyLabel(#_NAME).c_str(), _DESCRIPTION, value.##_NAME, TypePathEntry(path, TypePathEntry(#_NAME)), ShowUIOverrideContext::Field) == UIOverrideResult::Continue) \
            ret |= ShowUI(renderGraph, PrettyLabel(#_NAME).c_str(), _DESCRIPTION, value._NAME, TypePathEntry(path, TypePathEntry(#_NAME)), _FLAGS); \

#define STRUCT_STATIC_ARRAY(_TYPE, _NAME, _SIZE, _DEFAULT, _DESCRIPTION, _FLAGS) \
        if (ShowUIOverrideBase(renderGraph, _FLAGS, ret, PrettyLabel(#_NAME).c_str(), _DESCRIPTION, value.##_NAME, TypePathEntry(path, TypePathEntry(#_NAME)), ShowUIOverrideContext::Field) == UIOverrideResult::Continue) \
            ret |= ShowUI(renderGraph, PrettyLabel(#_NAME).c_str(), _DESCRIPTION, value._NAME, TypePathEntry(path, TypePathEntry(#_NAME)), _FLAGS); \

#define STRUCT_END() \
        if (label && label[0]) \
        { \
            ImGui::PopID(); \
            ImGui::Unindent(); \
        } \
        if (ret) \
            OnUIValueChange(label, value, path); \
        return ret; \
    }

// Variants

#define VARIANT_BEGIN(_NAME, _DESCRIPTION) \
    bool ShowUI(RenderGraph& renderGraph, const char* label, const char* tooltip, _NAME& value, TypePathEntry path) \
    { \
        path = TypePathEntry(path, TypePathEntry(#_NAME)); \
        bool ret = false; \
        if (ShowUIOverrideBase(renderGraph, 0, ret, label, tooltip, value, path, ShowUIOverrideContext::Type) == UIOverrideResult::Finished) \
            return ret; \
        bool showLabel = label && label[0]; \
        typedef _NAME ThisType; \
        switch(value._index) \
        { \

#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
            case ThisType::c_index_##_NAME: ret |= ShowUI(renderGraph, showLabel ? PrettyLabel(#_NAME).c_str() : nullptr, showLabel ? _DESCRIPTION : nullptr, value._NAME, TypePathEntry(path, TypePathEntry(#_NAME))); return ret;

#define VARIANT_END() \
        } \
        if (ret) \
            OnUIValueChange(label, value, path); \
        return ret; \
    }

// A catch all for OnUIValueChange, for things that are not overridden
template <typename T>
inline void OnUIValueChange(const char* label, const T& value, TypePathEntry path)
{
    int ijkl = 0;
}

// Overridden OnUIValueChange functions 

void OnUIValueChange(const char* label, const std::string& value, TypePathEntry path)
{
    switch (path())
    {
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_nodes, TypePaths::RenderGraphNode::cVariant, TypePaths::RenderGraphNode::c_actionSubGraph, TypePaths::RenderGraphNode_Action_SubGraph::cStruct, TypePaths::RenderGraphNode_Action_SubGraph::c_fileName)():
        {
            RefreshSubGraphNodes(); // We don't know which node changed, so refresh them all.
            break;
        }
    }
}

// A catch all template type to make compile errors about unsupported types easier to understand

template <typename T>
bool ShowUI(RenderGraph& renderGraph, const char* label, const char* tooltip, T& value, TypePathEntry path)
{
    // an update on 3/31/22 makes vs2022 makes this not able to be a static assert ):
    Assert(false, __FUNCSIG__ ": Unsupported type encountered!");
    return false;
}

// Built in types

bool ShowUI(RenderGraph& renderGraph, const char* label, const char* tooltip, std::string& value, TypePathEntry path)
{
    char buffer[4096];
    strcpy_s(buffer, value.c_str());

    // SCHEMA_FLAG_UI_MULTILINETEXT

    if (ImGui::InputText(label, buffer, sizeof(buffer)))
    {
        value = buffer;
        OnUIValueChange(label, value, path);
        return true;
    }
    ShowUIToolTip(tooltip);

    return false;
}

bool ShowUI(RenderGraph& renderGraph, const char* label, const char* tooltip, bool& value, TypePathEntry path)
{
    bool ret = ImGui::Checkbox(label, &value);
    ShowUIToolTip(tooltip);
    if (ret)
        OnUIValueChange(label, value, path);
    return ret;
}

bool ShowUI(RenderGraph& renderGraph, const char* label, const char* tooltip, float& value, TypePathEntry path)
{
    bool ret = ImGui::InputFloat(label, &value);
    ShowUIToolTip(tooltip);
    if (ret)
        OnUIValueChange(label, value, path);
    return ret;
}

bool ShowUI(RenderGraph& renderGraph, const char* label, const char* tooltip, int& value, TypePathEntry path)
{
    bool ret = ImGui::InputInt(label, &value, 0);
    ShowUIToolTip(tooltip);
    if (ret)
        OnUIValueChange(label, value, path);
    return ret;
}

bool ShowUI(RenderGraph& renderGraph, const char* label, const char* tooltip, unsigned int& value_, TypePathEntry path)
{
    int value = (int)value_;
    bool ret = ImGui::InputInt(label, &value, 0);
    if (ret)
        value_ = (unsigned int)value;
    ShowUIToolTip(tooltip);
    if (ret)
        OnUIValueChange(label, value_, path);
    return ret;
}

bool ShowUI(RenderGraph& renderGraph, const char* label, const char* tooltip, unsigned char& value_, TypePathEntry path)
{
    int value = (int)value_;
    bool ret = ImGui::InputInt(label, &value, 0);
    if (ret)
        value_ = (unsigned char)value;
    ShowUIToolTip(tooltip);
    if (ret)
        OnUIValueChange(label, value_, path);
    return ret;
}

// Static array UI function
// Dynamic and static should probably be merged more deeply.
// Dynamic should do what static does, but add the extra buttons.
template <typename T, size_t N>
bool ShowUI(RenderGraph& renderGraph, const char* label, const char* tooltip, TSTATICARRAY<T, N>& value, TypePathEntry path, size_t _FLAGS)
{
    bool ret = false;

    ImGui::PushID(label);
    // Special case where the array represents a color.
    if ((_FLAGS & SCHEMA_FLAG_UI_COLOR) != 0 && (N == 3 || N == 4))
    {
        // Only enable this code path for float.
        if constexpr (std::is_same_v<T, float>)
        {
			if (N == 3)
			{
				ImGui::ColorEdit3(label, value.data());
			}
			else if (N == 4)
			{
				ImGui::ColorEdit4(label, value.data());
			}
        }
    }
    else
    {
		if ((_FLAGS & SCHEMA_FLAG_UI_ARRAY_FATITEMS) != 0)
		{
			ImGui::Text("%s[%i]", label, (int)N);
			ShowUIToolTip(tooltip);
		}

		if ((_FLAGS & SCHEMA_FLAG_UI_ARRAY_FATITEMS) == 0)
		{
			float width = ImGui::GetContentRegionAvail().x / float(N + 2);
			ImGui::PushItemWidth(width);
		}

		bool showIndex = ((_FLAGS & SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX) == 0);

		if ((_FLAGS & SCHEMA_FLAG_UI_ARRAY_FATITEMS) != 0)
			ImGui::Indent();

		for (size_t index = 0; index < N; ++index)
		{
			ImGui::PushID((int)index);
			char label[256];
			sprintf_s(label, "[%i]", (int)index);
			ret |= ShowUI(renderGraph, showIndex ? label : "", nullptr, value[index], path);
			if ((_FLAGS & SCHEMA_FLAG_UI_ARRAY_FATITEMS) == 0 && index + 1 < N)
				ImGui::SameLine();
			ImGui::PopID();
		}

		if ((_FLAGS & SCHEMA_FLAG_UI_ARRAY_FATITEMS) == 0)
		{
			ImGui::SameLine();
			ImGui::Text("%s", label);
			ShowUIToolTip(tooltip);
		}

		if ((_FLAGS & SCHEMA_FLAG_UI_ARRAY_FATITEMS) != 0)
			ImGui::Unindent();

		if ((_FLAGS & SCHEMA_FLAG_UI_ARRAY_FATITEMS) == 0)
		{
			ImGui::PopItemWidth();
		}
    }

    ImGui::PopID();

    if (ret)
        OnUIValueChange(label, value, path);

    return ret;
}

// copied from ImGui, the optional endMarker adds a rectangle to the triangle arrow indicating a stop
// useful to scroll to beginning or end
bool ArrowButton2(const char* str_id, ImGuiDir dir, bool smallButton, bool endMarker)
{
	if (smallButton)
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

	ImGuiButtonFlags flags = ImGuiButtonFlags_None;
	float sz = ImGui::GetFrameHeight();
	ImVec2 size(sz, sz);

	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	const ImGuiID id = window->GetID(str_id);
	const ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + size.x, window->DC.CursorPos.y + size.y));
	const float default_size = ImGui::GetFrameHeight();
	ImGui::ItemSize(size, (size.y >= default_size) ? g.Style.FramePadding.y : -1.0f);

	if (window->SkipItems || !ImGui::ItemAdd(bb, id))
	{
		if (smallButton)
			ImGui::PopStyleVar();

		return false;
	}

	if (g.CurrentItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 bg_col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	const ImU32 text_col = ImGui::GetColorU32(ImGuiCol_Text);
	ImGui::RenderNavHighlight(bb, id);
	ImGui::RenderFrame(bb.Min, bb.Max, bg_col, true, g.Style.FrameRounding);
	ImVec2 pos = bb.Min;
	pos.x += ImMax(0.0f, (size.x - g.FontSize) * 0.5f);
	pos.y += ImMax(0.0f, (size.y - g.FontSize) * 0.5f);
	pos.x = roundf(pos.x);
	pos.y = roundf(pos.y);
	ImGui::RenderArrow(window->DrawList, pos, text_col, dir);
	if (endMarker)
	{
		const float h = roundf(g.FontSize / 8);
		const float w = roundf(g.FontSize - 2 * h);
		if (dir == ImGuiDir_Up || dir == ImGuiDir_Down)
		{
			pos.x += h;
			pos.y += h;
			if (dir == ImGuiDir_Down)
				pos.y += g.FontSize - 3 * h;
			window->DrawList->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), text_col, 0.0f);
		}
		else
		{
			pos.x += h;
			pos.y += h;
			if (dir == ImGuiDir_Right)
				pos.x += g.FontSize - 3 * h;
			window->DrawList->AddRectFilled(pos, ImVec2(pos.x + h, pos.y + w), text_col, 0.0f);
		}
	}

	if (smallButton)
		ImGui::PopStyleVar();

	IMGUI_TEST_ENGINE_ITEM_INFO(id, str_id, g.LastItemData.StatusFlags);
	return pressed;
}


// Dynamic array UI function
template <typename T>
bool ShowUI(RenderGraph& renderGraph, const char* label, const char* tooltip, TDYNAMICARRAY<T>& value, TypePathEntry path, size_t _FLAGS)
{
    bool ret = false;

    ImGui::PushID(label);

    bool showIndex = ((_FLAGS & SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX) == 0);

    ImGui::Indent();
    int deleteIndex = -1;
    int moveUpIndex = -1;
    int moveDownIndex = -1;
    int moveTopIndex = -1;
    int moveBottomIndex = -1;
    int duplicateIndex = -1;

    for (int index = 0; index < TDYNAMICARRAY_SIZE(value); ++index)
    {
        ImGui::PushID(index);

        {
            char localLabel[256];
			sprintf_s(localLabel, "%s[%i]", label, index);
			ImGui::TextUnformatted(localLabel);
			ShowUIToolTip(tooltip);
        }

        ImGui::SameLine();
		if (ImGui::SmallButton("X"))
			deleteIndex = index;
		ShowUIToolTip("Delete");
        ImGui::SameLine();
        {
			ImGui_Enabled enabled(index != 0);
			if (ArrowButton2("Up", ImGuiDir_Up, true, false))
                moveUpIndex = index;
			ShowUIToolTip("Up");
            ImGui::SameLine();
        }
        {
			ImGui_Enabled enabled(index + 1 != TDYNAMICARRAY_SIZE(value));
			if (ArrowButton2("Down", ImGuiDir_Down, true, false))
                moveDownIndex = index;
			ShowUIToolTip("Down");
			ImGui::SameLine();
        }
		{
			ImGui_Enabled enabled(index != 0);
			if (ArrowButton2("Up to Top", ImGuiDir_Up, true, true))
                moveTopIndex = index;
			ShowUIToolTip("Up To Top");
            ImGui::SameLine();
        }
		{
			ImGui_Enabled enabled(index + 1 != TDYNAMICARRAY_SIZE(value));
			if (ArrowButton2("Down to Bottom", ImGuiDir_Down, true, true))
                moveBottomIndex = index;
			ShowUIToolTip("Down to Bottom");
        }

        ImGui::SameLine();
        if (ImGui::SmallButton("Duplicate"))
            duplicateIndex = index;
        ShowUIToolTip("Duplicate");

        ImGui::Indent();

        // show sub UI without name and without tooltip
        ret |= ShowUI(renderGraph, "", nullptr, value[index], path);

        ImGui::Unindent();

        ImGui::PopID();
    }
    if (duplicateIndex >= 0)
    {
        // make room for another item at the end and move everything down one
        TDYNAMICARRAY_RESIZE(value, TDYNAMICARRAY_SIZE(value) + 1);
        for (int index = (int)TDYNAMICARRAY_SIZE(value) - 1; index > duplicateIndex; --index)
            value[index] = value[index - 1];

        // we modified the render graph
        ret = true;
    }
    if (deleteIndex >= 0)
    {
        for (int index = deleteIndex; index < TDYNAMICARRAY_SIZE(value) - 1; ++index)
            value[index] = value[index+1];
        TDYNAMICARRAY_RESIZE(value, TDYNAMICARRAY_SIZE(value) - 1);
        ret = true;
    }
    if (moveUpIndex > 0)
    {
        T temp = value[moveUpIndex-1];
        value[moveUpIndex-1]  = value[moveUpIndex];
        value[moveUpIndex] = temp;
        ret = true;
    }
    if (moveDownIndex >= 0 && (moveDownIndex + 1) < TDYNAMICARRAY_SIZE(value))
    {
        T temp = value[moveDownIndex+1];
        value[moveDownIndex+1]  = value[moveDownIndex];
        value[moveDownIndex] = temp;
        ret = true;
    }
    if (moveTopIndex > 0)
    {
        for (int i = moveTopIndex; i > 0; i--)
            std::swap(value[i - 1], value[i]);
        ret = true;
    }
    if (moveBottomIndex >= 0 && (moveBottomIndex + 1) < TDYNAMICARRAY_SIZE(value))
    {
        for (int i = moveBottomIndex; (i + 1) < TDYNAMICARRAY_SIZE(value); ++i)
            std::swap(value[i], value[i + 1]);
        ret = true;
    }

    ImGui::Unindent();

	char localLabel[256];
	sprintf_s(localLabel, "Add %s[] Item", label);
    if (ImGui::Button(localLabel))
    {
        ret = true;
        TDYNAMICARRAY_RESIZE(value, TDYNAMICARRAY_SIZE(value) + 1);
    }

    ImGui::PopID();

    if (ret)
        OnUIValueChange(label, value, path);

    return ret;
}

template <typename T>
inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, T& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return UIOverrideResult::Continue;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, NodePinReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::Text(label);
    ImGui::SameLine();

    ImGui::PushID(label);

    // Get the list of nodes
    float nodeWidth = 0.0f;
    std::vector<std::string> nodes;
    nodes.push_back("");
    for (const RenderGraphNode& node : renderGraph.nodes)
    {
        nodes.push_back(GetNodeName(node));
        nodeWidth = std::max(nodeWidth, ImGui::CalcTextSize(nodes.rbegin()->c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f);
    }

    // get the list of pins on the selected node
    float pinWidth = 0.0f;
    std::vector<std::string> pins;
    pins.push_back("");
    int nodeIndex = GetNodeIndexByName(renderGraph, value.node.c_str());
    if (nodeIndex >= 0)
    {
        std::vector<NodePinInfo> pinInfo = GetNodePins(renderGraph, renderGraph.nodes[nodeIndex]);
        for (const NodePinInfo& info : pinInfo)
        {
            pins.push_back(info.name);
            pinWidth = std::max(pinWidth, ImGui::CalcTextSize(pins.rbegin()->c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f);
        }
    }

    // Node drop down
    ImGui::SetNextItemWidth(nodeWidth + ImGui::GetTextLineHeightWithSpacing() + 10);
    if (ImGui::BeginCombo("##node", value.node.c_str()))
    {
        for (const std::string& node : nodes)
        {
            bool is_selected = value.node == node;
            if (ImGui::Selectable(node.c_str(), is_selected))
            {
                value.node = node;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    ImGui::SameLine();

    // pin drop down
    ImGui::SetNextItemWidth(pinWidth + ImGui::GetTextLineHeightWithSpacing() + 10);
    if (ImGui::BeginCombo("##node", value.pin.c_str()))
    {
        for (const std::string& pin : pins)
        {
            bool is_selected = value.pin == pin;
            if (ImGui::Selectable(pin.c_str(), is_selected))
            {
                value.pin = pin;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    ImGui::PopID();
    ShowUIToolTip(tooltip);
    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, NodePinReferenceOptional& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::Text(label);
    ImGui::SameLine();

    ImGui::PushID(label);

    // Get the list of nodes
    float nodeWidth = 0.0f;
    std::vector<std::string> nodes;
    nodes.push_back("");
    for (const RenderGraphNode& node : renderGraph.nodes)
    {
        nodes.push_back(GetNodeName(node));
        nodeWidth = std::max(nodeWidth, ImGui::CalcTextSize(nodes.rbegin()->c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f);
    }

    // get the list of pins on the selected node
    float pinWidth = 0.0f;
    std::vector<std::string> pins;
    pins.push_back("");
    int nodeIndex = GetNodeIndexByName(renderGraph, value.node.c_str());
    if (nodeIndex >= 0)
    {
        std::vector<NodePinInfo> pinInfo = GetNodePins(renderGraph, renderGraph.nodes[nodeIndex]);
        for (const NodePinInfo& info : pinInfo)
        {
            pins.push_back(info.name);
            pinWidth = std::max(pinWidth, ImGui::CalcTextSize(pins.rbegin()->c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f);
        }
    }

    // Node drop down
    ImGui::SetNextItemWidth(nodeWidth + ImGui::GetTextLineHeightWithSpacing() + 10);
    if (ImGui::BeginCombo("##node", value.node.c_str()))
    {
        for (const std::string& node : nodes)
        {
            bool is_selected = value.node == node;
            if (ImGui::Selectable(node.c_str(), is_selected))
            {
                value.node = node;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    ImGui::SameLine();

    // pin drop down
    ImGui::SetNextItemWidth(pinWidth + ImGui::GetTextLineHeightWithSpacing() + 10);
    if (ImGui::BeginCombo("##pin", value.pin.c_str()))
    {
        for (const std::string& pin : pins)
        {
            bool is_selected = value.pin == pin;
            if (ImGui::Selectable(pin.c_str(), is_selected))
            {
                value.pin = pin;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    ImGui::PopID();
    ShowUIToolTip(tooltip);
    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, TextureNodeReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    // Texture nodes drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.nodes.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.nodes[index - 1]._index != RenderGraphNode::c_index_resourceTexture)
                continue;

            std::string label = (index == 0) ? " " : GetNodeName(renderGraph.nodes[index - 1]).c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);
    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, BufferNodeReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    // Buffer nodes drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.nodes.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.nodes[index - 1]._index != RenderGraphNode::c_index_resourceBuffer)
                continue;

            std::string label = (index == 0) ? " " : GetNodeName(renderGraph.nodes[index - 1]).c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);
    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, TextureOrBufferNodeReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    // Texture and buffer nodes drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.nodes.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.nodes[index - 1]._index != RenderGraphNode::c_index_resourceTexture && renderGraph.nodes[index - 1]._index != RenderGraphNode::c_index_resourceBuffer)
                continue;

            std::string label = (index == 0) ? " " : GetNodeName(renderGraph.nodes[index - 1]).c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);
    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, ComputeShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    // Compute shader drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.shaders.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.shaders[index - 1].type != ShaderType::Compute)
                continue;

            std::string label = (index == 0) ? " " : renderGraph.shaders[index - 1].name.c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToShader(value.name.c_str());
    ShowUIToolTip("Go to shader");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RayGenShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    // Ray gen shader drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.shaders.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.shaders[index - 1].type != ShaderType::RTRayGen)
                continue;

            std::string label = (index == 0) ? " " : renderGraph.shaders[index - 1].name.c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToShader(value.name.c_str());
    ShowUIToolTip("Go to shader");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RTAnyHitShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    // Ray gen shader drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.shaders.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.shaders[index - 1].type != ShaderType::RTAnyHit)
                continue;

            std::string label = (index == 0) ? " " : renderGraph.shaders[index - 1].name.c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToShader(value.name.c_str());
    ShowUIToolTip("Go to shader");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RTClosestHitShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    // Ray gen shader drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.shaders.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.shaders[index - 1].type != ShaderType::RTClosestHit)
                continue;

            std::string label = (index == 0) ? " " : renderGraph.shaders[index - 1].name.c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToShader(value.name.c_str());
    ShowUIToolTip("Go to shader");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RTIntersectionShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    // Ray gen shader drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.shaders.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.shaders[index - 1].type != ShaderType::RTIntersection)
                continue;

            std::string label = (index == 0) ? " " : renderGraph.shaders[index - 1].name.c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToShader(value.name.c_str());
    ShowUIToolTip("Go to shader");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RTAnyHitShaderReferenceOptional& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    // Ray gen shader drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.shaders.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.shaders[index - 1].type != ShaderType::RTAnyHit)
                continue;

            std::string label = (index == 0) ? " " : renderGraph.shaders[index - 1].name.c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToShader(value.name.c_str());
    ShowUIToolTip("Go to shader");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RTClosestHitShaderReferenceOptional& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    // Ray gen shader drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.shaders.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.shaders[index - 1].type != ShaderType::RTClosestHit)
                continue;

            std::string label = (index == 0) ? " " : renderGraph.shaders[index - 1].name.c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToShader(value.name.c_str());
    ShowUIToolTip("Go to shader");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RTIntersectionShaderReferenceOptional& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    // Ray gen shader drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.shaders.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.shaders[index - 1].type != ShaderType::RTIntersection)
                continue;

            std::string label = (index == 0) ? " " : renderGraph.shaders[index - 1].name.c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToShader(value.name.c_str());
    ShowUIToolTip("Go to shader");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, VertexShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    // Ray gen shader drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.shaders.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.shaders[index - 1].type != ShaderType::Vertex)
                continue;

            std::string label = (index == 0) ? " " : renderGraph.shaders[index - 1].name.c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToShader(value.name.c_str());
    ShowUIToolTip("Go to shader");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, VertexShaderReferenceOptional& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    // Ray gen shader drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.shaders.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.shaders[index - 1].type != ShaderType::Vertex)
                continue;

            std::string label = (index == 0) ? " " : renderGraph.shaders[index - 1].name.c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToShader(value.name.c_str());
    ShowUIToolTip("Go to shader");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, PixelShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    // Ray gen shader drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.shaders.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.shaders[index - 1].type != ShaderType::Pixel)
                continue;

            std::string label = (index == 0) ? " " : renderGraph.shaders[index - 1].name.c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToShader(value.name.c_str());
    ShowUIToolTip("Go to shader");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, PixelShaderReferenceOptional& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    // Ray gen shader drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.shaders.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.shaders[index - 1].type != ShaderType::Pixel)
                continue;

            std::string label = (index == 0) ? " " : renderGraph.shaders[index - 1].name.c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToShader(value.name.c_str());
    ShowUIToolTip("Go to shader");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, AmplificationShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    // Ray gen shader drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.shaders.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.shaders[index - 1].type != ShaderType::Amplification)
                continue;

            std::string label = (index == 0) ? " " : renderGraph.shaders[index - 1].name.c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToShader(value.name.c_str());
    ShowUIToolTip("Go to shader");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, AmplificationShaderReferenceOptional& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    // Ray gen shader drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.shaders.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.shaders[index - 1].type != ShaderType::Amplification)
                continue;

            std::string label = (index == 0) ? " " : renderGraph.shaders[index - 1].name.c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToShader(value.name.c_str());
    ShowUIToolTip("Go to shader");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, MeshShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    // Ray gen shader drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.shaders.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.shaders[index - 1].type != ShaderType::Mesh)
                continue;

            std::string label = (index == 0) ? " " : renderGraph.shaders[index - 1].name.c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToShader(value.name.c_str());
    ShowUIToolTip("Go to shader");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, MeshShaderReferenceOptional& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    // Ray gen shader drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.shaders.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.shaders[index - 1].type != ShaderType::Mesh)
                continue;

            std::string label = (index == 0) ? " " : renderGraph.shaders[index - 1].name.c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToShader(value.name.c_str());
    ShowUIToolTip("Go to shader");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, VariableReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        // Sort the list of variables
        std::vector<std::string> vars;
        for (const Variable& var : renderGraph.variables)
            vars.push_back(var.name);
        CaseInsensitiveSort(vars);

        // add a blank to the beginning
        vars.insert(vars.begin(), "");

        // Show a drop down
        for (const std::string& label : vars)
        {
            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToVariable(value.name.c_str());
    ShowUIToolTip("Go to Variable");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, VariableReferenceNoConst& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        // Sort the list of variables
        std::vector<std::string> vars;
        for (const Variable& var : renderGraph.variables)
        {
            if (var.Const)
                continue;
            vars.push_back(var.name);
        }
        CaseInsensitiveSort(vars);

        // add a blank to the beginning
        vars.insert(vars.begin(), "");

        // Show a drop down
        for (const std::string& label : vars)
        {
            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToVariable(value.name.c_str());
    ShowUIToolTip("Go to Variable");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, VariableReferenceConstOnly& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        // Sort the list of variables
        std::vector<std::string> vars;
        for (const Variable& var : renderGraph.variables)
        {
            if (!var.Const)
                continue;
            vars.push_back(var.name);
        }
        CaseInsensitiveSort(vars);

        // add a blank to the beginning
        vars.insert(vars.begin(), "");

        // Show a drop down
        for (const std::string& label : vars)
        {
            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToVariable(value.name.c_str());
    ShowUIToolTip("Go to Variable");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, StructReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.structs.size() + 1; ++index)
        {
            std::string label = (index == 0) ? " " : renderGraph.structs[index - 1].name.c_str();

            bool is_selected = value.name == label;
            std::string safeLabel = label + "##";
            if (ImGui::Selectable(safeLabel.c_str(), is_selected))
            {
                value.name = (index == 0) ? "" : label;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    ImGui::SameLine();
    if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
        OnGoToStruct(value.name.c_str());
    ShowUIToolTip("Go to Struct");

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RenderGraphNode_Action_SubGraph& node, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    if (ImGui::Button("Refresh GG File Data"))
        dirtyFlag = RefreshSubGraphNode(node);
    return UIOverrideResult::Continue;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, ShaderResourceType& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    if (path() != TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_shaders, TypePaths::Shader::cStruct, TypePaths::Shader::cStruct, TypePaths::Shader::c_resources, TypePaths::ShaderResource::cStruct, TypePaths::ShaderResource::c_type)())
        return UIOverrideResult::Continue;

    std::string labelNameOuter = EnumToString(value);
    if (labelNameOuter == "Count")
        labelNameOuter = "<None>";

    if (ImGui::BeginCombo(label, labelNameOuter.c_str()))
    {
        for (int i = 0; i <= (int)ShaderResourceType::Count; ++i)
        {
            if (i == (int)ShaderResourceType::ConstantBuffer)
                continue;

            std::string labelName = EnumToString((ShaderResourceType)i);
            if (labelName == "Count")
                labelName = "<None>";

            bool is_selected = (std::string(labelName) == EnumToString(value));
            if (ImGui::Selectable(labelName.c_str(), is_selected))
            {
                value = (ShaderResourceType)i;
                dirtyFlag = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ShowUIToolTip(tooltip);

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, std::string& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    // Show "Dflt" as "Default" in the editor, instead.
    // default is a reserved word so we couldn't use it, but we can show it as a friendlier label!
    if (!_stricmp(label, "Dflt"))
    {
        dirtyFlag |= ShowUI(renderGraph, "Default", tooltip, value, path);
        return UIOverrideResult::Finished;
    }
    // Show a variable drop down for SubGraphVariableSettings.ReplaceWithStr, and call it "Replace With" instead.
    else if (!_stricmp(label, "Replace With Str"))
    {
        VariableReference dummyRef;
        dummyRef.name = value;
        UIOverrideResult ret = ShowUIOverride(renderGraph, _FLAGS, dirtyFlag, "Replace With", tooltip, dummyRef, path, showUIOverrideContext);
        value = dummyRef.name;
        return ret;
    }
    // any label of "fileName" can have a file dialog box.
    else if (!_stricmp(label,"fileName") || !_stricmp(label, "File Name"))
    {
        dirtyFlag |= ShowUI(renderGraph, label, nullptr, value, path);

        ShowUIToolTip(tooltip);

        std::filesystem::path defaultPath = std::filesystem::path(renderGraph.editorFileName).remove_filename();

        std::string exploreLocation;

        if (value.empty())
            exploreLocation = defaultPath.u8string();
        else
            exploreLocation = (defaultPath / std::filesystem::path(value)).remove_filename().u8string();
        exploreLocation = std::filesystem::absolute(std::filesystem::path(exploreLocation)).u8string();

        if (ImGui::Button("..."))
        {
            nfdchar_t* outPath = nullptr;
            if (NFD_OpenDialog(nullptr, exploreLocation.c_str(), &outPath) == NFD_OKAY)
            {
                value = defaultPath.empty() ? outPath : std::filesystem::relative(outPath, defaultPath).u8string();
                dirtyFlag = true;
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Edit"))
        {
            std::filesystem::path fullFileName = (defaultPath / std::filesystem::path(value));

            if (!_stricmp(fullFileName.extension().string().c_str(), ".gg"))
            {
                // Launch the process
                char commandLine[1024];
                sprintf_s(commandLine, "\"GigiEdit.exe\" \"%s\"", fullFileName.string().c_str());
                STARTUPINFOA si;
                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                PROCESS_INFORMATION pi;

                CreateProcessA(
                    nullptr,
                    commandLine,
                    nullptr,
                    nullptr,
                    FALSE,
                    0,
                    nullptr,
                    nullptr,
                    &si,
                    &pi);
            }
            else
            {
                ShellExecuteA(NULL, "open", fullFileName.string().c_str(), NULL, NULL, SW_SHOWDEFAULT);
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Explore"))
        {
            ShellExecuteA(NULL, "explore", exploreLocation.c_str(), NULL, NULL, SW_SHOWDEFAULT);
        }

        return UIOverrideResult::Finished;
    }

    switch(path())
    {
        // Enum drop down
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_variables, TypePaths::Variable::cStruct, TypePaths::Variable::c_Enum)():
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_structs, TypePaths::Struct::cStruct, TypePaths::Struct::c_fields, TypePaths::StructField::cStruct, TypePaths::StructField::c_Enum)():
        {
            if (ImGui::BeginCombo(label, value.c_str()))
            {
                for (int enumIndex = 0; enumIndex < renderGraph.enums.size() + 1; ++enumIndex)
                {
                    std::string label = (enumIndex == 0) ? " " : renderGraph.enums[enumIndex-1].name.c_str();

                    bool is_selected = value == label;
                    std::string safeLabel = label + "##";
                    if (ImGui::Selectable(safeLabel.c_str(), is_selected))
                    {
                        value = (enumIndex == 0) ? "" : label;
                        dirtyFlag = true;
                    }
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }
            ShowUIToolTip(tooltip);
            return UIOverrideResult::Finished;
        }

        // Variable drop down
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_setVars, TypePaths::SetVariable::cStruct, TypePaths::SetVariable::c_condition, TypePaths::Condition::cStruct, TypePaths::Condition::c_variable1)() :
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_setVars, TypePaths::SetVariable::cStruct, TypePaths::SetVariable::c_condition, TypePaths::Condition::cStruct, TypePaths::Condition::c_variable2)() :
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_nodes, TypePaths::RenderGraphNode::cVariant, TypePaths::RenderGraphNode::c_actionComputeShader, TypePaths::RenderGraphNode_Action_ComputeShader::cStruct, TypePaths::RenderGraphNode_ActionBase::cStruct, TypePaths::RenderGraphNode_ActionBase::c_condition, TypePaths::Condition::cStruct, TypePaths::Condition::c_variable1)() :
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_nodes, TypePaths::RenderGraphNode::cVariant, TypePaths::RenderGraphNode::c_actionComputeShader, TypePaths::RenderGraphNode_Action_ComputeShader::cStruct, TypePaths::RenderGraphNode_ActionBase::cStruct, TypePaths::RenderGraphNode_ActionBase::c_condition, TypePaths::Condition::cStruct, TypePaths::Condition::c_variable2)() :
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_nodes, TypePaths::RenderGraphNode::cVariant, TypePaths::RenderGraphNode::c_actionRayShader, TypePaths::RenderGraphNode_Action_RayShader::cStruct, TypePaths::RenderGraphNode_ActionBase::cStruct, TypePaths::RenderGraphNode_ActionBase::c_condition, TypePaths::Condition::cStruct, TypePaths::Condition::c_variable1)() :
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_nodes, TypePaths::RenderGraphNode::cVariant, TypePaths::RenderGraphNode::c_actionRayShader, TypePaths::RenderGraphNode_Action_RayShader::cStruct, TypePaths::RenderGraphNode_ActionBase::cStruct, TypePaths::RenderGraphNode_ActionBase::c_condition, TypePaths::Condition::cStruct, TypePaths::Condition::c_variable2)():
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_nodes, TypePaths::RenderGraphNode::cVariant, TypePaths::RenderGraphNode::c_actionCopyResource, TypePaths::RenderGraphNode_Action_CopyResource::cStruct, TypePaths::RenderGraphNode_ActionBase::cStruct, TypePaths::RenderGraphNode_ActionBase::c_condition, TypePaths::Condition::cStruct, TypePaths::Condition::c_variable1)() :
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_nodes, TypePaths::RenderGraphNode::cVariant, TypePaths::RenderGraphNode::c_actionCopyResource, TypePaths::RenderGraphNode_Action_CopyResource::cStruct, TypePaths::RenderGraphNode_ActionBase::cStruct, TypePaths::RenderGraphNode_ActionBase::c_condition, TypePaths::Condition::cStruct, TypePaths::Condition::c_variable2)() :
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_nodes, TypePaths::RenderGraphNode::cVariant, TypePaths::RenderGraphNode::c_actionDrawCall, TypePaths::RenderGraphNode_Action_DrawCall::cStruct, TypePaths::RenderGraphNode_ActionBase::cStruct, TypePaths::RenderGraphNode_ActionBase::c_condition, TypePaths::Condition::cStruct, TypePaths::Condition::c_variable1)() :
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_nodes, TypePaths::RenderGraphNode::cVariant, TypePaths::RenderGraphNode::c_actionDrawCall, TypePaths::RenderGraphNode_Action_DrawCall::cStruct, TypePaths::RenderGraphNode_ActionBase::cStruct, TypePaths::RenderGraphNode_ActionBase::c_condition, TypePaths::Condition::cStruct, TypePaths::Condition::c_variable2)() :
        {
            if (ImGui::BeginCombo(label, value.c_str()))
            {
                // Sort the list of variables
                std::vector<std::string> vars;
                for (const Variable& var : renderGraph.variables)
                    vars.push_back(var.name);
                CaseInsensitiveSort(vars);

                // add a blank to the beginning
                vars.insert(vars.begin(), " ");

                // Show a drop down
                for (const std::string& label : vars)
                {
                    bool is_selected = value == label;
                    std::string safeLabel = label + "##";
                    if (ImGui::Selectable(safeLabel.c_str(), is_selected))
                    {
                        value = (label == " ") ? "" : label;
                        dirtyFlag = true;
                    }
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }
            ShowUIToolTip(tooltip);
            return UIOverrideResult::Finished;
        }

        // Struct field drop down
        // NOTE: this would be a good idea but we don't know what struct it is for, because we don't know what node it is
        /*
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_nodes, TypePaths::RenderGraphNode::cVariant, TypePaths::RenderGraphNode::c_resourceShaderConstants, TypePaths::RenderGraphNode_Resource_ShaderConstants::cStruct, TypePaths::RenderGraphNode_Resource_ShaderConstants::c_setFromVar, TypePaths::SetCBFromVar::cStruct, TypePaths::SetCBFromVar::c_field)():
        {
            return UIOverrideResult::Finished;
        }
        */
    }

    if ((_FLAGS & SCHEMA_FLAG_UI_MULTILINETEXT) != 0)
    {
        auto ResizeCallback = [](ImGuiInputTextCallbackData* data) -> int
        {
            if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
            {
                std::string* my_str = (std::string*)data->UserData;
                my_str->resize(data->BufSize);
                data->Buf = &(*my_str)[0];
            }
            return 0;
        };

        dirtyFlag |= ImGui::InputTextMultiline(label, &value[0], (size_t)value.capacity(), ImVec2(0, 0), ImGuiInputTextFlags_CallbackResize, ResizeCallback, (void*)&value);

        ShowUIToolTip(tooltip, false);
        return UIOverrideResult::Finished;
    }

    return UIOverrideResult::Continue;
}

template <typename T>
inline UIOverrideResult ShowUIOverrideBase(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, T& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    if (((_FLAGS)&SCHEMA_FLAG_NO_UI) != 0)
        return UIOverrideResult::Finished;

    if (((_FLAGS)&SCHEMA_FLAG_NO_SERIALIZE) != 0)
        return UIOverrideResult::Finished;

    if (((_FLAGS)&SCHEMA_FLAG_UI_COLLAPSABLE) != 0 && label)
    {
        if (!ImGui::CollapsingHeader(label))
            return UIOverrideResult::Finished;
    }

    UIOverrideResult ret = ShowUIOverride(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
    if (dirtyFlag)
        OnUIValueChange(label, value, path);
    return ret;
}

#include "Schemas/Schemas.h"

struct ShaderTypeCodeGenerator
{
    ShaderTypeCodeGenerator(const RenderGraph& renderGraph, const Shader& shader, FILE* file)
        : m_renderGraph(renderGraph)
        , m_shader(shader)
        , m_file(file)
    {
    }

    void Function_Compute() const
    {
        fprintf(m_file, "/*$(_compute:%s)*/(uint3 DTid : SV_DispatchThreadID)\n{\n}\n", m_shader.entryPoint.c_str());
    }

    void Function_RTRayGen()
    {
        // get the name of the first (perhaps only) RT scene resource
        std::string sceneBufferName = "\t// TODO: Scene (TLAS) buffer here";
        for (const ShaderResource& resource : m_shader.resources)
        {
            if (resource.access == ShaderResourceAccessType::RTScene)
            {
                sceneBufferName = resource.name + ", // Scene (TLAS) buffer";
                break;
            }
        }

        // write the file
        fprintf(m_file,
            "struct Payload\n"
            "{\n"
            "\t// TODO: fill in the payload structure\n"
            "};\n"
            "\n"
            "/*$(_raygeneration:%s)*/\n"
            "{\n"
            "\tuint2 px = DispatchRaysIndex().xy;\n"
            "\tuint2 dims = DispatchRaysDimensions().xy;\n"
            "\n"
            "\tRayDesc ray;\n"
            "\t//ray.Origin =\n"
            "\t//ray.Direction =\n"
            "\tray.TMin = 0;\n"
            "\tray.TMax = 1000.0f;\n"
            "\n"
            "\tPayload payload = (Payload)0;\n"
            "\n"
            "\t/*$(RayTraceFn)*/(%s\n"
            "\t\tRAY_FLAG_FORCE_OPAQUE, // Ray flags\n"
            "\t\t0xFF, // Ray mask\n"
            "\t\t/*$(RTHitGroupIndex:)*/, // TODO: Add hit group name inside this Gigi macro\n"
            "\t\t/*$(RTHitGroupCount)*/,\n"
            "\t\t/*$(RTMissIndex:)*/, // TODO: Add miss shader name inside this Gigi macro\n"
            "\t\tray,\n"
            "\t\tpayload);\n"
            "}\n"
            "\n"
            "/*$(_miss:)*/\n"
            "{\n"
            "\t// TODO: add the miss entry point name to the Gigi macro above\n"
            "\t// TODO: fill out payload data\n"
            "\t//payload.hit = false;\n"
            "}\n"
            "\n"
            "/*$(_closesthit:)*/\n"
            "{\n"
            "\t// TODO: add the closest hit entry point name to the Gigi macro above, or remove this function, if appropriate\n"
            "\t// TODO: fill out payload data\n"
            "\t//payload.hit = true;\n"
            "}\n"
            "\n"
            "/*$(_anyhit:)*/\n"
            "{\n"
            "\t// TODO: add the any hit entry point name to the Gigi macro above, or remove this function, if appropriate\n"
            "\t// TODO: fill out payload data\n"
            "\t//payload.hit = true;\n"
            "}\n"
            "\n"
            "/*$(_intersection:)*/\n"
            "{\n"
            "\t// TODO: add the intersection entry point name to the Gigi macro above, or remove this function, if appropriate\n"
            "\t// TODO: fill out function\n"
            "}\n",
            m_shader.entryPoint.c_str(),
            sceneBufferName.c_str()
        );
    }

    void Function_RTAnyHit()
    {
    }

    void Function_RTIntersection()
    {
    }

    void Function_RTClosestHit()
    {
    }

    void Function_RTMiss()
    {
    }

    void Function_Vertex()
    {
        fprintf(m_file,
            "struct VSInput\n"
            "{\n"
            "\tfloat3 position   : POSITION;\n"
            "\tuint   vertexID   : SV_VertexID;\n"
            "\tuint   instanceId : SV_InstanceID;\n"
            "\t//TODO: fill this out\n"
            "};\n"
            "\n"
            "struct VSOutput // AKA PSInput\n"
            "{\n"
            "\tfloat4 position   : SV_POSITION;\n"
            "\t//TODO: fill this out\n"
            "};\n"
            "\n"
            "VSOutput %s(VSInput input)\n"
            "{\n"
            "\tVSOutput ret = (VSOutput)0;\n"
            "\t// TODO: fill this out\n"
            "\treturn ret;\n"
            "}\n",
            m_shader.entryPoint.c_str()
        );
    }

    void Function_Pixel()
    {
        fprintf(m_file,
            "struct PSInput // AKA VSOutput\n"
            "{\n"
            "\tfloat4 position   : SV_POSITION;\n"
            "\t//TODO: fill this out\n"
            "};\n"
            "\n"
            "struct PSOutput\n"
            "{\n"
            "\tfloat4 colorTarget : SV_Target0;\n"
            "\t//TODO: fill this out\n"
            "};\n"
            "\n"
            "PSOutput %s(PSInput input)\n"
            "{\n"
            "\tPSOutput ret = (PSOutput)0;\n"
            "\t// TODO: fill this out\n"
            "\treturn ret;\n"
            "}\n",
            m_shader.entryPoint.c_str()
        );
    }

    void Function_Amplification()
    {
        fprintf(m_file, "/*$(_amplification:%s)*/(uint3 dtid : SV_DispatchThreadID, uint3 gtid : SV_GroupThreadID, uint3 gid : SV_GroupID)\n{\n}\n", m_shader.entryPoint.c_str());
    }

    void Function_Mesh()
    {
        fprintf(m_file, "/*$(_mesh:%s)*/(uint3 dtid : SV_DispatchThreadID, uint3 gtid : SV_GroupThreadID, uint3 gid : SV_GroupID)\n{\n}\n", m_shader.entryPoint.c_str());
    }

    void Function_Count()
    {
    }

    const RenderGraph& m_renderGraph;
    const Shader& m_shader;
    FILE* m_file = nullptr;
};

template<>
inline UIOverrideResult ShowUIOverride<Shader>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, Shader& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    // This is kind of an ugly hack to keep from having infinite recursion.
    // We want to wrap the editing of a shader to react to changes, but we also want the default UI.
    static bool insideShader = false;
    if (insideShader)
        return UIOverrideResult::Continue;
    insideShader = true;

    // If the resources of a shader change, that can affect nodes that use this shader, or nodes that connect to nodes using this shader, so we may need to do some fix up
    TDYNAMICARRAY<ShaderResource> oldResources = value.resources;

    dirtyFlag |= ShowUI(renderGraph, label, tooltip, value, path);

    static int uniqueId = 0;
    uniqueId++;
    ImGui::PushID(&value);

    ImGui::Indent();
    if (ImGui::Button("Create Shader File"))
    {
        std::filesystem::path defaultPath = std::filesystem::path(renderGraph.editorFileName).remove_filename();

        std::string exploreLocation;

        if (value.fileName.empty())
            exploreLocation = defaultPath.u8string();
        else
            exploreLocation = (defaultPath / std::filesystem::path(value.fileName)).remove_filename().u8string();
        exploreLocation = std::filesystem::absolute(std::filesystem::path(exploreLocation)).u8string();

        nfdchar_t* outPath = nullptr;
        if (NFD_SaveDialog("hlsl", exploreLocation.c_str(), &outPath) == NFD_OKAY)
        {
            std::filesystem::path p(outPath);
            if (!p.has_extension() || p.extension() != ".hlsl")
                p.replace_extension(".hlsl");

            std::string fileName = defaultPath.empty() ? p.u8string() : std::filesystem::relative(p, defaultPath).u8string();
            if (value.fileName != fileName)
            {
                value.fileName = fileName;
                dirtyFlag = true;
            }

            // write out a shader file
            {
                FILE* file = nullptr;
                fopen_s(&file, p.string().c_str(), "wb");
                if (file)
                {
                    fprintf(file, "// %s technique, shader %s\n/*$(ShaderResources)*/\n\n", renderGraph.name.c_str(), value.name.c_str());

                    ShaderTypeCodeGenerator generator(renderGraph, value, file);
                    EnumDispatch(generator, value.type);

                    // Write out any shader resources
                    {
                        bool first = true;
                        if (value.resources.size() > 0)
                        {
                            if (first)
                            {
                                fprintf(file, "\n/*\n");
                                first = false;
                            }

                            fprintf(file, "Shader Resources:\n");
                            for (const ShaderResource& resource : value.resources)
                                fprintf(file, "\t%s %s (as %s)\n", EnumToString(resource.type), resource.name.c_str(), EnumToString(resource.access));
                        }

                        if (value.samplers.size() > 0)
                        {
                            if (first)
                            {
                                fprintf(file, "/*\n");
                                first = false;
                            }

                            fprintf(file, "Shader Samplers:\n");
                            for (const ShaderSampler& sampler : value.samplers)
                                fprintf(file, "\t%s filter: %s addressmode: %s\n", sampler.name.c_str(), EnumToString(sampler.filter), EnumToString(sampler.addressMode));
                        }

                        if (!first)
                            fprintf(file, "*/\n");
                    }

                    fclose(file);
                }
            }
        }
    }

    ImGui::Unindent();

    if (dirtyFlag)
    {
        // If a resource was added, let it react
        if (value.resources.size() > oldResources.size())
        {
            OnShaderResourceAdd(value, value.resources.rbegin()->name);
        }
        // If a resource was deleted, we need to unhook everything that was plugged into that pin
        else if (value.resources.size() < oldResources.size())
        {
            // find which index was deleted from the oldResources array. Note that it may have been the last one.
            int index = 0;
            while (index < value.resources.size() && value.resources[index].name == oldResources[index].name)
                index++;

            // Do the appropriate work for when this happens
            OnShaderResourceDelete(value, oldResources[index].name);
        }
        // Otherwise, there could have been a rename, or an item could have changed position in the array
        else
        {
            for (int index = 0; index < value.resources.size(); ++index)
            {
                // if we find a name mismatch that could be a rename or a reordering
                if (value.resources[index].name != oldResources[index].name)
                {
                    // If the next index also doesn't match, it's a reordering and nothing needs to be done
                    if (index + 1 < value.resources.size() && value.resources[index + 1].name != oldResources[index + 1].name)
                        break;

                    // otherwise it's a rename
                    OnShaderResourceRename(value, oldResources[index].name, value.resources[index].name);
                    break;
                }
            }
        }
    }

    insideShader = false;

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

template<>
inline UIOverrideResult ShowUIOverride<BackendRestriction>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, BackendRestriction& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::Text(label);
    ShowUIToolTip("Allows restrictions to specific backends. None checked means allowed for all backends.");

    ImGui::PushID(label);
    ImGui::Indent();

    auto ShowCheckbox = [&](const char* backendName, Backend backendValue)
    {
        bool checked = std::find(value.backends.begin(), value.backends.end(), backendValue) != value.backends.end();

        if (ImGui::Checkbox(backendName, &checked))
        {
            dirtyFlag = true;

            if (checked)
                value.backends.push_back(backendValue);
            else
                value.backends.erase(std::remove(value.backends.begin(), value.backends.end(), backendValue), value.backends.end());
        }
    };

    // clang-format off
    #include "external/df_serialize/_common.h"
    #define ENUM_ITEM(_NAME, _DESCRIPTION) ShowCheckbox(#_NAME, Backend::_NAME);
    #include "external/df_serialize/_fillunsetdefines.h"
    #include "Schemas/BackendList.h"
    // clang-format on

    ImGui::Unindent();
    ImGui::PopID();

    return UIOverrideResult::Finished;
}

template<>
inline UIOverrideResult ShowUIOverride<BufferCountDesc>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, BufferCountDesc& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    if (showUIOverrideContext != ShowUIOverrideContext::Type)
        return UIOverrideResult::Continue;

    bool onlyConstants = true;
    const char* baseSize = "1";
    char baseSizeBuffer[256];

    if (!value.node.name.empty())
    {
        onlyConstants = false;
        sprintf_s(baseSizeBuffer, "sizeof(%s).x", value.node.name.c_str());
        baseSize = baseSizeBuffer;
    }
    else if (!value.variable.name.empty())
    {
        onlyConstants = false;
        sprintf_s(baseSizeBuffer, "%s.x", value.variable.name.c_str());
        baseSize = baseSizeBuffer;
    }

    ImGui::Text("Size=(((%s+%i)*%i)/%i)+%i",
        baseSize,
        value.preAdd,
        value.multiply,
        value.divide,
        value.postAdd
    );

    if (onlyConstants)
    {
        int results = value.divide != 0
            ? ((1 + value.preAdd) * value.multiply) / value.divide + value.postAdd
            : 0;
        ImGui::Text("Size=%i", results);
    }

    return UIOverrideResult::Continue;
}

template<>
inline UIOverrideResult ShowUIOverride<TextureSizeDesc>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, TextureSizeDesc& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    if (showUIOverrideContext != ShowUIOverrideContext::Type)
        return UIOverrideResult::Continue;

    bool onlyConstants = true;
    const char* baseSize = "(1,1,1)";
    char baseSizeBuffer[256];

    if (!value.node.name.empty())
    {
        onlyConstants = false;
        sprintf_s(baseSizeBuffer, "sizeof(%s).xyz", value.node.name.c_str());
        baseSize = baseSizeBuffer;
    }
    else if (!value.variable.name.empty())
    {
        onlyConstants = false;
        sprintf_s(baseSizeBuffer, "%s.xyz", value.variable.name.c_str());
        baseSize = baseSizeBuffer;
    }

    ImGui::Text("Size=(((%s+(%i,%i,%i))*(%i,%i,%i))/(%i,%i,%i))+(%i,%i,%i)",
        baseSize,
        value.preAdd[0], value.preAdd[1], value.preAdd[2],
        value.multiply[0], value.multiply[1], value.multiply[2],
        value.divide[0], value.divide[1], value.divide[2],
        value.postAdd[0], value.postAdd[1], value.postAdd[2]
    );

    if (onlyConstants)
    {
        int results[3] = { 0, 0, 0 };
        for (int i = 0; i < 3; ++i)
            results[i] = ((1 + value.preAdd[i]) * value.multiply[i]) / value.divide[i] + value.postAdd[i];

        ImGui::Text("Size=(%i,%i,%i)", results[0], results[1], results[2]);
    }

    return UIOverrideResult::Continue;
}

template<>
inline UIOverrideResult ShowUIOverride<DispatchSizeDesc>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, DispatchSizeDesc& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    if (showUIOverrideContext != ShowUIOverrideContext::Type)
        return UIOverrideResult::Continue;

    if (!value.indirectBuffer.node.empty())
    {
        int nodeIndex = GetResourceNodeIndexForPin(renderGraph, value.indirectBuffer.node.c_str(), value.indirectBuffer.pin.c_str());
        if (nodeIndex != -1)
        {
            ImGui::Text("Dispatch=Indirect(%s)", GetNodeName(renderGraph.nodes[nodeIndex]).c_str());
            return UIOverrideResult::Continue;
        }
    }

    bool onlyConstants = true;
    const char* baseSize = "(1,1,1)";
    char baseSizeBuffer[256];

    if (!value.node.name.empty())
    {
        onlyConstants = false;
        sprintf_s(baseSizeBuffer, "sizeof(%s).xyz", value.node.name.c_str());
        baseSize = baseSizeBuffer;
    }
    else if (!value.variable.name.empty())
    {
        onlyConstants = false;
        sprintf_s(baseSizeBuffer, "%s.xyz", value.variable.name.c_str());
        baseSize = baseSizeBuffer;
    }

    ImGui::Text("Dispatch=(((%s+(%i,%i,%i))*(%i,%i,%i))/(%i,%i,%i))+(%i,%i,%i)",
        baseSize,
        value.preAdd[0], value.preAdd[1], value.preAdd[2],
        value.multiply[0], value.multiply[1], value.multiply[2],
        value.divide[0], value.divide[1], value.divide[2],
        value.postAdd[0], value.postAdd[1], value.postAdd[2]
    );

    if (onlyConstants)
    {
        int results[3] = { 0, 0, 0 };
        for (int i = 0; i < 3; ++i)
        {
            if (value.divide[i] != 0)
                results[i] = ((1 + value.preAdd[i]) * value.multiply[i]) / value.divide[i] + value.postAdd[i];
        }

        ImGui::Text("Dispatch=(%i,%i,%i)", results[0], results[1], results[2]);
    }

    return UIOverrideResult::Continue;
}

template<>
inline UIOverrideResult ShowUIOverride<RayDispatchSizeDesc>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RayDispatchSizeDesc& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    if (showUIOverrideContext != ShowUIOverrideContext::Type)
        return UIOverrideResult::Continue;

    bool onlyConstants = true;
    const char* baseSize = "(1,1,1)";
    char baseSizeBuffer[256];

    if (!value.node.name.empty())
    {
        onlyConstants = false;
        sprintf_s(baseSizeBuffer, "sizeof(%s).xyz", value.node.name.c_str());
        baseSize = baseSizeBuffer;
    }
    else if (!value.variable.name.empty())
    {
        onlyConstants = false;
        sprintf_s(baseSizeBuffer, "%s.xyz", value.variable.name.c_str());
        baseSize = baseSizeBuffer;
    }

    ImGui::Text("Dispatch=(((%s+(%i,%i,%i))*(%i,%i,%i))/(%i,%i,%i))+(%i,%i,%i)",
        baseSize,
        value.preAdd[0], value.preAdd[1], value.preAdd[2],
        value.multiply[0], value.multiply[1], value.multiply[2],
        value.divide[0], value.divide[1], value.divide[2],
        value.postAdd[0], value.postAdd[1], value.postAdd[2]
    );

    if (onlyConstants)
    {
        int results[3] = { 0, 0, 0 };
        for (int i = 0; i < 3; ++i)
            results[i] = ((1 + value.preAdd[i]) * value.multiply[i]) / value.divide[i] + value.postAdd[i];

        ImGui::Text("Dispatch=(%i,%i,%i)", results[0], results[1], results[2]);
    }

    return UIOverrideResult::Continue;
}
