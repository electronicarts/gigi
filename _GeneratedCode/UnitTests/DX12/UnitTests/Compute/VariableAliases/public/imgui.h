#pragma once

#include "technique.h"

namespace VariableAliases
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
        ImGui::PushID("gigi_VariableAliases");

        {
            float width = ImGui::GetContentRegionAvail().x / 4.0f;
            ImGui::PushID("RenderSize");
            ImGui::PushItemWidth(width);
            int localVarX = (int)context->m_input.variable_RenderSize[0];
            if(ImGui::InputInt("##X", &localVarX, 0))
                context->m_input.variable_RenderSize[0] = (unsigned int)localVarX;
            ImGui::SameLine();
            int localVarY = (int)context->m_input.variable_RenderSize[1];
            if(ImGui::InputInt("##Y", &localVarY, 0))
                context->m_input.variable_RenderSize[1] = (unsigned int)localVarY;
            ImGui::SameLine();
            ImGui::Text("RenderSize");
            ImGui::PopItemWidth();
            ImGui::PopID();
        }
        ImGui::InputInt("Node1Channel", &context->m_input.variable_Node1Channel, 0);
        ImGui::InputFloat("Node1Value", &context->m_input.variable_Node1Value);
        ImGui::InputInt("Node2Channel", &context->m_input.variable_Node2Channel, 0);
        ImGui::InputFloat("Node2Value", &context->m_input.variable_Node2Value);
        ImGui::InputInt("Node3Channel", &context->m_input.variable_Node3Channel, 0);
        ImGui::InputFloat("UnusedFloat", &context->m_input.variable_UnusedFloat);
        ShowToolTip("This is for the unused alias in the shader");

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
