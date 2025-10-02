namespace IndirectDispatch
{
	void MakeUI(halcyon::IndirectDispatchSettings& settings)
	{
		if (ImGui::CollapsingHeader("IndirectDispatchPass Settings"))
		{
			ImGui::PushID("gigi_IndirectDispatch");
        {
            int localVarX = (int)settings.Dispatch_Count_1[0];
            if(ImGui::InputInt("Dispatch_Count_1.x", &localVarX))
                settings.Dispatch_Count_1[0] = (unsigned int)localVarX;
            int localVarY = (int)settings.Dispatch_Count_1[1];
            if(ImGui::InputInt("Dispatch_Count_1.y", &localVarY))
                settings.Dispatch_Count_1[1] = (unsigned int)localVarY;
            int localVarZ = (int)settings.Dispatch_Count_1[2];
            if(ImGui::InputInt("Dispatch_Count_1.z", &localVarZ))
                settings.Dispatch_Count_1[2] = (unsigned int)localVarZ;
        }
        {
            int localVarX = (int)settings.Dispatch_Count_2[0];
            if(ImGui::InputInt("Dispatch_Count_2.x", &localVarX))
                settings.Dispatch_Count_2[0] = (unsigned int)localVarX;
            int localVarY = (int)settings.Dispatch_Count_2[1];
            if(ImGui::InputInt("Dispatch_Count_2.y", &localVarY))
                settings.Dispatch_Count_2[1] = (unsigned int)localVarY;
            int localVarZ = (int)settings.Dispatch_Count_2[2];
            if(ImGui::InputInt("Dispatch_Count_2.z", &localVarZ))
                settings.Dispatch_Count_2[2] = (unsigned int)localVarZ;
        }
			ImGui::InputInt("Second_Dispatch_Offset", &settings.Second_Dispatch_Offset);
			ImGui::PopID();
		}
	}
}
