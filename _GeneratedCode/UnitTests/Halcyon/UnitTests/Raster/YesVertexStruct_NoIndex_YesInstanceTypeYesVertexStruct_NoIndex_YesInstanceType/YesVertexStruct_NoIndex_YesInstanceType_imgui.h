namespace YesVertexStruct_NoIndex_YesInstanceType
{
	void MakeUI(halcyon::YesVertexStruct_NoIndex_YesInstanceTypeSettings& settings)
	{
		if (ImGui::CollapsingHeader("YesVertexStruct_NoIndex_YesInstanceTypePass Settings"))
		{
			ImGui::PushID("gigi_YesVertexStruct_NoIndex_YesInstanceType");
			{
				static const char* labels[] = {
					"Normal",
					"UV",
					"Solid",
				};
				ImGui::Combo("viewMode", (int*)&settings.viewMode, labels, 3);
			}
			ImGui::PopID();
		}
	}
}
