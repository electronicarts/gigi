namespace Mips_VSPS_2D
{
	void MakeUI(halcyon::Mips_VSPS_2DSettings& settings)
	{
		if (ImGui::CollapsingHeader("Mips_VSPS_2DPass Settings"))
		{
			ImGui::PushID("gigi_Mips_VSPS_2D");
        {
            int localVarX = (int)settings.RenderSize[0];
            if(ImGui::InputInt("RenderSize.x", &localVarX))
                settings.RenderSize[0] = (unsigned int)localVarX;
            int localVarY = (int)settings.RenderSize[1];
            if(ImGui::InputInt("RenderSize.y", &localVarY))
                settings.RenderSize[1] = (unsigned int)localVarY;
        }
			ImGui::PopID();
		}
	}
}
