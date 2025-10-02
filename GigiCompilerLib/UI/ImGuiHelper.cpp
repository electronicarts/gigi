///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "ImGuiHelper.h"

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
