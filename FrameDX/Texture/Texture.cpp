#include "stdafx.h"
#include "Texture.h"
#include "..\Device\Device.h"
#include "..\Core\Log.h"

using namespace FrameDX;
using namespace std;

atomic<int> Texture::NumberOfTextures(0);

StatusCode FrameDX::Texture::CreateSimpleSRV()
{
	// Device version 3 required to create a SRV1
	if(Version == 0 || OwnerDevice->GetDeviceVersion() < 3)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
		FillSRVDescription(&srv_desc);

		LogCheckWithReturn(OwnerDevice->GetDevice()->CreateShaderResourceView(TextureResource,&srv_desc,&SRV),LogCategory::Error);
	}
	else if(Version == 1)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC1 srv_desc;
		FillSRVDescription1(&srv_desc);

		LogCheckWithReturn(OwnerDevice->GetDevice3()->CreateShaderResourceView1(TextureResource,&srv_desc,(ID3D11ShaderResourceView1**)&SRV),LogCategory::Error);
	}
	else
		return StatusCode::InvalidArgument;

	return StatusCode::Ok;
}

StatusCode FrameDX::Texture::CreateSimpleUAV()
{
	// Device version 3 required to create an UAV1
	if(Version == 0 || OwnerDevice->GetDeviceVersion() < 3)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
		FillUAVDescription(&uav_desc);

		LogCheckWithReturn(OwnerDevice->GetDevice()->CreateUnorderedAccessView(TextureResource,&uav_desc,&UAV),LogCategory::Error);
	}
	else if(Version == 1)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC1 uav_desc;
		FillUAVDescription1(&uav_desc);

		LogCheckWithReturn(OwnerDevice->GetDevice3()->CreateUnorderedAccessView1(TextureResource,&uav_desc,(ID3D11UnorderedAccessView1**)&UAV),LogCategory::Error);
	}
	else
		return StatusCode::InvalidArgument;
	return StatusCode::Ok;
}

StatusCode FrameDX::Texture::CreateSimpleRTV()
{
	// Device version 3 required to create a RTV1
	if(Version == 0 || OwnerDevice->GetDeviceVersion() < 3)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
		FillRTVDescription(&rtv_desc);

		LogCheckWithReturn(OwnerDevice->GetDevice()->CreateRenderTargetView(TextureResource,&rtv_desc,&RTV),LogCategory::Error);
	}
	else if(Version == 1)
	{
		D3D11_RENDER_TARGET_VIEW_DESC1 rtv_desc;
		FillRTVDescription1(&rtv_desc);

		LogCheckWithReturn(OwnerDevice->GetDevice3()->CreateRenderTargetView1(TextureResource,&rtv_desc,(ID3D11RenderTargetView1**)&RTV),LogCategory::Error);
	}
	else
		return StatusCode::InvalidArgument;
	return StatusCode::Ok;
}

StatusCode FrameDX::Texture2D::CreateFromSwapChain(Device * device)
{
	OwnerDevice = device;

	// Check swapchain version
	if(OwnerDevice->GetSwapChainVersion() == 1)
	{
		Version = 1;
		LogCheckWithReturn(OwnerDevice->GetSwapChain1(false)->GetBuffer(0, __uuidof(ID3D11Texture2D1), (void**)&TextureResource),LogCategory::Error);
	}
	else
	{
		Version = 0;
		LogCheckWithReturn(OwnerDevice->GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D1), (void**)&TextureResource),LogCategory::Error);
	}
	
	// Get description
	D3D11_TEXTURE2D_DESC desc;
	((ID3D11Texture2D*)TextureResource)->GetDesc(&desc);

	Desc.SizeX = desc.Width;
	Desc.SizeY = desc.Height;
	Desc.Format = desc.Format;
	Desc.BindFlags = desc.BindFlags;
	Desc.Usage = desc.Usage;
	Desc.AccessFlags = desc.CPUAccessFlags;
	Desc.DebugName = L"Texture_Backbuffer";

	// Set debug name
	TextureResource->SetPrivateData(WKPDID_D3DDebugObjectName, Desc.DebugName.size()-1, Desc.DebugName.c_str());

	// Check if an UAV or SRV needs to be created
	if(desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
		LogCheckWithReturn(CreateSimpleSRV(),LogCategory::Error);
	if(desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
		LogCheckWithReturn(CreateSimpleUAV(),LogCategory::Error)
	if(desc.BindFlags & D3D11_BIND_RENDER_TARGET)
		LogCheckWithReturn(CreateSimpleRTV(),LogCategory::Error)

	return StatusCode::Ok;
}

StatusCode FrameDX::Texture2D::CreateFromDescription(Device * device, const Texture2D::Description & params,vector<uint8_t> Data)
{
	OwnerDevice = device;
	Desc = params;

	// Only transfer data
	D3D11_SUBRESOURCE_DATA sdata;
	sdata.pSysMem = nullptr;
	if(Data.size() > 0)
	{
		// Check that the size is valid
		auto bits = DirectX::LoaderHelpers::BitsPerPixel(Desc.Format);

		// Make sure that the data is at byte level
		// There are special formats that are not on byte level, ignore loading from them for now
		if(LogAssertAndContinue(bits % 8 == 0,LogCategory::Error))
			return StatusCode::NotImplemented;

		auto bpp = bits / 8;
		// Only use the simple memory layout for now
		if(LogAssertAndContinue(Desc.MemoryLayout != D3D11_TEXTURE_LAYOUT_64K_STANDARD_SWIZZLE,LogCategory::Error))
			return StatusCode::NotImplemented;

		// Check that the buffer is big enough
		if(LogAssertAndContinue(Data.size() >= Desc.SizeX*Desc.SizeY*bpp,LogCategory::Error))
			return StatusCode::InvalidArgument;

		// With all checks done, just fill the descriptor
		sdata.pSysMem = Data.data();
		sdata.SysMemPitch = Desc.SizeX * bpp;
		sdata.SysMemSlicePitch = Desc.SizeY * Desc.SizeX * bpp;
	}

	if(OwnerDevice->GetDeviceVersion() >= 3)
	{
		D3D11_TEXTURE2D_DESC1 desc;
		desc.ArraySize = 1;
		desc.Format = Desc.Format;
		desc.BindFlags = Desc.BindFlags;
		desc.CPUAccessFlags = Desc.AccessFlags;
		desc.MipLevels = Desc.MipLevels;
		desc.Width = Desc.SizeX;
		desc.Height = Desc.SizeY;
		desc.TextureLayout = Desc.MemoryLayout;
		desc.SampleDesc.Count = Desc.MSAACount;
		desc.SampleDesc.Quality = Desc.MSAAQuality;
		desc.MiscFlags = Desc.MiscFlags;
		desc.Usage = Desc.Usage;

		LogCheckWithReturn(OwnerDevice->GetDevice3()->CreateTexture2D1(&desc,sdata.pSysMem ? &sdata : nullptr,(ID3D11Texture2D1**)&TextureResource),LogCategory::Error);
	}
	else
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.ArraySize = 1;
		desc.Format = Desc.Format;
		desc.BindFlags = Desc.BindFlags;
		desc.CPUAccessFlags = Desc.AccessFlags;
		desc.MipLevels = Desc.MipLevels;
		desc.Width = Desc.SizeX;
		desc.Height = Desc.SizeY;
		desc.SampleDesc.Count = Desc.MSAACount;
		desc.SampleDesc.Quality = Desc.MSAAQuality;
		desc.MiscFlags = Desc.MiscFlags;
		desc.Usage = Desc.Usage;

		LogCheckWithReturn(OwnerDevice->GetDevice()->CreateTexture2D(&desc,sdata.pSysMem ? &sdata : nullptr,(ID3D11Texture2D**)&TextureResource),LogCategory::Error);
	}
	
	// Set debug name
	TextureResource->SetPrivateData(WKPDID_D3DDebugObjectName, Desc.DebugName.size()-1, Desc.DebugName.c_str());

	return StatusCode::Ok;
}

StatusCode FrameDX::Texture::CopyFrom(Texture* Source)
{
	// Make sure that there's no error from before
	SetLastError(S_OK);

	// This returns void, using GetLastError to know if it worked or not
	OwnerDevice->GetImmediateContext()->CopyResource(TextureResource,Source->TextureResource);

	return LAST_ERROR;
}
