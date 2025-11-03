///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ui/widgets.h"
#include "ui/builders.h"

inline void DrawPinIcon(const NodePinInfo& pin, bool isReroute, bool connected, int alpha)
{
    ax::Drawing::IconType iconType = ax::Drawing::IconType::Circle;
    ImColor color = ImColor(200, 200, 200);

    if (isReroute)
    {
        color = ImColor(150, 150, 150);
    }
    else if (!pin.required)
    {
        iconType = ax::Drawing::IconType::Diamond;
        color = ImColor(125, 125, 125);
    }

	color.Value.w = alpha / 255.0f;

	const int m_PinIconSize = 16;
	ax::Widgets::Icon(ImVec2(static_cast<float>(m_PinIconSize), static_cast<float>(m_PinIconSize)), iconType, connected, color, ImColor(32, 32, 32, alpha));
};
