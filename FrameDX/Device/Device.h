#pragma once
#include "../Core/Core.h"
#include "../Texture/Texture.h"

namespace FrameDX
{
	class Device
	{
	public:
		struct Description
		{
			D3D11_CULL_MODE CullMode;
			D3D11_FILL_MODE FillMode;
			int AdapterIndex;
			bool ComputeOnly; // Compute only tasks don't need a backbuffer and a swap chain

			// Only valid if ComputeOnly == false
			Texture::Description BackbufferDescription;
		} Desc;

		StatusCode Start(const Description& CreationParameters);
	private:
		ID3D11Device *        D3DDevice;
		ID3D11DeviceContext * ImmediateContext;

		// Only valid if ComputeOnly == false
		IDXGISwapChain * SwapChain;
		Texture *        Backbuffer; 	
	};
}
