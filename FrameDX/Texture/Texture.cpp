#include "stdafx.h"
#include "Texture.h"
#include "..\Device\Device.h"
#include "..\Core\Log.h"

using namespace FrameDX;
using namespace std;

atomic<int> Texture::NumberOfTextures(0);

StatusCode FrameDX::Texture::CreateSRV(void* DescPtr,int InVersion)
{
	int TargetVersion = (InVersion == -1) ? GetBestViewVersion<ViewType::SRV>() : InVersion;
	// If trying to create a higher version, make sure the device supports it
	// Just checking for v1 now, which requires device v3
	if(Version < 0 || (TargetVersion > 0 &&  OwnerDevice->GetDeviceVersion() < 3))
		return StatusCode::InvalidArgument;

	if(TargetVersion == 0)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
		if(DescPtr != nullptr)
			srv_desc = *(D3D11_SHADER_RESOURCE_VIEW_DESC*)DescPtr;
		else
			FillSRVDescription(&srv_desc);

		LogCheckWithReturn(OwnerDevice->GetDevice()->CreateShaderResourceView(TextureResource,&srv_desc,&SRV),LogCategory::Error);
	}
	else if(TargetVersion == 1)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC1 srv_desc;
		if(DescPtr != nullptr)
			srv_desc = *(D3D11_SHADER_RESOURCE_VIEW_DESC1*)DescPtr;
		else
			FillSRVDescription1(&srv_desc);

		LogCheckWithReturn(OwnerDevice->GetDevice3()->CreateShaderResourceView1(TextureResource,&srv_desc,(ID3D11ShaderResourceView1**)&SRV),LogCategory::Error);
	}
	else
		return StatusCode::InvalidArgument;
	
	return StatusCode::Ok;
}

StatusCode FrameDX::Texture::CreateUAV(void* DescPtr,int InVersion)
{
	int TargetVersion = (InVersion == -1) ? GetBestViewVersion<ViewType::UAV>() : InVersion;
	// If trying to create a higher version, make sure the device supports it
	// Just checking for v1 now, which requires device v3
	if(Version < 0 || (TargetVersion > 0 &&  OwnerDevice->GetDeviceVersion() < 3))
		return StatusCode::InvalidArgument;

	if(TargetVersion == 0)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
		if(DescPtr != nullptr)
			uav_desc = *(D3D11_UNORDERED_ACCESS_VIEW_DESC*)DescPtr;
		else
			FillUAVDescription(&uav_desc);

		LogCheckWithReturn(OwnerDevice->GetDevice()->CreateUnorderedAccessView(TextureResource,&uav_desc,&UAV),LogCategory::Error);
	}
	else if(TargetVersion == 1)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC1 uav_desc;
		if(DescPtr != nullptr)
			uav_desc = *(D3D11_UNORDERED_ACCESS_VIEW_DESC1*)DescPtr;
		else
			FillUAVDescription1(&uav_desc);

		LogCheckWithReturn(OwnerDevice->GetDevice3()->CreateUnorderedAccessView1(TextureResource,&uav_desc,(ID3D11UnorderedAccessView1**)&UAV),LogCategory::Error);
	}
	else
		return StatusCode::InvalidArgument;
	return StatusCode::Ok;
}

StatusCode FrameDX::Texture::CreateRTV(void* DescPtr,int InVersion)
{
	int TargetVersion = (InVersion == -1) ? GetBestViewVersion<ViewType::RTV>() : InVersion;
	// If trying to create a higher version, make sure the device supports it
	// Just checking for v1 now, which requires device v3
	if(Version < 0 || (TargetVersion > 0 &&  OwnerDevice->GetDeviceVersion() < 3))
		return StatusCode::InvalidArgument;

	if(TargetVersion == 0)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
		if(DescPtr != nullptr)
			rtv_desc = *(D3D11_RENDER_TARGET_VIEW_DESC*)DescPtr;
		else
			FillRTVDescription(&rtv_desc);

		LogCheckWithReturn(OwnerDevice->GetDevice()->CreateRenderTargetView(TextureResource,&rtv_desc,&RTV),LogCategory::Error);
	}
	else if(TargetVersion == 1)
	{
		D3D11_RENDER_TARGET_VIEW_DESC1 rtv_desc;
		if(DescPtr != nullptr)
			rtv_desc = *(D3D11_RENDER_TARGET_VIEW_DESC1*)DescPtr;
		else
			FillRTVDescription1(&rtv_desc);

		LogCheckWithReturn(OwnerDevice->GetDevice3()->CreateRenderTargetView1(TextureResource,&rtv_desc,(ID3D11RenderTargetView1**)&RTV),LogCategory::Error);
	}
	else
		return StatusCode::InvalidArgument;
	return StatusCode::Ok;
}

StatusCode FrameDX::Texture::CreateDSV(void* DescPtr,int InVersion)
{
	int TargetVersion = (InVersion == -1) ? GetBestViewVersion<ViewType::DSV>() : InVersion;
	// If trying to create a higher version, make sure the device supports it
	// Just checking for v1 now, which requires device v3
	if(Version < 0 || (TargetVersion > 0 &&  OwnerDevice->GetDeviceVersion() < 3))
		return StatusCode::InvalidArgument;

	if(TargetVersion == 0)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
		if(DescPtr != nullptr)
			dsv_desc = *(D3D11_DEPTH_STENCIL_VIEW_DESC*)DescPtr;
		else
			FillDSVDescription(&dsv_desc);

		LogCheckWithReturn(OwnerDevice->GetDevice()->CreateDepthStencilView(TextureResource,&dsv_desc,&DSV),LogCategory::Error);
	}
	else
		return StatusCode::InvalidArgument;
	return StatusCode::Ok;
}

StatusCode FrameDX::Texture2D::CreateFromBackbuffer(Device * device)
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

	// Check if an UAV, SRV or RTV needs to be created
	if(Desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
		LogCheckWithReturn(CreateSRV(),LogCategory::Error);
	if(Desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
		LogCheckWithReturn(CreateUAV(),LogCategory::Error);
	if(Desc.BindFlags & D3D11_BIND_RENDER_TARGET)
		LogCheckWithReturn(CreateRTV(),LogCategory::Error);

	return StatusCode::Ok;
}

StatusCode FrameDX::Texture2D::CreateFromDescription(Device * device, const Texture2D::Description & params,vector<uint8_t> Data, uint32_t ViewCreationFlags)
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
		Version = 1;

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
		Version = 0;

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

	// Create the required views
	if((ViewCreationFlags & CreateSRVFlag) && (Desc.BindFlags & D3D11_BIND_SHADER_RESOURCE))
		LogCheckWithReturn(CreateSRV(),LogCategory::Error);
	if((ViewCreationFlags & CreateUAVFlag) && (Desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS))
		LogCheckWithReturn(CreateUAV(),LogCategory::Error);
	if((ViewCreationFlags & CreateRTVFlag) && (Desc.BindFlags & D3D11_BIND_RENDER_TARGET))
		LogCheckWithReturn(CreateRTV(),LogCategory::Error);
	if((ViewCreationFlags & CreateDSVFlag) && (Desc.BindFlags & D3D11_BIND_DEPTH_STENCIL))
		LogCheckWithReturn(CreateDSV(),LogCategory::Error);

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
