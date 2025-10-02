namespace Mips_CS_2DArray
{
	void MakeUI(halcyon::Mips_CS_2DArraySettings& settings)
	{
		if (ImGui::CollapsingHeader("Mips_CS_2DArrayPass Settings"))
		{
			ImGui::PushID("gigi_Mips_CS_2DArray");
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
