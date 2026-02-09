///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "ImGuiHelper.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include <nfd.h>

ImGuiEnable::ImGuiEnable(bool enabled)
    : m_enabled(enabled)
{
    if (!m_enabled)
    {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.25f);
    }
}

ImGuiEnable::~ImGuiEnable()
{
    if (!m_enabled)
    {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }
}

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

/*
int ImGui_PathFileMenuItem(const char* fileNameWithPath, int index)
{
    assert(fileNameWithPath);

    if (*fileNameWithPath == 0)
        return false;

    ImGui::PushID(index);
    ImGui::BeginGroup();

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
    //    int ret = ImGui::Selectable(end, &selected, ImGuiSelectableFlags_SpanAllColumns);
    int ret = ImGui::Selectable(end, &selected, ImGuiSelectableFlags_AllowItemOverlap) ? 1 : 0;
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

    //        const char* icon = "\xef\x81\x97"; // disk with cross inside
    //        const char* icon = "\xef\x80\x8d"; // cross
    const char* icon = "\xef\x80\x94"; // trashcan

    // see https://github.com/ocornut/imgui/issues/7805
    float button_width = ImGui::CalcTextSize(icon).x; // Button text + style padding
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(button_width, 0));

    {
        ImGui::SameLine();
        float x = ImGui::GetContentRegionMax().x - button_width - ImGui::GetStyle().ItemSpacing.x;

        ImGui::SetCursorPosX(x);
        if (ImGuiIconButton("Delete", icon, true, true))
            ret = -1;
    }

    ImGui::EndGroup();
    ImGui::PopID();

    return activated;
}
*/

int ImGui_FilePathMenuItem(const char* fileNameWithPath, int index)
{
    assert(fileNameWithPath);

    if (*fileNameWithPath == 0)
        return false;

    ImGui::PushID(index);
    ImGui::BeginGroup();

    const char* str = fileNameWithPath;
    // end of path, beginning of filename
    const char* end = str;

    // find last \ or /
    if (const char* here = strrchr(end, '/'))
        end = here + 1;
    if (const char* here = strrchr(end, '\\'))
        end = here + 1;

    bool selected = false;
    // flag is needed to make delete icon behind it working

    const float menuWidth = ImGui::GetFontSize() * 30;
//    const float menuWidth = 400;

    // BeginChild is needed to not highlight the delete icon, sadly it clips characters at the bottom
//    ImGui::BeginChild("##", ImVec2(menuWidth, ImGui::GetFontSize()), false, ImGuiWindowFlags_NoBackground);
    // BeginChild is needed to not highlight the delete icon
    ImGui::BeginChild("##", ImVec2(menuWidth, ImGui::GetFontSize() + 2 * 1), false, ImGuiWindowFlags_NoBackground);

    // Selectable in menu has no FramePadding so we fix this
    ImGui::SetCursorPosX(ImGui::GetStyle().FramePadding.x);

    const char* iconDoc = "\xef\x85\x9b"; // Icon: filled document, right top corner fold

    ImGui::TextUnformatted(iconDoc);
    ImGui::SameLine();

    // file name
    int ret = ImGui::Selectable(end, &selected, ImGuiSelectableFlags_None) ? 1 : 0;
    bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 50));

    // omit last \ or /
    if (end != str)
        --end;

    float fileNameWidth = ImGui::CalcTextSize(iconDoc).x + ImGui::GetStyle().ItemSpacing.x + ImGui::CalcTextSize(end).x;
    ImGui::SetCursorPosX(fileNameWidth + ImGui::GetStyle().ItemSpacing.x);

   const char* iconDelete = "\xef\x81\x97"; // disk with cross inside
//        const char* iconDelete = "\xef\x80\x8d"; // cross
//    const char* iconDelete = "\xef\x80\x94"; // trashcan  too busy when small


    // path, clipped in rectangle, ideally we show "...", maybe right side
    ImVec2 clipSize = ImVec2(menuWidth - fileNameWidth - ImGui::GetStyle().WindowPadding.x - 2 * ImGui::GetStyle().FramePadding.x, ImGui::GetFontSize());
    {
        ImVec2 clip_min = ImGui::GetCursorScreenPos();
        ImVec2 clip_max = ImVec2(clip_min.x + clipSize.x, clip_min.y + clipSize.y);

        // Push the clip rect. 
        // Set the third parameter to 'true' to intersect with the current clip rect.
        ImGui::PushClipRect(clip_min, clip_max, true);

        ImGui::TextEx(str, end);

        ImGui::PopClipRect();
    }


    ImGui::PopStyleColor();

    ImGui::EndChild();

    {
        ImGui::SameLine();

        ImGui::SetCursorPosX(menuWidth + ImGui::GetStyle().WindowPadding.x + ImGui::GetStyle().ItemSpacing.x);

        if (ImGuiIconButton("Delete", iconDelete))
            ret = -1;
    }


    ImGui::EndGroup();
    ImGui::PopID();

    return ret;
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
//    return ImGui::SmallButton(label);

    // we need a InvisibleButton to know in advance if we are hovered
    float backup = ImGui::GetCursorPosX();
    bool ret = ImGui::InvisibleButton(label, ImGui::CalcTextSize(icon));
    ImGui::SameLine();
    ImGui::SetCursorPosX(backup);
    bool hovered = ImGui::IsItemHovered();

    ImVec4 hideColor = ImVec4(0, 0, 0, 0);
    ImVec4 textColor = ImGui::GetStyle().Colors[ImGuiCol_Text];

    ImGui::PushStyleColor(ImGuiCol_Text, hovered ? textColor : ImVec4(textColor.x, textColor.y, textColor.z, textColor.w * 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Border, hideColor);
    ImGui::PushStyleColor(ImGuiCol_BorderShadow, hideColor);
    ImGui::PushStyleColor(ImGuiCol_Button, hideColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, hideColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hideColor);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));  // make icon square
    ImGui::TextUnformatted(icon);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(6);

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
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));  // make icon square
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
    ImGui::PopStyleVar();
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

void ImGuiRightAlign(const char* text)
{
    assert(text);

    // 1. Calculate the position of the right edge of the current column
    float rightEdge = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x;

    // 2. Calculate the width of the text to be displayed
    float textWidth = ImGui::CalcTextSize(text).x;

    // 3. Calculate the new X position for the text to be right-aligned
    // Subtract text width from the right edge. Adjust for any desired padding (e.g., style.ItemSpacing.x) if needed.
    float posX = rightEdge - textWidth - ImGui::GetStyle().ItemSpacing.x;

    // Optional: Ensure the text doesn't overlap with the left side if the column is too narrow
    if (posX > ImGui::GetCursorPosX())
        ImGui::SetCursorPosX(posX);
}

void ImGuiKeyValueString(const char* key, const char* value)
{
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.7f), "%s = ", key);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 1.0f), "\"%s\"", value);
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
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));  // make icon square
    ImGui::SmallButton(checkIcon);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(6);

    ImGui::SameLine();

    return ImGui::BeginMenu(label, enabled);
}
