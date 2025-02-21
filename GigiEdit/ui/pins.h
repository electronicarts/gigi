///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ui/widgets.h"
#include "ui/builders.h"

enum class PinType
{
	Flow,
	Bool,
	Int,
	Float,
	String,
	Object,
	Function,
	Delegate,
};

ImColor GetIconColor(PinType type)
{
	switch (type)
	{
	default:
	case PinType::Flow:     return ImColor(255, 255, 255);
	case PinType::Bool:     return ImColor(220, 48, 48);
	case PinType::Int:      return ImColor(68, 201, 156);
	case PinType::Float:    return ImColor(147, 226, 74);
	case PinType::String:   return ImColor(124, 21, 153);
//	case PinType::Object:   return ImColor(51, 150, 215);
	case PinType::Object:   return ImColor(200, 200, 200); // Gigi choice, as we not have a type grey is appropriate
	case PinType::Function: return ImColor(218, 0, 183);
	case PinType::Delegate: return ImColor(255, 48, 48);
	}
};

// code is from imgui-node-editor blueprint example, reduced to single data type

inline void DrawPinIcon(const NodePinInfo& pin, bool isReroute, bool connected, int alpha)
{
	ax::Drawing::IconType iconType;

	PinType type = PinType::Object;
	
	ImColor color = isReroute ? ImColor(150, 150, 150, 255) : GetIconColor(type);
	color.Value.w = alpha / 255.0f;

	switch (type)
	{
	case PinType::Flow:     iconType = ax::Drawing::IconType::Flow;   break;
	case PinType::Bool:     iconType = ax::Drawing::IconType::Circle; break;
	case PinType::Int:      iconType = ax::Drawing::IconType::Circle; break;
	case PinType::Float:    iconType = ax::Drawing::IconType::Circle; break;
	case PinType::String:   iconType = ax::Drawing::IconType::Circle; break;
	case PinType::Object:   iconType = ax::Drawing::IconType::Circle; break;
	case PinType::Function: iconType = ax::Drawing::IconType::Circle; break;
	case PinType::Delegate: iconType = ax::Drawing::IconType::Square; break;
	default:
		return;
	}
	const int m_PinIconSize = 16;
	ax::Widgets::Icon(ImVec2(static_cast<float>(m_PinIconSize), static_cast<float>(m_PinIconSize)), iconType, connected, color, ImColor(32, 32, 32, alpha));
};
