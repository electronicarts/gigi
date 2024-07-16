#pragma once

#include "technique.h"

namespace simpleRT
{
    inline void ShowToolTip(const char* tooltip)
    {
        if (!tooltip || !tooltip[0])
            return;

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
        ImGui::Text("[?]");
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("%s", tooltip);
    }

    void MakeUI(Context* context, ID3D12CommandQueue* commandQueue)
    {
        ImGui::PushID("gigi_simpleRT");

        ImGui::Checkbox("enabled", &context->m_input.variable_enabled);
        ShowToolTip("");
        {
            float width = ImGui::GetContentRegionAvail().x / 5.0f;
            ImGui::PushID("hitColor");
            ImGui::PushItemWidth(width);
            ImGui::InputFloat("##X", &context->m_input.variable_hitColor[0]);
            ImGui::SameLine();
            ImGui::InputFloat("##Y", &context->m_input.variable_hitColor[1]);
            ImGui::SameLine();
            ImGui::InputFloat("##Z", &context->m_input.variable_hitColor[2]);
            ImGui::SameLine();
            ImGui::Text("hitColor");
            ImGui::PopItemWidth();
            ImGui::PopID();
            ShowToolTip("");
        }
        {
            float width = ImGui::GetContentRegionAvail().x / 5.0f;
            ImGui::PushID("missColor");
            ImGui::PushItemWidth(width);
            ImGui::InputFloat("##X", &context->m_input.variable_missColor[0]);
            ImGui::SameLine();
            ImGui::InputFloat("##Y", &context->m_input.variable_missColor[1]);
            ImGui::SameLine();
            ImGui::InputFloat("##Z", &context->m_input.variable_missColor[2]);
            ImGui::SameLine();
            ImGui::Text("missColor");
            ImGui::PopItemWidth();
            ImGui::PopID();
            ShowToolTip("");
        }

        ImGui::Checkbox("Profile", &context->m_profile);
        if (context->m_profile)
        {
            int numEntries = 0;
            const ProfileEntry* entries = context->ReadbackProfileData(commandQueue, numEntries);
            if (ImGui::BeginTable("profiling", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Label");
                ImGui::TableSetupColumn("CPU ms");
                ImGui::TableSetupColumn("GPU ms");
                ImGui::TableHeadersRow();
                float totalCpu = 0.0f;
                float totalGpu = 0.0f;
                for (int entryIndex = 0; entryIndex < numEntries; ++entryIndex)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(entries[entryIndex].m_label);
                    ImGui::TableNextColumn();
                    ImGui::Text("%0.3f", entries[entryIndex].m_cpu * 1000.0f);
                    ImGui::TableNextColumn();
                    ImGui::Text("%0.3f", entries[entryIndex].m_gpu * 1000.0f);
                    totalCpu += entries[entryIndex].m_cpu;
                    totalGpu += entries[entryIndex].m_gpu;
                }
                ImGui::EndTable();
            }
        }

        ImGui::PopID();
    }
};
