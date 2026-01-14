///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

# include "widgets.h"
# define IMGUI_DEFINE_MATH_OPERATORS
# include <imgui_internal.h>


#include "imgui_stdlib.h"
#include "commonUI.h"

std::string FromWideStringUTF8(const wchar_t* string);

void ax::Widgets::Icon(const ImVec2& size, IconType type, bool filled, const ImVec4& color/* = ImVec4(1, 1, 1, 1)*/, const ImVec4& innerColor/* = ImVec4(0, 0, 0, 0)*/)
{
    if (ImGui::IsRectVisible(size))
    {
        auto cursorPos = ImGui::GetCursorScreenPos();
        auto drawList  = ImGui::GetWindowDrawList();
        ax::Drawing::DrawIcon(drawList, cursorPos, cursorPos + size, type, filled, ImColor(color), ImColor(innerColor));
    }

    ImGui::Dummy(size);
}


bool ax::Widgets::ImGuiSearch(const char* label, std::string* value, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    assert(value);

    const char* lensIcon = "\xef\x80\x82";

    // label, show if it does not start with '##'
    bool showLabel = label[0] == 0 || label[1] == 0 || !(label[0] == '#' && label[1] == '#');

    ImGuiStyle& style = ImGui::GetStyle();

    float iconWidth = ImGui::CalcTextSize(lensIcon).x + ImGui::GetTextLineHeight();
    float leftDecoWithoutIcon = ImGui::CalcTextSize(lensIcon).x + ImGui::GetTextLineHeight();
    float rightDeco = ImGui::CalcTextSize(label).x + style.ItemSpacing.x;

    // label should not to close to the border
    rightDeco += style.ItemSpacing.x;

    if (!showLabel)
        rightDeco = 0;

    // width of the usable TextInput part
    float buttonWidth = ImGui::GetContentRegionAvail().x - leftDecoWithoutIcon - ImGui::GetTextLineHeight() - rightDeco;

    float x = ImGui::GetCursorPosX();

    ImGuiWindow* window = ImGui::GetCurrentWindow();

    {
        ImVec2 size(buttonWidth + iconWidth + ImGui::GetTextLineHeight(), ImGui::GetFontSize() + 2 * style.FramePadding.y);

        const ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + size.x, window->DC.CursorPos.y + size.y));
        const ImU32 col = ImGui::GetColorU32(ImGuiCol_FrameBg);
        ImGui::RenderFrame(bb.Min, bb.Max, col, true, 100.0f);
    }

    bool ret;
    {
        ImStyleColor_RAII seeThrough;

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));

        ImGui::SameLine();
        ImGui::SetCursorPosX(x + leftDecoWithoutIcon);
        ImGui::PushItemWidth(buttonWidth);
        ImGui::PushID(label);
        ret = ImGui::InputText("##", value, flags, callback, user_data);
        ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::PopStyleVar();
    }

    // icon
    ImGui::SameLine();
    ImGui::SetCursorPosX(x + ImGui::GetFontSize() * 0.5f);
    ImGui::TextUnformatted(lensIcon);

    if(showLabel)
    {
        ImGui::SameLine();
        ImGui::SetCursorPosX(x + buttonWidth + ImGui::CalcTextSize(lensIcon).x + 2 * ImGui::GetTextLineHeight() + style.ItemSpacing.x);
        ImGui::TextUnformatted(label);
    }

    return ret;
}

void BeginTooltip()
{
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.9f, 0.9f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
    ImGui::BeginTooltip();
}

void EndTooltip()
{
    ImGui::EndTooltip();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

void ax::Widgets::ImGuiShowIconsWindow(ImFont *font, bool &show)
{
    if(!show)
        return;

    int IconsPerLine = 8;

    ImGui::SetNextWindowSizeConstraints(ImVec2(500, 300), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::SetNextWindowSize(ImVec2(500, 680), ImGuiCond_FirstUseEver);

    if(!ImGui::Begin("Icons", &show))
        return;

    static std::string characterToShow;
    static std::string literalToShow;

    float fontWidth = 0;
    {
        for (unsigned int base = 0; base <= IM_UNICODE_CODEPOINT_MAX; base += 256)
        {
            // optimization
            if (!(base & 4095) && font->IsGlyphRangeUnused(base, base + 4095))
            {
                base += 4096 - 256;
                continue;
            }
            for (unsigned int n = 0; n < 256; n++)
            {
                const ImFontGlyph* glyph = font->FindGlyphNoFallback((ImWchar)(base + n));
                if (glyph)
                {
                    fontWidth = std::max(fontWidth, glyph->X1 - glyph->X0);
                }
            }
        }
    }

    float iconHeight;
    {
        ImGui::PushFont(font);
        ImGui::Text("%s", characterToShow.c_str());
        iconHeight = ImGui::GetTextLineHeight();
        ImGui::PopFont();
        ImGui::Text("%s", literalToShow.c_str());
        ImGui::Separator();
    }

    const float padding = 2;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, padding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImVec2 itemSpacing = ImGui::GetStyle().ItemSpacing;

//    const ImU32 glyph_col = ImGui::GetColorU32(ImGuiCol_Text);
    const float cell_size = fontWidth; //font->FontSize * 1;

    if (ImGui::BeginChild("ScrollReg", ImVec2((cell_size + itemSpacing.x) * IconsPerLine + ImGui::GetStyle().ScrollbarSize, 0), false))
    {
        int printedCharId = 0;
        for (unsigned int base = 0; base <= IM_UNICODE_CODEPOINT_MAX; base += 256)
        {
            // Skip ahead if a large bunch of glyphs are not present in the font (test in chunks of 4k)
            // This is only a small optimization to reduce the number of iterations when IM_UNICODE_MAX_CODEPOINT
            // is large // (if ImWchar==ImWchar32 we will do at least about 272 queries here)
            if (!(base & 4095) && font->IsGlyphRangeUnused(base, base + 4095))
            {
                base += 4096 - 256;
                continue;
            }

            for (unsigned int n = 0; n < 256; n++)
            {
                const ImFontGlyph* glyph = font->FindGlyphNoFallback((ImWchar)(base + n));
                if (glyph && glyph->Visible)
                {
                    ImGui::SetCursorPosX((printedCharId % IconsPerLine) * (cell_size + itemSpacing.x));
                    
                    // static to avoid memory allocations
                    static std::wstring wstr;
                    wstr.clear();
                    wstr.push_back((wchar_t)(base + n));
                    ImGui::PushID(n);
                    // inefficient (memory allocation) but simple
                    ImGui::PushFont(font);
                    ImGui::Button(FromWideStringUTF8(wstr.c_str()).c_str(), ImVec2(cell_size, 0));
                    ImGui::PopFont();
                    ImGui::PopID();

                    printedCharId++;
                    if (printedCharId % IconsPerLine)
                        ImGui::SameLine();

                    if(ImGui::IsItemActive())
                    {
                        literalToShow.clear();
                        // e.g. "\xef\x80\x81" = U+f001
                        //std::wstring wstr;
                        //wstr.push_back((wchar_t)(base + n));
                        characterToShow = FromWideStringUTF8(wstr.c_str());
                        const char* p = characterToShow.c_str();
                        literalToShow += "\"";
                        while(*p)
                        {
                            char str[8];
                            sprintf_s(str, sizeof(str), "\\x%hhx", *p++);
                            literalToShow += str;
                        }
                        literalToShow += "\"";

                        ImGui::LogToClipboard();
                        ImGui::LogText("%s", literalToShow.c_str());
                        ImGui::LogFinish();
                    }
                    if (ImGui::IsItemHovered())
                    {
                        BeginTooltip();
                        ImGui::Text("Codepoint: U+%04X", base + n);
                        EndTooltip();
                    }

                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::PopStyleVar(2);
    ImGui::SameLine();
    ImGui::TextUnformatted("cursor up/down/space or click on a character to copy it into the clipboard");

    ImGui::End();
}


