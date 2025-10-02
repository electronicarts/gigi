namespace VariableAliases
{
	void MakeUI(halcyon::VariableAliasesSettings& settings)
	{
		if (ImGui::CollapsingHeader("VariableAliasesPass Settings"))
		{
			ImGui::PushID("gigi_VariableAliases");
        {
            int localVarX = (int)settings.RenderSize[0];
            if(ImGui::InputInt("RenderSize.x", &localVarX))
                settings.RenderSize[0] = (unsigned int)localVarX;
            int localVarY = (int)settings.RenderSize[1];
            if(ImGui::InputInt("RenderSize.y", &localVarY))
                settings.RenderSize[1] = (unsigned int)localVarY;
        }
			ImGui::InputInt("Node1Channel", &settings.Node1Channel);
			ImGui::InputFloat("Node1Value", &settings.Node1Value);
			ImGui::InputInt("Node2Channel", &settings.Node2Channel);
			ImGui::InputFloat("Node2Value", &settings.Node2Value);
			ImGui::InputInt("Node3Channel", &settings.Node3Channel);
			ImGui::InputFloat("UnusedFloat", &settings.UnusedFloat);
			ImGui::PopID();
		}
	}
}
