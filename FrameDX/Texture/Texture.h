#pragma once
#include "stdafx.h"

namespace FrameDX
{
	// Forward declaration to prevent dependency loop
	class Device;

	class Texture
	{
	public:
		struct Description
		{
			uint32_t SizeX;
			uint32_t SizeY;
			DXGI_FORMAT Format;
		};

	private:

	};
}