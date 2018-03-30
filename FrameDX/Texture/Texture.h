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

		// Returns the best version for a specified view type
		enum class ViewType { SRV, UAV, RTV, DSV };

		template<ViewType T>
		int GetBestViewVersion() { return -1; }
		
		template<>
		int GetBestViewVersion<ViewType::SRV>() { return Version > 0 ? 1 : 0; }
		template<>
		int GetBestViewVersion<ViewType::UAV>() { return Version > 0 ? 1 : 0; }
		template<>
		int GetBestViewVersion<ViewType::RTV>() { return Version > 0 ? 1 : 0; }
		template<>
		int GetBestViewVersion<ViewType::DSV>() { return 0; }

		// Creates a view
		// The description can be manually provided by passing a non-null pointer
		// If -1 (default) is specified on the version parameter, the description version is selected automatically
		//		THE DESCRIPTION VERSION PROVIDED (if any) ON THAT CASE MUST MATCH WHAT GetBestViewVersion<xxx>() RETURNS
		StatusCode CreateSRV(void* DescPtr = nullptr,int Version = -1);
		StatusCode CreateUAV(void* DescPtr = nullptr,int Version = -1);
		StatusCode CreateRTV(void* DescPtr = nullptr,int Version = -1);
		StatusCode CreateDSV(void* DescPtr = nullptr,int Version = -1);

		ID3D11RenderTargetView* RTV;
		ID3D11ShaderResourceView* SRV;
		ID3D11UnorderedAccessView* UAV;
		ID3D11DepthStencilView* DSV;

		virtual void FillSRVDescription(D3D11_SHADER_RESOURCE_VIEW_DESC* DescPtr) = 0;
		virtual void FillUAVDescription(D3D11_UNORDERED_ACCESS_VIEW_DESC* DescPtr) = 0;
		virtual void FillRTVDescription(D3D11_RENDER_TARGET_VIEW_DESC* DescPtr) = 0;
		virtual void FillDSVDescription(D3D11_DEPTH_STENCIL_VIEW_DESC* DescPtr) = 0;
		virtual void FillSRVDescription1(D3D11_SHADER_RESOURCE_VIEW_DESC1* DescPtr) = 0;
		virtual void FillUAVDescription1(D3D11_UNORDERED_ACCESS_VIEW_DESC1* DescPtr) = 0;
		virtual void FillRTVDescription1(D3D11_RENDER_TARGET_VIEW_DESC1* DescPtr) = 0;

		ID3D11Resource* GetResource() { return TextureResource;  }
	protected:
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
		// It also allows to disable automatic view creation. The flags can be combined with a bitwise OR
		// By default it tries to create all the views it can
		enum { CreateSRVFlag = 1, CreateUAVFlag = 2, CreateRTVFlag = 4, CreateDSVFlag = 8};
		StatusCode CreateFromDescription( Device * OwnerDevice, 
										 const Texture2D::Description & params,
										 vector<uint8_t> Data = {}, 
										 uint32_t ViewCreationFlags = CreateSRVFlag | CreateUAVFlag | CreateRTVFlag | CreateDSVFlag);

		// Creates a texture from the backbuffer of the swap chain
		// Depending on the access flags it also creates a SRV and a UAV
		StatusCode CreateFromBackbuffer(Device * OwnerDevice);
	
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
			if(Desc.MSAACount > 1)
				DescPtr->ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
			else
				DescPtr->ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		}
		virtual void FillDSVDescription(D3D11_DEPTH_STENCIL_VIEW_DESC* DescPtr) final override
		{
			DescPtr->Format = Desc.Format;
			DescPtr->Texture2D.MipSlice = 0;
			DescPtr->Flags = 0; // Used to indicate read-only https://msdn.microsoft.com/en-us/library/windows/desktop/ff476116%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
			if(Desc.MSAACount > 1)
				DescPtr->ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			else
				DescPtr->ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
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
			if(Desc.MSAACount > 1)
				DescPtr->ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
			else
				DescPtr->ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		}
	};
}

		