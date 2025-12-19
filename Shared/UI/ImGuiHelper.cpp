///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "ImGuiHelper.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include <nfd.h>

bool ImGui_Checkbox(const char* label, bool* value) 
{
	//	return ImGui::Checkbox(label, value);

	ImGui::PushID(label);
	
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
	const ImVec2 frame_size = ImGui::CalcItemSize(ImVec2(0, 0), ImGui::CalcItemWidth(), 0);

	const float square_sz = ImGui::GetFrameHeight();

	ImGuiStyle& style = ImGui::GetStyle();

	// this is needed to look right with indented UI
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	float indentStart = window->DecoOuterSizeX1 + window->WindowPadding.x - window->Scroll.x;
	float indent = window->DC.Indent.x - indentStart;

	ImGui::SetCursorPosX(style.WindowPadding.x + indent + frame_size.x - square_sz);
	bool ret = ImGui::Checkbox(label, value);

	ImGui::PopID();

	return ret;
}

bool ImGui_CheckboxButton(const char* label, bool* value, ImVec4 color)
{
	// uncomment to see default UI
//	return ImGui::Checkbox(label, value);

	assert(value);

	ImGuiStyle& style = ImGui::GetStyle();

	ImVec4 black = ImVec4(0, 0, 0, 1);

	// invert Button depending on *value state
	ImVec4 colorButton = *value ? color : black;

	ImGui::PushStyleColor(ImGuiCol_Text, *value ? black : color);
	ImGui::PushStyleColor(ImGuiCol_Button, colorButton);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorButton);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorButton);
	bool ret = ImGui::Button(label);
	if(ret)
		*value = !*value;
	ImGui::PopStyleColor(4);

	return ret;
}

bool ImGui_PathFileMenuItem(const char* fileNameWithPath, int index)
{
    assert(fileNameWithPath);

    if (*fileNameWithPath == 0)
        return false;

    ImGui::PushID(index);

    const char* str = fileNameWithPath;
    // end of path, beginning of filename
    const char* end = str;

    // find last \ or /
    if (const char* here = strrchr(end, '/'))
        end = here + 1;
    if (const char* here = strrchr(end, '\\'))
        end = here + 1;

    float x = ImGui::GetCursorPosX();
    x += ImGui::GetStyle().ItemSpacing.x / 2;
    ImGui::SetCursorPosX(x + ImGui::CalcTextSize(str, end).x);  // like MenuItem() behaves, inner padding
    bool selected = false;
    bool activated = ImGui::Selectable(end, &selected, ImGuiSelectableFlags_SpanAllColumns);
    bool hovered = ImGui::IsItemHovered();
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(ImGui::GetStyle().ItemSpacing.x / 2, 0.0f));  // like MenuItem() behaves, inner padding
    ImGui::SameLine();

    if (!hovered)
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 50));

    ImGui::SetCursorPosX(x);
    ImGui::TextEx(str, end);
    if (!hovered)
        ImGui::PopStyleColor();

    ImGui::PopID();

    return activated;
}

bool ImGui_FilePathMenuItem(const char* fileNameWithPath, int index)
{
    assert(fileNameWithPath);

    if (*fileNameWithPath == 0)
        return false;

    ImGui::PushID(index);

    const char* str = fileNameWithPath;
    // end of path, beginning of filename
    const char* end = str;

    // find last \ or /
    if (const char* here = strrchr(end, '/'))
        end = here + 1;
    if (const char* here = strrchr(end, '\\'))
        end = here + 1;

    bool selected = false;
    bool activated = ImGui::Selectable(end, &selected, ImGuiSelectableFlags_SpanAllColumns);
    bool hovered = ImGui::IsItemHovered();
    ImGui::SameLine();

    if (!hovered)
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 50));

    // omit last \ or /
    if (end != str)
        --end;

    ImGui::TextEx(str, end);
    if (!hovered)
        ImGui::PopStyleColor();

    ImGui::PopID();

    return activated;
}

bool ImGui_File(const char* label, std::string& inOutName, const char* filterList)
{
	ImGui::PushID(label);

	char fileName[4096];
	strcpy_s(fileName, inOutName.c_str());

	const char* buttonText = "..";

	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
	const ImVec2 frame_size = ImGui::CalcItemSize(ImVec2(0, 0), ImGui::CalcItemWidth(), 0);
	float ButtonWidth = ImGui::CalcTextSize(buttonText).x + ImGui::GetStyle().FramePadding.x * 2;
	float gap = 2;

	ImGui::SetNextItemWidth(frame_size.x - ButtonWidth - gap);

	bool ret = false;

	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(gap, 0));

		ret = ImGui::InputText("", fileName, 4096);

		if (ret)
			inOutName = fileName;

		ImGui::SameLine();

		if (ImGui::Button(buttonText)) {
			nfdchar_t* outPath = nullptr;
			if (NFD_OpenDialog(filterList, "", &outPath) == NFD_OKAY)
			{
				inOutName = outPath;
				ret = true;
			}
		}
		ImGui::PopStyleVar();
	}

	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemInnerSpacing.x, ImGui::GetStyle().ItemInnerSpacing.y));
		ImGui::SameLine();
		ImGui::TextUnformatted(label);
		ImGui::PopStyleVar();
	}

	ImGui::PopID();

	return ret;
}

bool ImGuiIconButton(const char* label, const char* icon)
{
    // classic BeginMenu in comparison
//    return ImGui::Button(label);

    ImVec4 hideColor = ImVec4(0, 0, 0, 0);

    ImGui::PushStyleColor(ImGuiCol_Border, hideColor);
    ImGui::PushStyleColor(ImGuiCol_BorderShadow, hideColor);
    ImGui::PushStyleColor(ImGuiCol_Button, hideColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, hideColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hideColor);
    bool ret = ImGui::Button(icon);
    ImGui::PopStyleColor(5);

    return ret;
}

bool ImGuiMenuItem(const char* label, const char* icon, const char* shortcut, bool* p_checked, bool enabled)
{
    // classic MenuItem in comparison
//	return ImGui::MenuItem(label, shortcut, p_checked, enabled);

    ImGui::PushID(label);

    // useful for tooltips
    ImGui::BeginGroup();

    ImVec4 hideColor = ImVec4(0, 0, 0, 0);
    ImVec4 disabledColor = ImGui::GetStyle().Colors[ImGuiCol_TextDisabled];
    ImVec4 normalColor = ImGui::GetStyle().Colors[ImGuiCol_Text];

    if (!enabled)
        ImGui::PushStyleColor(ImGuiCol_Text, disabledColor);

    const char* checkIcon = "\xef\x80\x8c";

    ImVec4 color = enabled ? normalColor : disabledColor;

    // to be consistent
    if (p_checked)
        icon = 0;

    bool ret = false;

    ImGui::PushStyleColor(ImGuiCol_Border, hideColor);
    ImGui::PushStyleColor(ImGuiCol_BorderShadow, hideColor);
    if (p_checked)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, *p_checked ? color : hideColor);

        if (ImGui::SmallButton(checkIcon) && enabled)
            *p_checked = !*p_checked;

        ImGui::PopStyleColor();

        ImGui::SameLine();
    }
    else if (icon)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::PushStyleColor(ImGuiCol_Button, hideColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, hideColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hideColor);

        if (ImGui::SmallButton(icon))
            ret = true;

        ImGui::PopStyleColor(4);
        ImGui::SameLine();
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Text, hideColor);
        ImGui::PushStyleColor(ImGuiCol_Button, hideColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, hideColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hideColor);
        ImGui::SmallButton(checkIcon);
        ImGui::PopStyleColor(4);
        ImGui::SameLine();
    }
    ImGui::PopStyleColor(2);

    // Use Selectable for the main logic of a menu item
    // Pass 'p_checked' state to ImGui::Selectable to handle selection logic, 
    // but we won't let it draw the built-in checkmark (by manually handling the bool state)
    bool selected = false;

    ImGuiSelectableFlags flags = ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_DontClosePopups;

    //    if (!icon)
    //    flags |= ImGuiSelectableFlags_SpanAllColumns;

    if (ImGui::Selectable(label, &selected, flags))
    {
        // Toggle the checkbox state when the item is clicked
        if (p_checked)
            *p_checked = !*p_checked;
        ret = true;
    }

    // uncomment to test
//	shortcut = "ABCDEF";

    if (shortcut)
    {
        ImGui::SameLine();
        // see https://github.com/ocornut/imgui/issues/7805
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImMax(0.0f, ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(shortcut).x));
        ImGui::TextDisabled("%s", shortcut);
    }

    if (!enabled)
        ImGui::PopStyleColor();

    ImGui::EndGroup();

    ImGui::PopID();

    return ret;
}

bool ImGuiBeginMenu(const char* label, bool enabled)
{
    // classic BeginMenu in comparison
//    return ImGui::BeginMenu(label, enabled);

    ImVec4 hideColor = ImVec4(0, 0, 0, 0);

    const char* checkIcon = "\xef\x80\x8c";

    ImGui::PushStyleColor(ImGuiCol_Border, hideColor);
    ImGui::PushStyleColor(ImGuiCol_BorderShadow, hideColor);
    ImGui::PushStyleColor(ImGuiCol_Text, hideColor);
    ImGui::PushStyleColor(ImGuiCol_Button, hideColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, hideColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hideColor);
    ImGui::SmallButton(checkIcon);
    ImGui::PopStyleColor(6);

    ImGui::SameLine();

    return ImGui::BeginMenu(label, enabled);
}
