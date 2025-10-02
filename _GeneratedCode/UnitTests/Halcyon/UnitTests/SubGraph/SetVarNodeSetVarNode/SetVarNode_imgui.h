namespace SetVarNode
{
	void MakeUI(halcyon::SetVarNodeSettings& settings)
	{
		if (ImGui::CollapsingHeader("SetVarNodePass Settings"))
		{
			ImGui::PushID("gigi_SetVarNode");
        {
            int localVar = (int)settings.Sub_BufferSize;
            if(ImGui::InputInt("Sub_BufferSize", &localVar))
                settings.Sub_BufferSize = (unsigned int)localVar;
        }
        {
            int localVarX = (int)settings.Sub_TextureSize[0];
            if(ImGui::InputInt("Sub_TextureSize.x", &localVarX))
                settings.Sub_TextureSize[0] = (unsigned int)localVarX;
            int localVarY = (int)settings.Sub_TextureSize[1];
            if(ImGui::InputInt("Sub_TextureSize.y", &localVarY))
                settings.Sub_TextureSize[1] = (unsigned int)localVarY;
            int localVarZ = (int)settings.Sub_TextureSize[2];
            if(ImGui::InputInt("Sub_TextureSize.z", &localVarZ))
                settings.Sub_TextureSize[2] = (unsigned int)localVarZ;
        }
			ImGui::PopID();
		}
	}
}
