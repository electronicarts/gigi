namespace Mips_DrawCall
{
	void MakeUI(halcyon::Mips_DrawCallSettings& settings)
	{
		if (ImGui::CollapsingHeader("Mips_DrawCallPass Settings"))
		{
			ImGui::PushID("gigi_Mips_DrawCall");
			ImGui::Checkbox("UseMips", &settings.UseMips);
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
