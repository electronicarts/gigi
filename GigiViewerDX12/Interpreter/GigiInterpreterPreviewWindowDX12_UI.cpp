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

static void ShowToolTip(const char* tooltip)
{
	if (!tooltip || !tooltip[0])
		return;

	// ImGui::SameLine();
	// ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[?]");
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("%s", tooltip);
}

static void ShowUI_Int(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
	if (variable.enumIndex != -1)
	{
		const Enum& e = renderGraph.enums[variable.enumIndex];

		std::vector<const char*> labels;
		for (const EnumItem& item : e.items)
			labels.push_back(item.displayLabel.c_str());

		ImGui::Combo(variable.originalName.c_str(), (int*)storage, labels.data(), (int)labels.size());
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

			ImGui::DragInt(variable.originalName.c_str(), (int*)storage, speed, min, max);
		}
		else if (variable.UISettings.UIHint == VariableUIHint::Slider)
		{
			int min = 0;
			int max = 65535;

			if (!variable.UISettings.min.empty())
				sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

			if (!variable.UISettings.max.empty())
				sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

			ImGui::SliderInt(variable.originalName.c_str(), (int*)storage, min, max);
		}
		else
			ImGui::InputInt(variable.originalName.c_str(), (int*)storage);
	}
}

static void ShowUI_Int2(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
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

		ImGui::DragInt2(variable.originalName.c_str(), (int*)storage, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

		ImGui::SliderInt2(variable.originalName.c_str(), (int*)storage, min, max);
	}
	else
		ImGui::InputInt2(variable.originalName.c_str(), (int*)storage);
}

static void ShowUI_Int3(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
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

		ImGui::DragInt3(variable.originalName.c_str(), (int*)storage, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

		ImGui::SliderInt3(variable.originalName.c_str(), (int*)storage, min, max);
	}
	else
		ImGui::InputInt3(variable.originalName.c_str(), (int*)storage);
}

static void ShowUI_Int4(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
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

		ImGui::DragInt4(variable.originalName.c_str(), (int*)storage, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

		ImGui::SliderInt4(variable.originalName.c_str(), (int*)storage, min, max);
	}
	else
		ImGui::InputInt4(variable.originalName.c_str(), (int*)storage);
}

static void ShowUI_Uint(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
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

		ImGui::DragInt(variable.originalName.c_str(), v, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

		ImGui::SliderInt(variable.originalName.c_str(), v, min, max);
	}
	else
		ImGui::InputInt(variable.originalName.c_str(), v);

	v2[0] = v[0];
}

static void ShowUI_Uint2(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
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

		ImGui::DragInt2(variable.originalName.c_str(), v, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

		ImGui::SliderInt2(variable.originalName.c_str(), v, min, max);
	}
	else
		ImGui::InputInt2(variable.originalName.c_str(), v);

	v2[0] = v[0];
	v2[1] = v[1];
}

static void ShowUI_Uint3(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
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

		ImGui::DragInt3(variable.originalName.c_str(), v, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

		ImGui::SliderInt3(variable.originalName.c_str(), v, min, max);
	}
	else
		ImGui::InputInt3(variable.originalName.c_str(), v);

	v2[0] = v[0];
	v2[1] = v[1];
	v2[2] = v[2];
}

static void ShowUI_Uint4(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
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

		ImGui::DragInt4(variable.originalName.c_str(), v, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

		ImGui::SliderInt4(variable.originalName.c_str(), v, min, max);
	}
	else
		ImGui::InputInt4(variable.originalName.c_str(), v);

	v2[0] = v[0];
	v2[1] = v[1];
	v2[2] = v[2];
	v2[3] = v[3];
}

static void ShowUI_Float(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
	if (variable.UISettings.UIHint == VariableUIHint::Angle)
	{
		ImGui::SliderAngle(variable.originalName.c_str(), (float*)storage);
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

		ImGui::DragFloat(variable.originalName.c_str(), (float*)storage, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		float min = 0.0f;
		float max = 1.0f;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%f", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%f", &max);

		ImGui::SliderFloat(variable.originalName.c_str(), (float*)storage, min, max);
	}
	else
	{
		ImGui::InputFloat(variable.originalName.c_str(), (float*)storage);
	}
}

static void ShowUI_Float2(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
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

		ImGui::DragFloat2(variable.originalName.c_str(), (float*)storage, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		float min = 0.0f;
		float max = 1.0f;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%f", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%f", &max);

		ImGui::SliderFloat2(variable.originalName.c_str(), (float*)storage, min, max);
	}
	else
	{
		ImGui::InputFloat2(variable.originalName.c_str(), (float*)storage);
	}
}

static void ShowUI_Float3(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
	if (variable.UISettings.UIHint == VariableUIHint::Color)
		ImGui::ColorEdit3(variable.originalName.c_str(), (float*)storage);
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

		ImGui::DragFloat3(variable.originalName.c_str(), (float*)storage, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		float min = 0.0f;
		float max = 1.0f;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%f", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%f", &max);

		ImGui::SliderFloat3(variable.originalName.c_str(), (float*)storage, min, max);
	}
	else
	{
		ImGui::InputFloat3(variable.originalName.c_str(), (float*)storage);
	}
}

static void ShowUI_Float4(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
	if (variable.UISettings.UIHint == VariableUIHint::Color)
		ImGui::ColorEdit4(variable.originalName.c_str(), (float*)storage, ImGuiColorEditFlags_AlphaPreview);
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

		ImGui::DragFloat4(variable.originalName.c_str(), (float*)storage, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		float min = 0.0f;
		float max = 1.0f;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%f", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%f", &max);

		ImGui::SliderFloat4(variable.originalName.c_str(), (float*)storage, min, max);
	}
	else
	{
		ImGui::InputFloat4(variable.originalName.c_str(), (float*)storage);
	}
}

static void ShowUI_Bool(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
	if (!paused && variable.UISettings.UIHint == VariableUIHint::Button)
		*(bool*)storage = ImGui::Button(variable.originalName.c_str());
	else
		ImGui_Checkbox(variable.originalName.c_str(), (bool*)storage);
}

static void ShowUI_Float4x4(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
	ImGui::Text(variable.originalName.c_str());
	ImGui::InputFloat4("row 0", (float*)storage + 0);
	ImGui::InputFloat4("row 1", (float*)storage + 4);
	ImGui::InputFloat4("row 2", (float*)storage + 8);
	ImGui::InputFloat4("row 3", (float*)storage + 12);
}

static void ShowUI_Uint_16(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
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

		ImGui::DragInt(variable.originalName.c_str(), v, speed, min, max);
	}
	else if (variable.UISettings.UIHint == VariableUIHint::Slider)
	{
		int min = 0;
		int max = 65535;

		if (!variable.UISettings.min.empty())
			sscanf_s(variable.UISettings.min.c_str(), "%i", &min);

		if (!variable.UISettings.max.empty())
			sscanf_s(variable.UISettings.max.c_str(), "%i", &max);

		ImGui::SliderInt(variable.originalName.c_str(), v, min, max);
	}
	else
		ImGui::InputInt(variable.originalName.c_str(), v);

	v2[0] = v[0];
}

static void ShowUI_Count(const RenderGraph& renderGraph, const bool paused, const Variable& variable, void* storage)
{
	// No-op. Shouldn't ever happen
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
			for (const RuntimeVariable& variable : m_runtimeVariables)
				memcpy(variable.storage.value, variable.storage.dflt, variable.storage.size);
		}

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
					const RuntimeVariable& variable = *var;

					if (variable.variable->Const)
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					}

					switch (variable.variable->type)
					{
#include "external/df_serialize/_common.h"
#define ENUM_ITEM(_NAME, _DESCRIPTION) \
	case DataFieldType::_NAME: ShowUI_##_NAME(m_renderGraph, paused, *variable.variable, variable.storage.value); break;
						// clang-format off
#include "external/df_serialize/_fillunsetdefines.h"
#include "Schemas/DataFieldTypes.h"
						// clang-format on
					}

					ShowToolTip(variable.variable->comment.c_str());

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