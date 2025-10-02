namespace Mips_CS_3D
{
	void MakeUI(halcyon::Mips_CS_3DSettings& settings)
	{
		if (ImGui::CollapsingHeader("Mips_CS_3DPass Settings"))
		{
			ImGui::PushID("gigi_Mips_CS_3D");
        {
            int localVarX = (int)settings.RenderSize[0];
            if(ImGui::InputInt("RenderSize.x", &localVarX))
                settings.RenderSize[0] = (unsigned int)localVarX;
            int localVarY = (int)settings.RenderSize[1];
            if(ImGui::InputInt("RenderSize.y", &localVarY))
                settings.RenderSize[1] = (unsigned int)localVarY;
            int localVarZ = (int)settings.RenderSize[2];
            if(ImGui::InputInt("RenderSize.z", &localVarZ))
                settings.RenderSize[2] = (unsigned int)localVarZ;
        }
			ImGui::PopID();
		}
	}
}
