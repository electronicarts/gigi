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

#define BASIC_INPUTBOX_WIDTH 75.0f
//#define TEXT_INPUTBOX_WIDTH 200.0f

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
        struct ShowUIEnumEntry \
        { \
            const char* name; \
            EnumType value; \
        }; \
        const ShowUIEnumEntry enumItems[] = {

#define ENUM_ITEM(_NAME, _DESCRIPTION) \
            { #_NAME, EnumType::_NAME },

#define ENUM_END() \
        }; \
        float itemWidth = 0.0f; \
        for (int enumItemIndex = 0; enumItemIndex < _countof(enumItems); ++enumItemIndex) \
            itemWidth = std::max(itemWidth, ImGui::CalcTextSize(enumItems[enumItemIndex].name).x + ImGui::GetStyle().FramePadding.x * 2.0f); \
        ImGui::SetNextItemWidth(itemWidth + ImGui::GetTextLineHeightWithSpacing() + 10); \
        if (ImGui::BeginCombo(label, labelNameOuter.c_str())) \
        { \
            for (int enumItemIndex = 0; enumItemIndex < _countof(enumItems); ++enumItemIndex) \
            { \
                std::string labelName = enumItems[enumItemIndex].name; \
                if (labelName == "Count") \
                    labelName = "<None>"; \
                bool is_selected = (std::string(enumItems[enumItemIndex].name) == EnumToString(value)); \
                if (ImGui::Selectable(labelName.c_str(), is_selected)) \
                { \
                    value = enumItems[enumItemIndex].value; \
                    ret = true; \
                } \
                if (is_selected) \
                    ImGui::SetItemDefaultFocus(); \
            } \
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
        if (ShowUIOverrideBase(renderGraph, _FLAGS, ret, PrettyLabel(#_NAME, (_FLAGS & SCHEMA_FLAG_UI_NO_PRETTY_LABEL) == 0).c_str(), _DESCRIPTION, value.##_NAME, TypePathEntry(path, TypePathEntry(#_NAME)), ShowUIOverrideContext::Field) == UIOverrideResult::Continue) \
        { \
            if (_FLAGS & SCHEMA_FLAG_UI_CONST) \
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true); \
            ret |= ShowUI(renderGraph, PrettyLabel(#_NAME, (_FLAGS & SCHEMA_FLAG_UI_NO_PRETTY_LABEL) == 0).c_str(), _DESCRIPTION, value.##_NAME, TypePathEntry(path, TypePathEntry(#_NAME))); \
            if (_FLAGS & SCHEMA_FLAG_UI_CONST) \
                ImGui::PopItemFlag(); \
        }

#define STRUCT_CONST(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS)

#define STRUCT_DYNAMIC_ARRAY(_TYPE, _NAME, _DESCRIPTION, _FLAGS) \
        if (ShowUIOverrideBase(renderGraph, _FLAGS, ret, PrettyLabel(#_NAME, (_FLAGS & SCHEMA_FLAG_UI_NO_PRETTY_LABEL) == 0).c_str(), _DESCRIPTION, value.##_NAME, TypePathEntry(path, TypePathEntry(#_NAME)), ShowUIOverrideContext::Field) == UIOverrideResult::Continue) \
            ret |= ShowUI(renderGraph, PrettyLabel(#_NAME, (_FLAGS & SCHEMA_FLAG_UI_NO_PRETTY_LABEL) == 0).c_str(), _DESCRIPTION, value._NAME, TypePathEntry(path, TypePathEntry(#_NAME)), _FLAGS); \

#define STRUCT_STATIC_ARRAY(_TYPE, _NAME, _SIZE, _DEFAULT, _DESCRIPTION, _FLAGS) \
        if (ShowUIOverrideBase(renderGraph, _FLAGS, ret, PrettyLabel(#_NAME, (_FLAGS & SCHEMA_FLAG_UI_NO_PRETTY_LABEL) == 0).c_str(), _DESCRIPTION, value.##_NAME, TypePathEntry(path, TypePathEntry(#_NAME)), ShowUIOverrideContext::Field) == UIOverrideResult::Continue) \
            ret |= ShowUI(renderGraph, PrettyLabel(#_NAME, (_FLAGS & SCHEMA_FLAG_UI_NO_PRETTY_LABEL) == 0).c_str(), _DESCRIPTION, value._NAME, TypePathEntry(path, TypePathEntry(#_NAME)), _FLAGS); \

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
            case ThisType::c_index_##_NAME: ret |= ShowUI(renderGraph, showLabel ? PrettyLabel(#_NAME, true).c_str() : nullptr, showLabel ? _DESCRIPTION : nullptr, value._NAME, TypePathEntry(path, TypePathEntry(#_NAME))); return ret;

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
    GigiAssert(false, __FUNCSIG__ ": Unsupported type encountered!");
    return false;
}

// Built in types

bool ShowUI(RenderGraph& renderGraph, const char* label, const char* tooltip, std::string& value, TypePathEntry path)
{
    char buffer[4096];
    strcpy_s(buffer, value.c_str());

    // SCHEMA_FLAG_UI_MULTILINETEXT

    //ImGui::SetNextItemWidth(TEXT_INPUTBOX_WIDTH);
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
    ImGui::SetNextItemWidth(BASIC_INPUTBOX_WIDTH);
    bool ret = ImGui::InputFloat(label, &value);
    ShowUIToolTip(tooltip);
    if (ret)
        OnUIValueChange(label, value, path);
    return ret;
}

bool ShowUI(RenderGraph& renderGraph, const char* label, const char* tooltip, int& value, TypePathEntry path)
{
    ImGui::SetNextItemWidth(BASIC_INPUTBOX_WIDTH);
    bool ret = ImGui::InputInt(label, &value, 0);
    ShowUIToolTip(tooltip);
    if (ret)
        OnUIValueChange(label, value, path);
    return ret;
}

bool ShowUI(RenderGraph& renderGraph, const char* label, const char* tooltip, unsigned int& value_, TypePathEntry path)
{
    ImGui::SetNextItemWidth(BASIC_INPUTBOX_WIDTH);
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
    ImGui::SetNextItemWidth(BASIC_INPUTBOX_WIDTH);
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
bool ShowUI(RenderGraph& renderGraph, const char* label, const char* tooltip, TSTATICARRAY<T, N>& value, TypePathEntry path, size_t _FLAGS = 0)
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
			//float width = ImGui::GetContentRegionAvail().x / float(N + 2);
			//ImGui::PushItemWidth(width);
            ImGui::SetNextItemWidth(BASIC_INPUTBOX_WIDTH);
		}

		bool showIndex = ((_FLAGS & SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX) == 0);

		if ((_FLAGS & SCHEMA_FLAG_UI_ARRAY_FATITEMS) != 0)
			ImGui::Indent();

		for (size_t index = 0; index < N; ++index)
		{
			ImGui::PushID((int)index);
			char label[256];
			sprintf_s(label, "[%i]", (int)index);
			ret |= ShowUI(renderGraph, showIndex ? label : "", tooltip, value[index], path);
			if ((_FLAGS & SCHEMA_FLAG_UI_ARRAY_FATITEMS) == 0 && index + 1 < N)
				ImGui::SameLine();
			ImGui::PopID();
		}

		if ((_FLAGS & SCHEMA_FLAG_UI_ARRAY_FATITEMS) == 0)
		{
            bool hideLabel = (!label || (label[0] == '#' && label[1] == '#'));

            if (!hideLabel)
            {
                ImGui::SameLine();
                ImGui::Text("%s", label);
                ShowUIToolTip(tooltip);
            }
		}

		if ((_FLAGS & SCHEMA_FLAG_UI_ARRAY_FATITEMS) != 0)
			ImGui::Unindent();

		if ((_FLAGS & SCHEMA_FLAG_UI_ARRAY_FATITEMS) == 0)
		{
			//ImGui::PopItemWidth();
		}
    }

    ImGui::PopID();

    if (ret)
        OnUIValueChange(label, value, path);

    return ret;
}

enum class ArrowButton2Type
{
    Arrow,
    Plus,
    Dot,
};

// copied from ImGui, the optional endMarker adds a rectangle to the triangle arrow indicating a stop
// useful to scroll to beginning or end
// Adapted to draw more than arrows
bool ArrowButton2(const char* str_id, ImGuiDir dir, bool smallButton, bool endMarker, ArrowButton2Type type = ArrowButton2Type::Arrow)
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
    ImVec2 pos;

    switch (type)
    {
        case ArrowButton2Type::Arrow:
        {
            pos = bb.Min;
            pos.x += ImMax(0.0f, (size.x - g.FontSize) * 0.5f);
            pos.y += ImMax(0.0f, (size.y - g.FontSize) * 0.5f);
            pos.x = roundf(pos.x);
            pos.y = roundf(pos.y);

            ImGui::RenderArrow(window->DrawList, pos, text_col, dir);
            break;
        }
        case ArrowButton2Type::Plus:
        {
            ImVec2 bbMid = (bb.Min + bb.Max) / 2.0f;
            pos = bbMid;

            float margin = window->DrawList->_Data->FontSize * 0.2f;

            window->DrawList->AddLine(ImVec2(bbMid.x, bb.Min.y + margin), ImVec2(bbMid.x, bb.Max.y - margin), text_col, 2.0f);
            window->DrawList->AddLine(ImVec2(bb.Min.x + margin, bbMid.y), ImVec2(bb.Max.x - margin, bbMid.y), text_col, 2.0f);
            break;
        }
        case ArrowButton2Type::Dot:
        {
            pos = (bb.Min + bb.Max) / 2.0f;

            window->DrawList->AddCircleFilled(pos, window->DrawList->_Data->FontSize * 0.20f, text_col, 8);
            break;
        }
    }

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
    bool allowResize = ((_FLAGS & SCHEMA_FLAG_UI_NO_RESIZE_ARRAY) == 0);

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

        if (showIndex)
        {
            char localLabel[256];
			sprintf_s(localLabel, "%s[%i]", label, index);
			ImGui::TextUnformatted(localLabel);
			ShowUIToolTip(tooltip);
        }

        if (allowResize)
        {
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

            if (allowResize)
            {
                ImGui::SameLine();
                if (ImGui::SmallButton("Duplicate"))
                    duplicateIndex = index;
                ShowUIToolTip("Duplicate");
            }
        }

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

    if (allowResize)
    {
        char localLabel[256];
        sprintf_s(localLabel, "Add %s[] Item", label);
        if (ImGui::Button(localLabel))
        {
            ret = true;
            TDYNAMICARRAY_RESIZE(value, TDYNAMICARRAY_SIZE(value) + 1);
        }
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
    auto ProcessLabel = [&value, &dirtyFlag](const std::string& label)
    {
        bool is_selected = value.name == label;
        std::string safeLabel = label + "##";
        if (ImGui::Selectable(safeLabel.c_str(), is_selected))
        {
            value.name = (label == " ") ? "" : label;
            dirtyFlag = true;
        }
        if (is_selected)
            ImGui::SetItemDefaultFocus();
    };

    // Texture nodes drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        std::vector<std::string> labels;

        for (const RenderGraphNode& nodeBase : renderGraph.nodes)
        {
            switch (nodeBase._index)
            {
                // Show textures
                case RenderGraphNode::c_index_resourceTexture:
                {
                    const RenderGraphNode_Resource_Texture& node = nodeBase.resourceTexture;
                    labels.push_back(node.name);
                    break;
                }
                // Show resources exported from subgraph nodes.
                // We should limit to textures if we can.
                case RenderGraphNode::c_index_actionSubGraph:
                {
                    const RenderGraphNode_Action_SubGraph& node = nodeBase.actionSubGraph;

                    if (node.loopCount > 1)
                    {
                        for (int loopIndex = 0; loopIndex < node.loopCount; ++loopIndex)
                        {
                            for (const std::string& exportedResource : node.subGraphData.exportedResources)
                            {
                                char buffer[1024];
                                sprintf_s(buffer, "%s_Iteration_%i.%s", node.name.c_str(), loopIndex, exportedResource.c_str());
                                labels.push_back(buffer);
                            }
                        }
                    }
                    else
                    {
                        for (const std::string& exportedResource : node.subGraphData.exportedResources)
                            labels.push_back(node.name + "." + exportedResource);
                    }

                    break;
                }
            }
        }

        std::sort(labels.begin(), labels.end());

        ProcessLabel(" ");
        for (const std::string& label : labels)
            ProcessLabel(label);

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

template<ShaderType TYPE, typename SHADER_REF>
inline UIOverrideResult ShowShaderReferenceUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, SHADER_REF& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ImGui::PushID(label);

    // Compute shader drop down
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
        for (int index = 0; index < renderGraph.shaders.size() + 1; ++index)
        {
            if (index > 0 && renderGraph.shaders[index - 1].type != TYPE)
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

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, WorkGraphShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowShaderReferenceUIOverride<ShaderType::WorkGraph>(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, ComputeShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowShaderReferenceUIOverride<ShaderType::Compute>(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RayGenShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowShaderReferenceUIOverride<ShaderType::RTRayGen>(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RTAnyHitShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowShaderReferenceUIOverride<ShaderType::RTAnyHit>(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RTClosestHitShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowShaderReferenceUIOverride<ShaderType::RTClosestHit>(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RTIntersectionShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowShaderReferenceUIOverride<ShaderType::RTIntersection>(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RTAnyHitShaderReferenceOptional& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowShaderReferenceUIOverride<ShaderType::RTAnyHit>(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RTClosestHitShaderReferenceOptional& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowShaderReferenceUIOverride<ShaderType::RTClosestHit>(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RTIntersectionShaderReferenceOptional& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowShaderReferenceUIOverride<ShaderType::RTIntersection>(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, VertexShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowShaderReferenceUIOverride<ShaderType::Vertex>(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, VertexShaderReferenceOptional& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowShaderReferenceUIOverride<ShaderType::Vertex>(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, PixelShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowShaderReferenceUIOverride<ShaderType::Pixel>(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, PixelShaderReferenceOptional& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowShaderReferenceUIOverride<ShaderType::Pixel>(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, AmplificationShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowShaderReferenceUIOverride<ShaderType::Amplification>(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, AmplificationShaderReferenceOptional& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowShaderReferenceUIOverride<ShaderType::Amplification>(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, MeshShaderReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowShaderReferenceUIOverride<ShaderType::Mesh>(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, MeshShaderReferenceOptional& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowShaderReferenceUIOverride<ShaderType::Mesh>(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext);
}

enum ShowUIOverride_ConstRequirement
{
    None,
    Const,
    NotConst
};

inline std::vector<std::string> GetListOfVariableNames(RenderGraph& renderGraph, ShowUIOverride_ConstRequirement constRequirement, DataFieldType dataFieldRequirement)
{
    std::vector<std::string> vars;
    for (const Variable& var : renderGraph.variables)
    {
        bool validVar = true;
        switch (constRequirement)
        {
            case ShowUIOverride_ConstRequirement::Const: validVar &= (var.Const == true); break;
            case ShowUIOverride_ConstRequirement::NotConst: validVar &= (var.Const != true); break;
        }
        if (dataFieldRequirement != DataFieldType::Count)
            validVar &= (var.type == dataFieldRequirement);
        if (!validVar)
            continue;
        vars.push_back(var.name);
    }
    CaseInsensitiveSort(vars);
    return vars;
}

template <typename TReference, typename TOnCreateVarLambda>
inline UIOverrideResult ShowUIOverride_VariableRef_Constraints(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, TReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext, const char* newVarName, ShowUIOverride_ConstRequirement constRequirement, DataFieldType dataFieldRequirement, TOnCreateVarLambda OnCreateVarLambda)
{
    ImGui::PushID(label);

    // Get a sorted list of variables
    std::vector<std::string> vars = GetListOfVariableNames(renderGraph, constRequirement, dataFieldRequirement);

    // add a blank to the beginning
    vars.insert(vars.begin(), "");

    // Get the longest text width of the server names
    float comboWidth = ImGui::CalcTextSize(value.name.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f;
    for (const std::string& name : vars)
        comboWidth = std::max(comboWidth, ImGui::CalcTextSize(name.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f);

    // Show a drop down
    ImGui::SetNextItemWidth(comboWidth + ImGui::GetTextLineHeightWithSpacing() + 10);
    if (ImGui::BeginCombo(label, value.name.c_str()))
    {
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
    if (!value.name.empty())
    {
        if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
            OnGoToVariable(value.name.c_str());
        ShowUIToolTip((std::string("Go to Variable: ") + value.name).c_str());
    }
    else
    {
        if (ArrowButton2("CreateVar", ImGuiDir_Right, true, false, ArrowButton2Type::Plus))
        {
            value.name = OnCreateVariable(newVarName, dataFieldRequirement);
            OnCreateVarLambda();
            OnGoToVariable(value.name.c_str());
            dirtyFlag = true;
        }
        ShowUIToolTip("Create New Variable");
    }

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, VariableReference& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    ShowUIOverride_ConstRequirement constRequirement = ShowUIOverride_ConstRequirement::None;
    DataFieldType dataFieldRequirement = value.UIType;

    switch (path())
    {
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_variables, TypePaths::Variable::cStruct, TypePaths::Variable::c_onUserChange)():
        {
            constRequirement = ShowUIOverride_ConstRequirement::NotConst;
            dataFieldRequirement = DataFieldType::Bool;
            break;
        }
    }

    return ShowUIOverride_VariableRef_Constraints(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext, label, constRequirement, dataFieldRequirement, [](){});
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, VariableReferenceNoConst& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowUIOverride_VariableRef_Constraints(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext, label, ShowUIOverride_ConstRequirement::NotConst, DataFieldType::Count, [](){});
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, VariableReferenceConstOnly& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return ShowUIOverride_VariableRef_Constraints(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext, label, ShowUIOverride_ConstRequirement::Const, DataFieldType::Count, [](){});
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
    node.condition.hideUI = true;
    return UIOverrideResult::Continue;
}

inline UIOverrideResult ShowUIOverride(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, ShaderResourceAccessType& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    // As of writing this, the only place ShaderResourceAccessType is used, that is displayed to the user (goes through this function) is in Shader.resources[] ShaderResource.Action

    struct AllowedTypes
    {
        ShaderResourceAccessType value;
        const char* label = nullptr;
    };

    static const AllowedTypes allowedTypes[] =
    {
        { ShaderResourceAccessType::Count, " " },
        { ShaderResourceAccessType::SRV, "Read Only" },
        { ShaderResourceAccessType::UAV, "Read/Write" },
        { ShaderResourceAccessType::RTScene, "Ray Tracing Scene" },
    };

    int allowedTypeIndex = 0;
    for (int index = 0; index < _countof(allowedTypes); ++index)
    {
        if (allowedTypes[index].value == value)
        {
            allowedTypeIndex = index;
            break;
        }
    }

    if (ImGui::BeginCombo(label, allowedTypes[allowedTypeIndex].label))
    {
        for (int index = 0; index < _countof(allowedTypes); ++index)
        {
            bool is_selected = (allowedTypeIndex == index);
            if (ImGui::Selectable(allowedTypes[index].label, is_selected))
            {
                value = allowedTypes[index].value;
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
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_nodes, TypePaths::RenderGraphNode::cVariant, TypePaths::RenderGraphNode::c_actionWorkGraph, TypePaths::RenderGraphNode_Action_WorkGraph::cStruct, TypePaths::RenderGraphNode_ActionBase::cStruct, TypePaths::RenderGraphNode_ActionBase::c_condition, TypePaths::Condition::cStruct, TypePaths::Condition::c_variable1)() :
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_nodes, TypePaths::RenderGraphNode::cVariant, TypePaths::RenderGraphNode::c_actionWorkGraph, TypePaths::RenderGraphNode_Action_WorkGraph::cStruct, TypePaths::RenderGraphNode_ActionBase::cStruct, TypePaths::RenderGraphNode_ActionBase::c_condition, TypePaths::Condition::cStruct, TypePaths::Condition::c_variable2)() :
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_nodes, TypePaths::RenderGraphNode::cVariant, TypePaths::RenderGraphNode::c_actionExternal, TypePaths::RenderGraphNode_Action_External::cStruct, TypePaths::RenderGraphNode_ActionBase::cStruct, TypePaths::RenderGraphNode_ActionBase::c_condition, TypePaths::Condition::cStruct, TypePaths::Condition::c_variable1)() :
        case TypePaths::Get(TypePaths::cEmpty, TypePaths::RenderGraph::cStruct, TypePaths::RenderGraph::c_nodes, TypePaths::RenderGraphNode::cVariant, TypePaths::RenderGraphNode::c_actionExternal, TypePaths::RenderGraphNode_Action_External::cStruct, TypePaths::RenderGraphNode_ActionBase::cStruct, TypePaths::RenderGraphNode_ActionBase::c_condition, TypePaths::Condition::cStruct, TypePaths::Condition::c_variable2)() :
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

    void Function_WorkGraph() const
    {
        fprintf(m_file,
            "[Shader(\"node\")]\n"
            "[NodeIsProgramEntry]\n"
            "[NodeLaunch(\"thread\")]\n"
        );

        fprintf(m_file, "/*$(_workgraph:%s)*/()\n{\n}\n", m_shader.entryPoint.c_str());
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
inline UIOverrideResult ShowUIOverride<Condition>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, Condition& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return value.hideUI ? UIOverrideResult::Finished : UIOverrideResult::Continue;
}

template<>
inline UIOverrideResult ShowUIOverride<ShaderResourceBuffer>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, ShaderResourceBuffer& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return value.hideUI ? UIOverrideResult::Finished : UIOverrideResult::Continue;
}

template<>
inline UIOverrideResult ShowUIOverride<ShaderResourceTexture>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, ShaderResourceTexture& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return value.hideUI ? UIOverrideResult::Finished : UIOverrideResult::Continue;
}

template<>
inline UIOverrideResult ShowUIOverride<ShaderResource>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, ShaderResource& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    // Hide UI that isn't applicable
    value.buffer.hideUI = true;
    value.texture.hideUI = true;
    switch (value.type)
    {
        case ShaderResourceType::Buffer: value.buffer.hideUI = false; break;
        case ShaderResourceType::Texture: value.texture.hideUI = false; break;
    }

    return UIOverrideResult::Continue;
}

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
            // find which index was added from the oldResources array. Note that it may have been the last one.
            int index = 0;

            while (index < value.resources.size()
                && index < oldResources.size()
                && value.resources[index].name == oldResources[index].name)
                index++;

            // make sure name is unique
            bool isUnique = false;
            while (!isUnique)
            {
                isUnique = true;
                for (const ShaderResource& old : oldResources)
                {
                    if (old.name == value.resources[index].name)
                    {
                        isUnique = false; // wasn't unique, check the version with the suffix
                        value.resources[index].name += " Copy";
                        break;
                    }
                }
            }

            OnShaderResourceAdd(value, value.resources[index].name);
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

                    // otherwise it's a rename --- don't allow duplicates
                    // // this doesn't reallydo anything except not call the callback, it
                    bool isUnique = true;
                    for (const ShaderResource& old : oldResources)
                    {
                        if (old.name == value.resources[index].name)
                        {
                            isUnique = false; // wasn't unique, check the version with the suffix
                            value.resources[index].name = oldResources[index].name;
                            break;
                        }
                    }
                    if (isUnique)
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

        ImGui::Text("Dispatch=(%i, %i, %i)", results[0], results[1], results[2]);

        if (value.shaderNumThreads[0] > 0 && value.shaderNumThreads[1] > 0 && value.shaderNumThreads[2] > 0)
        {
            int threadGroups[3];
            for (int i = 0; i < 3; ++i)
            {
                threadGroups[i] = (results[i] + value.shaderNumThreads[i] - 1) / value.shaderNumThreads[i];
            }

            ImGui::Text("ThreadGroups=(%i, %i, %i)", threadGroups[0], threadGroups[1], threadGroups[2]);

            ImGui::Text("NumThreads=(%i, %i, %i)", value.shaderNumThreads[0], value.shaderNumThreads[1], value.shaderNumThreads[2]);
        }
    }
    else
    {
        if (value.shaderNumThreads[0] > 0 && value.shaderNumThreads[1] > 0 && value.shaderNumThreads[2] > 0)
            ImGui::Text("NumThreads=(%i, %i, %i)", value.shaderNumThreads[0], value.shaderNumThreads[1], value.shaderNumThreads[2]);
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

// only show sampleCount when Texture2DMS is used
template <>
inline UIOverrideResult ShowUIOverride<Resource_Texture_MSAA>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, Resource_Texture_MSAA& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return value.hideUI ? UIOverrideResult::Finished : UIOverrideResult::Continue;
}
template <>
inline UIOverrideResult ShowUIOverride<RenderGraphNode_Resource_Texture>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RenderGraphNode_Resource_Texture& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    value.msaaSettings.hideUI = value.dimension != TextureDimensionType::Texture2DMS;
    return UIOverrideResult::Continue;
}

template <>
inline UIOverrideResult ShowUIOverride<CopyResource_BufferToBuffer>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, CopyResource_BufferToBuffer& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    return value.hideUI ? UIOverrideResult::Finished : UIOverrideResult::Continue;
}

template <>
inline UIOverrideResult ShowUIOverride<RenderGraphNode_Action_CopyResource>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RenderGraphNode_Action_CopyResource& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    value.bufferToBuffer.hideUI = false;

    int srcNodeIndex = GetResourceNodeIndexForPin(renderGraph, value.source.node.c_str(), value.source.pin.c_str());
    int destNodeIndex = GetResourceNodeIndexForPin(renderGraph, value.dest.node.c_str(), value.dest.pin.c_str());

    if (srcNodeIndex != -1 && destNodeIndex != -1)
    {
        unsigned int srcType = renderGraph.nodes[srcNodeIndex]._index;
        unsigned int destType = renderGraph.nodes[destNodeIndex]._index;

        if (srcType != RenderGraphNode::c_index_resourceBuffer || destType != RenderGraphNode::c_index_resourceBuffer)
        {
            value.bufferToBuffer.hideUI = true;
        }
    }

    return UIOverrideResult::Continue;
}

template <>
inline UIOverrideResult ShowUIOverride<RenderGraphNode_Action_ComputeShader>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RenderGraphNode_Action_ComputeShader& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    value.shaderVariableAliases.shaderIndex = GetShaderIndexByName(renderGraph, value.shader.name.c_str());
    return UIOverrideResult::Continue;
}

template <>
inline UIOverrideResult ShowUIOverride<RenderGraphNode_Action_RayShader>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RenderGraphNode_Action_RayShader& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    value.shaderVariableAliases.shaderIndex = GetShaderIndexByName(renderGraph, value.shader.name.c_str());
    return UIOverrideResult::Continue;
}

template <>
inline UIOverrideResult ShowUIOverride<RenderGraphNode_Action_DrawCall>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, RenderGraphNode_Action_DrawCall& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    value.amplificationShaderVariableAliases.shaderIndex = GetShaderIndexByName(renderGraph, value.amplificationShader.name.c_str());
    value.meshShaderVariableAliases.shaderIndex = GetShaderIndexByName(renderGraph, value.meshShader.name.c_str());
    value.vertexShaderVariableAliases.shaderIndex = GetShaderIndexByName(renderGraph, value.vertexShader.name.c_str());
    value.pixelShaderVariableAliases.shaderIndex = GetShaderIndexByName(renderGraph, value.pixelShader.name.c_str());
    return UIOverrideResult::Continue;
}

template <>
inline UIOverrideResult ShowUIOverride<ShaderVariableAliases>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, ShaderVariableAliases& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    // Prevent infinite recursion
    static bool inside = false;
    if (inside)
        return UIOverrideResult::Continue;

    if (value.shaderIndex == -1)
        return UIOverrideResult::Finished;

    const Shader& shader = renderGraph.shaders[value.shaderIndex];
    if (shader.variableAliases.empty())
        return UIOverrideResult::Finished;

    // Make a ShaderVariableAliases which is the same size and content as the shader says it should be,
    // but with the existing values mapped in where possible. Make that be the new ShaderVariableAliases.
    {
        ShaderVariableAliases mappedValues;
        mappedValues.aliases.resize(shader.variableAliases.size());
        for (int i = 0; i < shader.variableAliases.size(); ++i)
        {
            const ShaderVariableAliasDeclaration& alias = shader.variableAliases[i];
            bool found = false;
            for (const ShaderVariableAlias& existing : value.aliases)
            {
                if (existing.name == alias.name)
                {
                    mappedValues.aliases[i] = existing;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                mappedValues.aliases[i].name = alias.name;
                mappedValues.aliases[i].variable.name = "";
            }
            mappedValues.aliases[i].variable.UIType = alias.type;
        }
        value = mappedValues;
    }

    if (value.aliases.empty())
        return UIOverrideResult::Finished;

    inside = true;
    dirtyFlag |= ShowUI(renderGraph, "Variable Aliases", nullptr, value, path);
    inside = false;
    return UIOverrideResult::Finished;
}

template <typename T, typename TOnCreateVarLambda>
UIOverrideResult ShowUIOverride_ValueOrVariable(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, T& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext, DataFieldType dataFieldType, TOnCreateVarLambda OnCreateVarLambda)
{
    ImGui::PushID(label);

    // Show the value
    dirtyFlag |= ShowUI(renderGraph, "##value", tooltip, value.value, path);
    ImGui::SameLine();

    // Show the variable popup window
    {
        bool hasVariable = !value.variable.name.empty();
        ImGui::PushStyleColor(ImGuiCol_Text, hasVariable ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 255, 255, 255));
        if (ArrowButton2("Variable", ImGuiDir_Right, true, false, ArrowButton2Type::Dot))
        {
            ImGui::OpenPopup("Choose Variable");
        }

        ImGui::PopStyleColor();

        if (ImGui::BeginPopupContextItem("Choose Variable"))
        {
            // Get a sorted list of variables
            std::vector<std::string> vars = GetListOfVariableNames(renderGraph, ShowUIOverride_ConstRequirement::None, dataFieldType);

            // add a blank to the beginning
            vars.insert(vars.begin(), "");

            for (size_t labelIndex = 0; labelIndex < vars.size(); ++labelIndex)
            {
                bool checked = labelIndex > 0 && vars[labelIndex] == value.variable.name;
                ImGui::MenuItem(labelIndex == 0 ? " " : vars[labelIndex].c_str(), nullptr, &checked);
                if (checked)
                {
                    value.variable.name = vars[labelIndex];
                    dirtyFlag = true;
                }
            }

            ImGui::EndPopup();
        }

        ShowUIToolTip(hasVariable ? (std::string("Variable: ") + value.variable.name).c_str() : "Set a variable");

        if (hasVariable)
        {
            ImGui::SameLine();
            if (ArrowButton2("GoToData", ImGuiDir_Right, true, false))
                OnGoToVariable(value.variable.name.c_str());
            ShowUIToolTip((std::string("Go to Variable: ") + value.variable.name).c_str());
        }
        else
        {
            ImGui::SameLine();
            if (ArrowButton2("CreateVar", ImGuiDir_Right, true, false, ArrowButton2Type::Plus))
            {
                value.variable.name = OnCreateVariable(label, dataFieldType);
                OnCreateVarLambda();
                OnGoToVariable(value.variable.name.c_str());
                dirtyFlag = true;
            }
            ShowUIToolTip("Create New Variable");
        }
        ImGui::SameLine();
    }

    // Show the label
    ImGui::TextUnformatted(label);
    ShowUIToolTip(tooltip);

    ImGui::PopID();

    return UIOverrideResult::Finished;
}

template <>
inline UIOverrideResult ShowUIOverride<ValueOrVariable_Bool>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, ValueOrVariable_Bool& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    auto OnCreateVarLambda = [&renderGraph, &value, &tooltip]()
    {
        int variableIndex = GetVariableIndexByName(renderGraph, value.variable.name.c_str());
        renderGraph.variables[variableIndex].comment = tooltip;
    };

    UIOverrideResult ret = ShowUIOverride_ValueOrVariable(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext, DataFieldType::Bool, OnCreateVarLambda);
    if (!value.variable.name.empty())
    {
        int variableIndex = GetVariableIndexByName(renderGraph, value.variable.name.c_str());
        if (variableIndex != -1)
        {
            char defaultString[256];
            sprintf_s(defaultString, "%s", value.value ? "true" : "false");
            if (strcmp(defaultString, renderGraph.variables[variableIndex].dflt.c_str()))
            {
                dirtyFlag = true;
                renderGraph.variables[variableIndex].dflt = defaultString;
            }
        }
    }
    return ret;
}

template <>
inline UIOverrideResult ShowUIOverride<ValueOrVariable_Float>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, ValueOrVariable_Float& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    auto OnCreateVarLambda = [&renderGraph, &value, &tooltip]()
    {
        int variableIndex = GetVariableIndexByName(renderGraph, value.variable.name.c_str());
        renderGraph.variables[variableIndex].comment = tooltip;
    };

    UIOverrideResult ret = ShowUIOverride_ValueOrVariable(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext, DataFieldType::Float, OnCreateVarLambda);
    if (!value.variable.name.empty())
    {
        int variableIndex = GetVariableIndexByName(renderGraph, value.variable.name.c_str());
        if (variableIndex != -1)
        {
            char defaultString[256];
            sprintf_s(defaultString, "%f", value.value);
            if (strcmp(defaultString, renderGraph.variables[variableIndex].dflt.c_str()))
            {
                dirtyFlag = true;
                renderGraph.variables[variableIndex].dflt = defaultString;
            }
        }
    }
    return ret;
}

template <>
inline UIOverrideResult ShowUIOverride<ValueOrVariable_Float2>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, ValueOrVariable_Float2& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    auto OnCreateVarLambda = [&renderGraph, &value, &tooltip]()
    {
        int variableIndex = GetVariableIndexByName(renderGraph, value.variable.name.c_str());
        renderGraph.variables[variableIndex].comment = tooltip;
    };

    UIOverrideResult ret = ShowUIOverride_ValueOrVariable(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext, DataFieldType::Float2, OnCreateVarLambda);
    if (!value.variable.name.empty())
    {
        int variableIndex = GetVariableIndexByName(renderGraph, value.variable.name.c_str());
        if (variableIndex != -1)
        {
            char defaultString[256];
            sprintf_s(defaultString, "%f, %f", value.value[0], value.value[1]);
            if (strcmp(defaultString, renderGraph.variables[variableIndex].dflt.c_str()))
            {
                dirtyFlag = true;
                renderGraph.variables[variableIndex].dflt = defaultString;
            }
        }
    }
    return ret;
}

template <>
inline UIOverrideResult ShowUIOverride<ValueOrVariable_Float3>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, ValueOrVariable_Float3& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    auto OnCreateVarLambda = [&renderGraph, &value, &tooltip]()
    {
        int variableIndex = GetVariableIndexByName(renderGraph, value.variable.name.c_str());
        renderGraph.variables[variableIndex].comment = tooltip;
    };

    UIOverrideResult ret = ShowUIOverride_ValueOrVariable(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext, DataFieldType::Float3, OnCreateVarLambda);
    if (!value.variable.name.empty())
    {
        int variableIndex = GetVariableIndexByName(renderGraph, value.variable.name.c_str());
        if (variableIndex != -1)
        {
            char defaultString[256];
            sprintf_s(defaultString, "%f, %f, %f", value.value[0], value.value[1], value.value[2]);
            if (strcmp(defaultString, renderGraph.variables[variableIndex].dflt.c_str()))
            {
                dirtyFlag = true;
                renderGraph.variables[variableIndex].dflt = defaultString;
            }
        }
    }
    return ret;
}

template <>
inline UIOverrideResult ShowUIOverride<ValueOrVariable_Int4>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, ValueOrVariable_Int4& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    auto OnCreateVarLambda = [&renderGraph, &value, &tooltip]()
        {
            int variableIndex = GetVariableIndexByName(renderGraph, value.variable.name.c_str());
            renderGraph.variables[variableIndex].comment = tooltip;
        };

    UIOverrideResult ret = ShowUIOverride_ValueOrVariable(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext, DataFieldType::Int4, OnCreateVarLambda);
    if (!value.variable.name.empty())
    {
        int variableIndex = GetVariableIndexByName(renderGraph, value.variable.name.c_str());
        if (variableIndex != -1)
        {
            char defaultString[256];
            sprintf_s(defaultString, "%i, %i, %i, %i", value.value[0], value.value[1], value.value[2], value.value[3]);
            if (strcmp(defaultString, renderGraph.variables[variableIndex].dflt.c_str()))
            {
                dirtyFlag = true;
                renderGraph.variables[variableIndex].dflt = defaultString;
            }
        }
    }
    return ret;
}

template <>
inline UIOverrideResult ShowUIOverride<ValueOrVariable_Enum_ExternalNode_AMD_FidelityFXSDK_Upscaling_GenerateReactiveMask_ReactiveMaskMode>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, ValueOrVariable_Enum_ExternalNode_AMD_FidelityFXSDK_Upscaling_GenerateReactiveMask_ReactiveMaskMode& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    auto OnCreateVarLambda = [&renderGraph, &value, &tooltip]()
    {
        int variableIndex = GetVariableIndexByName(renderGraph, value.variable.name.c_str());
        renderGraph.variables[variableIndex].comment = tooltip;
        renderGraph.variables[variableIndex].Enum = OnCreateSystemEnum<decltype(value.value)>();
    };

    UIOverrideResult ret = ShowUIOverride_ValueOrVariable(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext, DataFieldType::Int, OnCreateVarLambda);
    if (!value.variable.name.empty())
    {
        int variableIndex = GetVariableIndexByName(renderGraph, value.variable.name.c_str());
        if (variableIndex != -1)
        {
            char defaultString[256];
            sprintf_s(defaultString, "%s", EnumToString(value.value));
            if (strcmp(defaultString, renderGraph.variables[variableIndex].dflt.c_str()))
            {
                dirtyFlag = true;
                renderGraph.variables[variableIndex].dflt = defaultString;
            }
        }
    }
    return ret;
}

template <>
inline UIOverrideResult ShowUIOverride<ValueOrVariable_Enum_ExternalNode_AMD_FidelityFXSDK_Upscaling_Version>(RenderGraph& renderGraph, uint64_t _FLAGS, bool& dirtyFlag, const char* label, const char* tooltip, ValueOrVariable_Enum_ExternalNode_AMD_FidelityFXSDK_Upscaling_Version& value, TypePathEntry path, ShowUIOverrideContext showUIOverrideContext)
{
    auto OnCreateVarLambda = [&renderGraph, &value, &tooltip]()
        {
            int variableIndex = GetVariableIndexByName(renderGraph, value.variable.name.c_str());
            renderGraph.variables[variableIndex].comment = tooltip;
            renderGraph.variables[variableIndex].Enum = OnCreateSystemEnum<decltype(value.value)>();
        };

    UIOverrideResult ret = ShowUIOverride_ValueOrVariable(renderGraph, _FLAGS, dirtyFlag, label, tooltip, value, path, showUIOverrideContext, DataFieldType::Int, OnCreateVarLambda);
    if (!value.variable.name.empty())
    {
        int variableIndex = GetVariableIndexByName(renderGraph, value.variable.name.c_str());
        if (variableIndex != -1)
        {
            char defaultString[256];
            sprintf_s(defaultString, "%s", EnumToString(value.value));
            if (strcmp(defaultString, renderGraph.variables[variableIndex].dflt.c_str()))
            {
                dirtyFlag = true;
                renderGraph.variables[variableIndex].dflt = defaultString;
            }
        }
    }
    return ret;
}
