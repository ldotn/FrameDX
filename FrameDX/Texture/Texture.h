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
			Description()
			{
				SizeX = 0;
				SizeY = 0;
				Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				MSAACount = 1;
				MSAAQuality = 0;
			}

			uint32_t SizeX;
			uint32_t SizeY;
			DXGI_FORMAT Format;
		
			uint32_t MSAACount;
			uint32_t MSAAQuality;
		};

	private:

	};
}