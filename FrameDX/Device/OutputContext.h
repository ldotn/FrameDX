#pragma once
#include "stdafx.h"
#include "../Core/Core.h"
#include "../Core/Binding.h"

namespace FrameDX
{
	// Stores the output context like render targets, uavs, viewport, etc
	// If both RTVs and UAVs are linked, it'll bind the UAVs right after the RTVs
	// If no dsv is linked, 
	class OutputContext : public Bindable
	{
	public:
		OutputContext(Device* OwnerDev) 
		{ 
			LinkFlags = CanLinkRTV | CanLinkDSV | CanLinkUAV | CanLinkViewport | CanLinkDepthState | CanLinkRasterState;
			OwnerDevice = OwnerDev;
		}

		virtual StatusCode Bind() final override;
		virtual StatusCode Unbind() final override;
	};
}