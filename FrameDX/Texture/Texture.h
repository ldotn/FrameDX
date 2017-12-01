#pragma once
#include "stdafx.h"

namespace FrameDX
{
	// Forward declaration to prevent dependency loop
	class Device;

	class Texture
	{
	public:
		Texture()
		{
			TextureResource = nullptr;	
			TargetView = nullptr;	
			ResourceView = nullptr;	
			DepthStencil = nullptr;	
			DepthSRV = nullptr;	
			Version = -1;
		}

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

		// Creates a texture from the backbuffer of the swap chain
		// Depending on the access flags it also creates a SRV and a UAV
		StatusCode CreateFromSwapChain(const Device& );

	private:
		int Version;

		ID3D11Texture2D* TextureResource;	
		ID3D11RenderTargetView* TargetView;
		ID3D11ShaderResourceView* ResourceView;

		ID3D11DepthStencilView* DepthStencil;
		ID3D11ShaderResourceView* DepthSRV;
	};
}