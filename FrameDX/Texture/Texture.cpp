#include "stdafx.h"
#include "Texture.h"
#include "..\Device\Device.h"
#include "..\Core\Log.h"

using namespace FrameDX;

StatusCode FrameDX::Texture::CreateFromSwapChain(Device * OwnerDevice)
{
	// Check swapchain version
	auto sw1 = OwnerDevice->GetSwapChain1(false);
	if(sw1)
	{
		Version = 1;
		LogCheckWithReturn(sw1->GetBuffer(0, __uuidof(ID3D11Texture2D1), (void**)&TextureResource),LogCategory::Error);
	}
	else
	{
		Version = 0;
		LogCheckWithReturn(OwnerDevice->GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D1), (void**)&TextureResource),LogCategory::Error);
	}
	
	// Get description
	D3D11_TEXTURE2D_DESC desc;
	TextureResource->GetDesc(&desc);

	Desc.SizeX = desc.Width;
	Desc.SizeY = desc.Height;
	Desc.Format = desc.Format;

	// Check if an UAV or SRV needs to be created
	//if(desc.BindFlags & )

	return StatusCode::Ok;
}
