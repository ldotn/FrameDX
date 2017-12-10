#pragma once
#include "stdafx.h"
#include "../Core/Core.h"

namespace FrameDX
{
	// Forward declaration to prevent dependency loop
	class Device;

	// Abstract class for textures
	// It stores the pointer to the device, so it must remain valid (it must remain valid anyway because DX)
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
			TextureResource = nullptr;
			OwnerDevice = nullptr;
		}

		struct Description
		{
			Description()
			{
				MipLevels = 1;
				Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				BindFlags = D3D11_BIND_SHADER_RESOURCE;
				Usage = D3D11_USAGE_DEFAULT;
				MemoryLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;
				MSAACount = 1;
				MSAAQuality = 0;
				MiscFlags = 0;
				DebugName = L"Texture" + to_wstring(NumberOfTextures.fetch_add(1));
				AccessFlags = 0;
			}

			uint32_t MipLevels;
			DXGI_FORMAT Format;
			uint32_t BindFlags;
			D3D11_USAGE Usage;
			uint32_t AccessFlags;
			D3D11_TEXTURE_LAYOUT MemoryLayout;
			uint32_t MSAACount;
			uint32_t MSAAQuality;
			uint32_t MiscFlags;
			wstring DebugName;
		};
		
		// Makes a full copy from a source texture
		StatusCode CopyFrom(Texture* Source);

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

		StatusCode CreateSimpleSRV();
		StatusCode CreateSimpleUAV();
		StatusCode CreateSimpleRTV();
		
		int Version;
		ID3D11Resource* TextureResource;
		Device * OwnerDevice;

		static atomic<int> NumberOfTextures;
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

		// Creates a texture, optionally filling it with the provided vector
		StatusCode CreateFromDescription(Device * OwnerDevice, const Texture2D::Description & params,vector<uint8_t> Data = {});

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
			DescPtr->Format = Desc.Format;
			DescPtr->Texture2D.MipSlice = 0;
			DescPtr->ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		}
		virtual void FillRTVDescription(D3D11_RENDER_TARGET_VIEW_DESC* DescPtr) final override
		{
			DescPtr->Format = Desc.Format;
			DescPtr->Texture2D.MipSlice = 0;
			DescPtr->ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		}
		virtual void FillSRVDescription1(D3D11_SHADER_RESOURCE_VIEW_DESC1* DescPtr) final override
		{
			DescPtr->Format = Desc.Format;
			DescPtr->Texture2D.MipLevels = Desc.MipLevels;
			DescPtr->Texture2D.MostDetailedMip = 0;
			DescPtr->Texture2D.PlaneSlice = 0;
			DescPtr->ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		}
		virtual void FillUAVDescription1(D3D11_UNORDERED_ACCESS_VIEW_DESC1* DescPtr) final override
		{
			DescPtr->Format = Desc.Format;
			DescPtr->Texture2D.MipSlice = 0;
			DescPtr->Texture2D.PlaneSlice = 0;
			DescPtr->ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		}
		virtual void FillRTVDescription1(D3D11_RENDER_TARGET_VIEW_DESC1* DescPtr) final override
		{
			DescPtr->Format = Desc.Format;
			DescPtr->Texture2D.MipSlice = 0;
			DescPtr->Texture2D.PlaneSlice = 0;
			DescPtr->ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		}
	};
}

		