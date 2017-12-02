#pragma once
#include "stdafx.h"
#include "../Core/Core.h"

namespace FrameDX
{
	// Forward declaration to prevent dependency loop
	class Device;

	// Abstract class for textures
	class Texture
	{
	public:
		Texture()
		{
			RTV = nullptr;	
			SRV = nullptr;	
			DSV = nullptr;	
			DepthSRV = nullptr;	
			UAV = nullptr;
			Version = -1;
		}

		struct Description
		{
			Description()
			{
				MipLevels = 1;
				Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				BindFlags = D3D11_BIND_SHADER_RESOURCE;
				Usage = D3D11_USAGE_DEFAULT;
			}

			uint32_t MipLevels;
			DXGI_FORMAT Format;
			uint32_t BindFlags;
			D3D11_USAGE Usage;
			uint32_t AccessFlags;
		};

		// Creates an empty texture 
		virtual StatusCode CreateFromDescription(Device * OwnerDevice, const Texture::Description& Desc) = 0;

		// Add functions to create custom SRVs, UAVs, etc HERE!

		ID3D11RenderTargetView* RTV;
		ID3D11ShaderResourceView* SRV;
		ID3D11UnorderedAccessView* UAV;
		ID3D11DepthStencilView* DSV;
		ID3D11ShaderResourceView* DepthSRV;
	protected:
		virtual void FillSRVDescription(D3D11_SHADER_RESOURCE_VIEW_DESC* DescPtr) = 0;
		virtual void FillUAVDescription(D3D11_UNORDERED_ACCESS_VIEW_DESC* DescPtr) = 0;
		virtual void FillRTVDescription(D3D11_RENDER_TARGET_VIEW_DESC* DescPtr) = 0;
		virtual void FillSRVDescription1(D3D11_SHADER_RESOURCE_VIEW_DESC1* DescPtr) = 0;
		virtual void FillUAVDescription1(D3D11_UNORDERED_ACCESS_VIEW_DESC1* DescPtr) = 0;
		virtual void FillRTVDescription1(D3D11_RENDER_TARGET_VIEW_DESC1* DescPtr) = 0;

		StatusCode CreateSimpleSRV(Device * OwnerDevice);
		StatusCode CreateSimpleUAV(Device * OwnerDevice);
		StatusCode CreateSimpleRTV(Device * OwnerDevice);
		
		int Version;
		ID3D11Resource* TextureResource;
	};

	class Texture2D : public Texture
	{
	public:
		Texture2D() : Texture(){}

		struct Description : public Texture::Description
		{
			Description() : Texture::Description()
			{
				SizeX = 0;
				SizeY = 0;
			}

			uint32_t SizeX;
			uint32_t SizeY;
		} Desc;

		// Creates an empty texture 
		virtual StatusCode CreateFromDescription(Device * OwnerDevice, const Texture::Description& Desc) final override;

		// Creates a texture from the backbuffer of the swap chain
		// Depending on the access flags it also creates a SRV and a UAV
		StatusCode CreateFromSwapChain(Device * OwnerDevice);
	private:
		virtual void FillSRVDescription(D3D11_SHADER_RESOURCE_VIEW_DESC* DescPtr) final override
		{
			DescPtr->Format = Desc.Format;
			DescPtr->Texture2D.MipLevels = Desc.MipLevels;
			DescPtr->Texture2D.MostDetailedMip = 0;
			DescPtr->ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		}
		virtual void FillUAVDescription(D3D11_UNORDERED_ACCESS_VIEW_DESC* DescPtr) final override
		{
			uav_desc.Format = Desc.Format;
			uav_desc.Texture2D.MipSlice = 0;
			uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		}
		virtual void FillRTVDescription(D3D11_RENDER_TARGET_VIEW_DESC* DescPtr) final override
		virtual void FillSRVDescription1(D3D11_SHADER_RESOURCE_VIEW_DESC1* DescPtr) final override
		{
			DescPtr->Format = Desc.Format;
			DescPtr->Texture2D.MipLevels = Desc.MipLevels;
			DescPtr->Texture2D.MostDetailedMip = 0;
			DescPtr->Texture2D.PlaneSlice = 0;
			DescPtr->ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		}
		virtual void FillUAVDescription1(D3D11_UNORDERED_ACCESS_VIEW_DESC1* DescPtr) final override
		virtual void FillRTVDescription1(D3D11_RENDER_TARGET_VIEW_DESC1* DescPtr) final override
	};
}

		