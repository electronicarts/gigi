///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

// clang-format off
#include "GigiInterpreterPreviewWindowDX12.h"
#include "ImGuiHelper.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <unordered_set>
// clang-format on

// needed to make ImGui::Text() align with UI that is larger because of FarmePadding
void ShowUI_StartGap()
{
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y);
}

static void ShowToolTip(const char* tooltip)
{
	if (!tooltip || !tooltip[0])
		return;

	// ImGui::SameLine();
	// ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[?]");
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("%s", tooltip);
}

static bool ShowUI_Int(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
    bool ret = false;
	if (variable.enumIndex != -1)
	{
		const Enum& e = renderGraph.enums[variable.enumIndex];

		std::vector<const char*> labels;
		for (const EnumItem& item : e.items)
			labels.push_back(item.displayLabel.c_str());

		ret = ImGui::Combo(variable.originalName.c_str(), (int*)storage, labels.data(), (int)labels.size());
	}
	else
	{
		if (variable.UISettings.UIHint == VariableUIHint::Drag)
		{
			float speed = 1.0f;
			int	  min	= 0;
			int	  max	= 0;

			if (!variable.UISettings.step.empty())
				sscanf_s(variable.UISettings.step.c_str(), "%f", &speed);

			if (!variable.UISettings.min.empty())
				sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

			if (!variable.UISettings.max.empty())
				sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

            ret = ImGui::DragInt(variable.originalName.c_str(), (int*)storage, speed, min, max);
		}
		else if (variable.UISettings.UIHint == VariableUIHint::Slider)
		{
			int min = 0;
			int max = 65535;

			if (!variable.UISettings.min.empty())
				sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

			if (!variable.UISettings.max.empty())
				sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

            ret = ImGui::SliderInt(variable.originalName.c_str(), (int*)storage, min, max);
		}
		else
            ret = ImGui::InputInt(variable.originalName.c_str(), (int*)storage);
	}
    return ret;
}

static bool ShowUI_Int2(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
    bool ret = false;
	if (variable.UISettings.UIHint == VariableUIHint::Drag)
	{
		float speed = 1.0f;
		int	  min	= 0;
		int	  max	= 0;

		if (!variable.UISettings.step.empty())
			sscanf_s(variable.UISettings.step.c_str(), "%f", &speed);

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

        ret = ImGui::DragInt2(variable.originalName.c_str(), (int*)storage, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

        ret = ImGui::SliderInt2(variable.originalName.c_str(), (int*)storage, min, max);
	}
	else
		ret = ImGui::InputInt2(variable.originalName.c_str(), (int*)storage);
    return ret;
}

static bool ShowUI_Int3(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
    bool ret = false;
	if (variable.UISettings.UIHint == VariableUIHint::Drag)
	{
		float speed = 1.0f;
		int	  min	= 0;
		int	  max	= 0;

		if (!variable.UISettings.step.empty())
			sscanf_s(variable.UISettings.step.c_str(), "%f", &speed);

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

        ret = ImGui::DragInt3(variable.originalName.c_str(), (int*)storage, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

        ret = ImGui::SliderInt3(variable.originalName.c_str(), (int*)storage, min, max);
	}
	else
		ret = ImGui::InputInt3(variable.originalName.c_str(), (int*)storage);
    return ret;
}

static bool ShowUI_Int4(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
    bool ret = false;
	if (variable.UISettings.UIHint == VariableUIHint::Drag)
	{
		float speed = 1.0f;
		int	  min	= 0;
		int	  max	= 0;

		if (!variable.UISettings.step.empty())
			sscanf_s(variable.UISettings.step.c_str(), "%f", &speed);

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

        ret = ImGui::DragInt4(variable.originalName.c_str(), (int*)storage, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

        ret = ImGui::SliderInt4(variable.originalName.c_str(), (int*)storage, min, max);
	}
	else
        ret = ImGui::InputInt4(variable.originalName.c_str(), (int*)storage);
    return ret;
}

static bool ShowUI_Uint(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
    bool ret = false;
	unsigned int* v2  = (unsigned int*)storage;
	int			  v[] = { (int)v2[0] };

	if (variable.UISettings.UIHint == VariableUIHint::Drag)
	{
		float speed = 1.0f;
		int	  min	= 0;
		int	  max	= 0;

		if (!variable.UISettings.step.empty())
			sscanf_s(variable.UISettings.step.c_str(), "%f", &speed);

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

		ret = ImGui::DragInt(variable.originalName.c_str(), v, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

        ret = ImGui::SliderInt(variable.originalName.c_str(), v, min, max);
	}
	else
        ret = ImGui::InputInt(variable.originalName.c_str(), v);

	v2[0] = v[0];
    return ret;
}

static bool ShowUI_Uint2(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
    bool ret = false;
	unsigned int* v2  = (unsigned int*)storage;
	int			  v[] = { (int)v2[0], (int)v2[1] };

	if (variable.UISettings.UIHint == VariableUIHint::Drag)
	{
		float speed = 1.0f;
		int	  min	= 0;
		int	  max	= 0;

		if (!variable.UISettings.step.empty())
			sscanf_s(variable.UISettings.step.c_str(), "%f", &speed);

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

        ret = ImGui::DragInt2(variable.originalName.c_str(), v, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

        ret = ImGui::SliderInt2(variable.originalName.c_str(), v, min, max);
	}
	else
        ret = ImGui::InputInt2(variable.originalName.c_str(), v);

	v2[0] = v[0];
	v2[1] = v[1];
    return ret;
}

static bool ShowUI_Uint3(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
    bool ret = false;
	unsigned int* v2  = (unsigned int*)storage;
	int			  v[] = { (int)v2[0], (int)v2[1], (int)v2[2] };

	if (variable.UISettings.UIHint == VariableUIHint::Drag)
	{
		float speed = 1.0f;
		int	  min	= 0;
		int	  max	= 0;

		if (!variable.UISettings.step.empty())
			sscanf_s(variable.UISettings.step.c_str(), "%f", &speed);

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

        ret = ImGui::DragInt3(variable.originalName.c_str(), v, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

        ret = ImGui::SliderInt3(variable.originalName.c_str(), v, min, max);
	}
	else
        ret = ImGui::InputInt3(variable.originalName.c_str(), v);

	v2[0] = v[0];
	v2[1] = v[1];
	v2[2] = v[2];
    return ret;
}

static bool ShowUI_Uint4(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
    bool ret = false;
	unsigned int* v2  = (unsigned int*)storage;
	int			  v[] = { (int)v2[0], (int)v2[1], (int)v2[2], (int)v2[3] };

	if (variable.UISettings.UIHint == VariableUIHint::Drag)
	{
		float speed = 1.0f;
		int	  min	= 0;
		int	  max	= 0;

		if (!variable.UISettings.step.empty())
			sscanf_s(variable.UISettings.step.c_str(), "%f", &speed);

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

        ret = ImGui::DragInt4(variable.originalName.c_str(), v, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

        ret = ImGui::SliderInt4(variable.originalName.c_str(), v, min, max);
	}
	else
		ret = ImGui::InputInt4(variable.originalName.c_str(), v);

	v2[0] = v[0];
	v2[1] = v[1];
	v2[2] = v[2];
	v2[3] = v[3];
    return ret;
}

static bool ShowUI_Float(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
    bool ret = false;
	if (variable.UISettings.UIHint == VariableUIHint::Angle)
	{
        ret = ImGui::SliderAngle(variable.originalName.c_str(), (float*)storage);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Drag)
	{
		float speed = 1.0f;
		float min	= 0.0f;
		float max	= 0.0f;

		if (!variable.UISettings.step.empty())
			sscanf_s(variable.UISettings.step.c_str(), "%f", &speed);

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%f", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%f", &max);

        ret = ImGui::DragFloat(variable.originalName.c_str(), (float*)storage, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		float min = 0.0f;
		float max = 1.0f;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%f", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%f", &max);

        ret = ImGui::SliderFloat(variable.originalName.c_str(), (float*)storage, min, max);
	}
	else
	{
        ret = ImGui::InputFloat(variable.originalName.c_str(), (float*)storage);
	}
    return ret;
}

static bool ShowUI_Float2(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
    bool ret = false;
	if (variable.UISettings.UIHint == VariableUIHint::Drag)
	{
		float speed = 1.0f;
		float min	= 0.0f;
		float max	= 0.0f;

		if (!variable.UISettings.step.empty())
			sscanf_s(variable.UISettings.step.c_str(), "%f", &speed);

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%f", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%f", &max);

        ret = ImGui::DragFloat2(variable.originalName.c_str(), (float*)storage, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		float min = 0.0f;
		float max = 1.0f;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%f", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%f", &max);

        ret = ImGui::SliderFloat2(variable.originalName.c_str(), (float*)storage, min, max);
	}
	else
	{
        ret = ImGui::InputFloat2(variable.originalName.c_str(), (float*)storage);
	}
    return ret;
}

static bool ShowUI_Float3(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
    bool ret = false;
	if (variable.UISettings.UIHint == VariableUIHint::Color)
        ret = ImGui::ColorEdit3(variable.originalName.c_str(), (float*)storage);
	else if (variable.UISettings.UIHint == VariableUIHint::Drag)
	{
		float speed = 1.0f;
		float min	= 0.0f;
		float max	= 0.0f;

		if (!variable.UISettings.step.empty())
			sscanf_s(variable.UISettings.step.c_str(), "%f", &speed);

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%f", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%f", &max);

        ret = ImGui::DragFloat3(variable.originalName.c_str(), (float*)storage, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		float min = 0.0f;
		float max = 1.0f;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%f", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%f", &max);

        ret = ImGui::SliderFloat3(variable.originalName.c_str(), (float*)storage, min, max);
	}
	else
	{
        ret = ImGui::InputFloat3(variable.originalName.c_str(), (float*)storage);
	}
    return ret;
}

static bool ShowUI_Float4(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
    bool ret = false;
	if (variable.UISettings.UIHint == VariableUIHint::Color)
        ret = ImGui::ColorEdit4(variable.originalName.c_str(), (float*)storage, ImGuiColorEditFlags_AlphaPreview);
	else if (variable.UISettings.UIHint == VariableUIHint::Drag)
	{
		float speed = 1.0f;
		float min	= 0.0f;
		float max	= 0.0f;

		if (!variable.UISettings.step.empty())
			sscanf_s(variable.UISettings.step.c_str(), "%f", &speed);

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%f", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%f", &max);

        ret = ImGui::DragFloat4(variable.originalName.c_str(), (float*)storage, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		float min = 0.0f;
		float max = 1.0f;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%f", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%f", &max);

        ret = ImGui::SliderFloat4(variable.originalName.c_str(), (float*)storage, min, max);
	}
	else
	{
        ret = ImGui::InputFloat4(variable.originalName.c_str(), (float*)storage);
	}
    return ret;
}

static bool ShowUI_Bool(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
    bool ret = false;
    if (!paused && variable.UISettings.UIHint == VariableUIHint::Button)
    {
        *(bool*)storage = ImGui::Button(variable.originalName.c_str());
        ret = *(bool*)storage;
    }
	else
        ret = ImGui_Checkbox(variable.originalName.c_str(), (bool*)storage);
    return ret;
}

static bool ShowUI_Float4x4(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
    bool ret = false;
	ShowUI_StartGap();
	ImGui::Text(variable.originalName.c_str());
	ret |= ImGui::InputFloat4("row 0", (float*)storage + 0);
    ret |= ImGui::InputFloat4("row 1", (float*)storage + 4);
    ret |= ImGui::InputFloat4("row 2", (float*)storage + 8);
    ret |= ImGui::InputFloat4("row 3", (float*)storage + 12);
    return ret;
}

static bool ShowUI_Uint_16(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
    bool ret = false;
	uint16_t* v2  = (uint16_t*)storage;
	int		  v[] = { (int)v2[0] };

	if (variable.UISettings.UIHint == VariableUIHint::Drag)
	{
		float speed = 1.0f;
		int	  min	= 0;
		int	  max	= 0;

		if (!variable.UISettings.step.empty())
			sscanf_s(variable.UISettings.step.c_str(), "%f", &speed);

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

		ret = ImGui::DragInt(variable.originalName.c_str(), v, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

        ret = ImGui::SliderInt(variable.originalName.c_str(), v, min, max);
	}
	else
        ret = ImGui::InputInt(variable.originalName.c_str(), v);

	v2[0] = v[0];
    return ret;
}

static bool ShowUI_Float_16(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* rawStorage)
{
    bool ret = false;
    half* storageHalf = (half*)rawStorage;
    float value = *storageHalf;

    if (variable.UISettings.UIHint == VariableUIHint::Angle)
    {
        ret = ImGui::SliderAngle(variable.originalName.c_str(), &value);
    }
    else if (variable.UISettings.UIHint == VariableUIHint::Drag)
    {
        float speed = 1.0f;
        float min = 0.0f;
        float max = 0.0f;

        if (!variable.UISettings.step.empty())
            sscanf_s(variable.UISettings.step.c_str(), "%f", &speed);

        if (!variable.UISettings.min.empty())
            sscanf_s(variable.UISettings.min.c_str(), "%f", &min);

        if (!variable.UISettings.max.empty())
            sscanf_s(variable.UISettings.max.c_str(), "%f", &max);

        ret = ImGui::DragFloat(variable.originalName.c_str(), &value, speed, min, max);
    }
    else if (variable.UISettings.UIHint == VariableUIHint::Slider)
    {
        float min = 0.0f;
        float max = 1.0f;

        if (!variable.UISettings.min.empty())
            sscanf_s(variable.UISettings.min.c_str(), "%f", &min);

        if (!variable.UISettings.max.empty())
            sscanf_s(variable.UISettings.max.c_str(), "%f", &max);

        ret = ImGui::SliderFloat(variable.originalName.c_str(), &value, min, max);
    }
    else
    {
        ret = ImGui::InputFloat(variable.originalName.c_str(), &value);
    }

    *storageHalf = value;
    return ret;
}

static bool ShowUI_Int_64(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
    bool ret = false;
	int64_t* v2 = (int64_t*)storage;
	int		  v[] = { (int)v2[0] };

	if (variable.UISettings.UIHint == VariableUIHint::Drag)
	{
		float speed = 1.0f;
		int	  min = 0;
		int	  max = 0;

		if (!variable.UISettings.step.empty())
			sscanf_s(variable.UISettings.step.c_str(), "%f", &speed);

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

        ret = ImGui::DragInt(variable.originalName.c_str(), v, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

        ret = ImGui::SliderInt(variable.originalName.c_str(), v, min, max);
	}
	else
		ret = ImGui::InputInt(variable.originalName.c_str(), v);

	v2[0] = v[0];
    return ret;
}

static bool ShowUI_Uint_64(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
    bool ret = false;
	uint64_t* v2 = (uint64_t*)storage;
	int		  v[] = { (int)v2[0] };

	if (variable.UISettings.UIHint == VariableUIHint::Drag)
	{
		float speed = 1.0f;
		int	  min = 0;
		int	  max = 0;

		if (!variable.UISettings.step.empty())
			sscanf_s(variable.UISettings.step.c_str(), "%f", &speed);

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

		ret = ImGui::DragInt(variable.originalName.c_str(), v, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

        ret = ImGui::SliderInt(variable.originalName.c_str(), v, min, max);
	}
	else
        ret = ImGui::InputInt(variable.originalName.c_str(), v);

	v2[0] = v[0];
    return ret;
}


static bool ShowUI_Count(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
	// No-op. Shouldn't ever happen
    return false;
}

static std::string VariableUIScope(const Variable& variable)
{
	std::string ret;

	ret = variable.scope;

	if (!variable.UIGroup.empty())
	{
		if (!ret.empty())
			ret += ".";
		ret += variable.UIGroup + ".";
	}

	return ret;
}


// copied from ImGui::Bullet()
void LargeBullet(ImVec4 color, float scale = 1.0f)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	ImVec2 size(g.FontSize * scale, g.FontSize * scale);

	const ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + size.y, window->DC.CursorPos.y + size.y));
	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, 0))
	{
		ImGui::SameLine(0, style.FramePadding.x * 2);
		return;
	}
	
	ImDrawList* draw_list = window->DrawList;

	// Render and stay on same line
	ImVec2 pos;
	pos.x = bb.Min.x + style.FramePadding.x + size.x * 0.5f;
	pos.y = bb.Min.y + style.FramePadding.y + size.y * 0.5f;
	draw_list->AddCircleFilled(pos, size.x * 0.50f, ImGui::GetColorU32(color), 24);	// was *0.2f for small bullet
}

void setToDefault(GigiInterpreterPreviewWindowDX12::RuntimeVariable& variable)
{
	memcpy(variable.storage.value, variable.storage.dflt, variable.storage.size);
	variable.storage.overrideValue = false;
}

// assumes we are in an Indent Block
void VariableUIStart(GigiInterpreterPreviewWindowDX12::RuntimeVariable& variable)
{
	static std::string deferredReset;

	if (variable.variable->name == deferredReset)
	{
		setToDefault(variable);
		deferredReset.clear();
	}

	const ImVec4 oldTextColor = ImVec4(0.1f, 0.1f, 0.8f, 1);;
	const ImVec4 constColor = ImVec4(0.4f, 0.4f, 0.4f, 1);
	const ImVec4 overrrideColor = ImVec4(1, 1, 0.2f, 1);
	const ImVec4 transientColor = ImVec4(1, 1, 0.4f, 1);
	const ImVec4 systemColor = ImVec4(1, 0.4f, 1, 1);

	ImVec4 color = oldTextColor;
	const char* txt = "default (value comes from .gg file)";

	bool canReset = true;

	if (variable.storage.overrideValue)
	{
		color = overrrideColor;
		txt = "override (stored in .gguser file)";
	}
	if (variable.variable->Const)
	{
		color = constColor;
		txt = "constant (cannot be changed, will not be saved)";
		canReset = false;
	}
	if (variable.variable->transient)
	{
		color = transientColor;
		txt = "transient (will not be saved)";
	}
	if (variable.storage.systemValue)
	{
		color = systemColor;
		txt = "system (set by the viewer, will not be saved)";
		canReset = false;
	}

	if (variable.storage.isDefault())
		canReset = false;

	float space = ImGui::GetStyle().IndentSpacing;

	float oldX = ImGui::GetCursorPosX();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() - space);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);
	LargeBullet(color);
	ImGui::SameLine();
	ImGui::PopStyleVar();
	ImGui::SetCursorPosX(oldX);

	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		if (ImGui::BeginTooltipEx(ImGuiTooltipFlags_OverridePreviousTooltip, ImGuiWindowFlags_None))
		{
			ImVec2 pos = ImGui::GetCursorPos();
			LargeBullet(color, 2.0f);
			float gap = ImGui::GetItemRectSize().x + 2 * ImGui::GetStyle().FramePadding.x + ImGui::GetStyle().WindowPadding.x;
			ImGui::SetCursorPos(pos);

			ImGui::Indent(gap);
			ImGui::Text("Variable: %s", variable.variable->name.c_str());
			ImGui::Text(" Comment: %s", variable.variable->comment.c_str());
			ImGui::Unindent(gap);
			ImGui::Text("");
			ImGui::Text("Default%s: %s", 
				canReset ? " (left click to reset)" : "", variable.variable->dflt.c_str());

			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			ImGui::Text("\nColor code meaning: %s", txt);
			ImGui::PopStyleVar();

			ImGui::EndTooltip();
		}
	}
	// we could put this in a context menu
	if (canReset && ImGui::IsItemClicked())
	{
		// Sadly with ImGui we have to defer the operation a frame, even ClearActiveID() does not fix it.
		// This is to fix the case where the element is currently in edit mode.
		deferredReset = variable.variable->name;
	}
}

void GigiInterpreterPreviewWindowDX12::ShowUI(bool minimalUI, bool paused)
{
	// Minimal UI only shows public variables.
	// If there are none, and this is minimalUI, exit early so there's no variables window.
	bool foundPublicVar = false;
	for (const RuntimeVariable& variable : m_runtimeVariables)
	{
		if (variable.variable->visibility == VariableVisibility::User)
		{
			foundPublicVar = true;
			break;
		}
	}
	if (minimalUI && !foundPublicVar)
		return;

	// Show variables, grouped by visibility
	if (m_showVariablesUI || minimalUI)
	{
		if (minimalUI)
			ImGui::Begin("Interface");
		else
			ImGui::Begin("Variables", &m_showVariablesUI);

		// Reset variables to default if we should
		if (ImGui::Button("Reset To Defaults"))
		{
			for (RuntimeVariable& variable : m_runtimeVariables)
			{
				setToDefault(variable);
			}
		}

		/*
		// Color Legend
		{
//			ImGui::Text("Colors: ");
			ImGui::SameLine();
			ImGui::TextColored(overrrideColor, "default");
			ImGui::SameLine();
			ImGui::TextColored(oldTextColor, "override");
			ImGui::SameLine();
			ImGui::TextColored(constColor, "const");
			ImGui::SameLine();
			ImGui::TextColored(transientColor, "transient");
			ImGui::SameLine();
			ImGui::TextColored(systemColor, "system");
		}
		*/

		struct VariableGroup
		{
			VariableVisibility visibility;
			const char*		   label;
		};

		static const VariableGroup variableGroups[] = {
			{ VariableVisibility::User, "User" },
			{ VariableVisibility::Host, "Host" },
			{ VariableVisibility::Internal, "Internal" },
		};

		for (size_t variableGroupIndex = 0; variableGroupIndex < _countof(variableGroups); ++variableGroupIndex)
		{
			if (minimalUI && variableGroupIndex > 0)
				break;

			const VariableGroup& variableGroup = variableGroups[variableGroupIndex];

			// Make everything in sub-loops unique
			ImGui::PushID(variableGroup.label);

			// get the list of scopes in this group of variables
			std::unordered_set<std::string> UIScopes;
			for (const RuntimeVariable& variable : m_runtimeVariables)
			{
				if (variable.variable->visibility != variableGroup.visibility)
					continue;

				std::string UIScope = VariableUIScope(*variable.variable);
				UIScopes.insert(UIScope);
			}

			// make an alpha sorted list of scopes
			std::vector<std::string> UIScopesSorted;
			for (const std::string& UIScope : UIScopes)
				UIScopesSorted.push_back(UIScope);
			std::sort(UIScopesSorted.begin(), UIScopesSorted.end());

			// show the scopes, one at a time, in alpha order
			bool visibilityHeaderShown = false;
			bool visibilityHeaderOpen  = true;
			for (const std::string& currentUIScope : UIScopesSorted)
			{
				// If this visibility header is closed, nothing to do
				if (!visibilityHeaderOpen)
					continue;

				// Get a list of variables in this scope, in this visibility
				std::vector<const RuntimeVariable*> runtimeVariablesSorted;
				for (const RuntimeVariable& variable : m_runtimeVariables)
				{
					if (variable.variable->visibility != variableGroup.visibility || VariableUIScope(*variable.variable) != currentUIScope)
						continue;

					runtimeVariablesSorted.push_back(&variable);
				}
				if (runtimeVariablesSorted.size() == 0)
					continue;

				// The first variable in this visibility is responsible for the collapsing header
				if (!visibilityHeaderShown)
				{
					if (variableGroupIndex != 0)
					{
						if (!ImGui::CollapsingHeader(variableGroup.label))
						{
							visibilityHeaderOpen = false;
							break;
						}
					}
					visibilityHeaderShown = true;

					ImGui::Indent();
				}

				// if the scope isn't empty, indent and make a collapsing header
				if (!currentUIScope.empty())
				{
					std::string scopeLabel = currentUIScope.substr(0, currentUIScope.length() - 1); // trim off the dot
					if (!ImGui::CollapsingHeader(scopeLabel.c_str()))
						continue;
				}

				// push the scope to make the variable imgui IDs unique
				ImGui::PushID(currentUIScope.c_str());

				// Show variable labels and value
				for (const RuntimeVariable* var : runtimeVariablesSorted)
				{
					RuntimeVariable& variable = (RuntimeVariable&)*var;
					
					if (variable.variable->Const)
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					}

					// copy variable.storage.value to get "hasChanged" without affecting a lot of UI code
					// static to avoid reallocations, makes this no reentrant
					static std::vector<const char*> valueCopy;
					valueCopy.clear();
					valueCopy.resize(variable.storage.size);
					memcpy(valueCopy.data(), variable.storage.value, variable.storage.size);

					VariableUIStart(variable);
//					ImGui::PushStyleColor(ImGuiCol_Text, color);

                    bool modifiedByUser = false;
					switch (variable.variable->type)
					{
#include "external/df_serialize/_common.h"
#define ENUM_ITEM(_NAME, _DESCRIPTION) \
	case DataFieldType::_NAME: modifiedByUser = ShowUI_##_NAME(m_renderGraph, paused, *variable.variable, variable.storage.value); break;
						// clang-format off
#include "external/df_serialize/_fillunsetdefines.h"
#include "Schemas/DataFieldTypes.h"
						// clang-format on
					}

                    // If this variable is supposed to set the value of a bool to true when it changes, do that now
                    if (modifiedByUser && variable.variable->onUserChange.variableIndex != -1)
                        SetRuntimeVariableFromString(variable.variable->onUserChange.variableIndex, "true");

//					ImGui::PopStyleColor(1);

					ShowToolTip(variable.variable->comment.c_str());

					bool hasChanged = valueCopy.size() != variable.storage.size
						|| memcmp(variable.storage.value, valueCopy.data(), valueCopy.size()) != 0;

					if (hasChanged)
					{
						variable.storage.overrideValue = true;
					}

					if (variable.variable->Const)
					{
						ImGui::PopItemFlag();
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "(const)");
						if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
							ImGui::SetTooltip("This variable is const and cannot be altered in the viewer. It can only be altered in the .gg file.");
					}
				}

				ImGui::PopID(); // scope
			}

			if (visibilityHeaderShown)
				ImGui::Unindent();

			ImGui::PopID(); // variableGroup.label
		}

		ImGui::End();
	}
}