#pragma once
#include "stdafx.h"
#include "Core.h"
#include "Log.h"
#include "../Device/Device.h"

namespace FrameDX
{
	// Abstract base class for all resources that are bound to the pipeline (shaders mostly)
	// Bindable resources can have other resources linked to them (like textures) that are bound when the resource is bound
	class Bindable
	{
	protected:
		enum
		{ 
			CanLinkSRV = 1 << 0,
			CanLinkUAV = 1 << 1,
			CanLinkRTV = 1 << 2,
			CanLinkDSV = 1 << 3,
			CanLinkSampler = 1 << 4,
			CanLinkCB = 1 << 5,
			CanLinkViewport = 1 << 6,
			CanLinkRasterState = 1 << 7,
			CanLinkDepthState = 1 << 8
		};

		uint32_t LinkFlags;
	public:
		Bindable()
		{
			IsDSVLinked = false;
			IsRasterStateLinked = false;
			IsDepthStateLinked = false;
		}

		StatusCode LinkSRV(ID3D11ShaderResourceView* SRV,uint32_t Slot)
		{
			// Log and return if the resource doesn't support linking this type
			if(LogAssertAndContinue(LinkFlags & CanLinkSRV,LogCategory::Warning))
				return StatusCode::Ok;

			LogAssertWithReturn(Slot < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT,LogCategory::Error,StatusCode::InvalidArgument);

			if(LinkedSRVs.size() <= Slot)
				LinkedSRVs.resize(Slot + 1);

			LinkedSRVs[Slot] = SRV;
			return StatusCode::Ok;
		}

		StatusCode LinkUAV(ID3D11UnorderedAccessView* UAV,uint32_t Slot)
		{ 
			// Log and return if the resource doesn't support linking this type
			if(LogAssertAndContinue(LinkFlags & CanLinkUAV,LogCategory::Warning))
				return StatusCode::Ok;

			LogAssertWithReturn(Slot < ((OwnerDevice->GetDeviceVersion() > 0) ? D3D11_1_UAV_SLOT_COUNT : D3D11_PS_CS_UAV_REGISTER_COUNT),LogCategory::Error,StatusCode::InvalidArgument);

			if(LinkedUAVs.size() <= Slot)
				LinkedUAVs.resize(Slot + 1);

			LinkedUAVs[Slot] = UAV;
			return StatusCode::Ok;
		}

		StatusCode LinkRTV(ID3D11RenderTargetView* RTV,uint32_t Slot)
		{
			// Log and return if the resource doesn't support linking this type
			if(LogAssertAndContinue(LinkFlags & CanLinkRTV,LogCategory::Warning))
				return StatusCode::Ok;

			LogAssertWithReturn(Slot < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,LogCategory::Error,StatusCode::InvalidArgument);

			if(LinkedRTVs.size() <= Slot)
				LinkedRTVs.resize(Slot + 1);

			LinkedRTVs[Slot] = RTV;
			return StatusCode::Ok;
		}

		StatusCode LinkDSV(ID3D11DepthStencilView* DSV)
		{
			// Log and return if the resource doesn't support linking this type
			if(LogAssertAndContinue(LinkFlags & CanLinkDSV,LogCategory::Warning))
				return StatusCode::Ok;

			// Doing this as you could link nullptr as a DSV
			IsDSVLinked = true;
			LinkedDSV = DSV;
			return StatusCode::Ok;
		}

		StatusCode LinkSampler(ID3D11SamplerState* Sampler,uint32_t Slot)
		{
			// Log and return if the resource doesn't support linking this type
			if(LogAssertAndContinue(LinkFlags & CanLinkSampler,LogCategory::Warning))
				return StatusCode::Ok;

			LogAssertWithReturn(Slot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT,LogCategory::Error,StatusCode::InvalidArgument);

			if(LinkedSamplers.size() <= Slot)
				LinkedSamplers.resize(Slot + 1);

			LinkedSamplers[Slot] = Sampler;
			return StatusCode::Ok;
		}

		StatusCode LinkConstantBuffer(ID3D11Buffer* CB,uint32_t Slot)
		{
			// Log and return if the resource doesn't support linking this type
			if(LogAssertAndContinue(LinkFlags & CanLinkCB,LogCategory::Warning))
				return StatusCode::Ok;

			LogAssertWithReturn(Slot < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT,LogCategory::Error,StatusCode::InvalidArgument);

			if(LinkedCBs.size() <= Slot)
				LinkedCBs.resize(Slot + 1);

			LinkedCBs[Slot] = CB;
			return StatusCode::Ok;
		}

		// Viewports are copied, so it's called "set" instead of link
		StatusCode SetViewport(D3D11_VIEWPORT Viewport,uint32_t Slot)
		{
			// Log and return if the resource doesn't support linking this type
			if(LogAssertAndContinue(LinkFlags & CanLinkViewport,LogCategory::Warning))
				return StatusCode::Ok;

			LogAssertWithReturn(Slot < D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE,LogCategory::Error,StatusCode::InvalidArgument);

			if(Viewports.size() <= Slot)
				Viewports.resize(Slot + 1);

			Viewports[Slot] = Viewport;
			return StatusCode::Ok;
		}

		StatusCode LinkRasterState(ID3D11RasterizerState* RS)
		{
			// Log and return if the resource doesn't support linking this type
			if(LogAssertAndContinue(LinkFlags & CanLinkRasterState,LogCategory::Warning))
				return StatusCode::Ok;

			RasterState = RS;
			IsRasterStateLinked = true;
			return StatusCode::Ok;
		}

		StatusCode LinkDepthState(ID3D11DepthStencilState* DS,UINT StencilRefVal = -1)
		{
			// Log and return if the resource doesn't support linking this type
			if(LogAssertAndContinue(LinkFlags & CanLinkDepthState,LogCategory::Warning))
				return StatusCode::Ok;

			DepthState = DS;
			IsDepthStateLinked = true;
			StencilRefValue = StencilRefVal;
			return StatusCode::Ok;
		}

		// TODO !
		// remember the prev binded stuff and add a check to make sure you are not binding twice the same stuff
		// do this for all the the independent parts that are bound
		// store some unique identifier on the device pointer an check before binding

		virtual StatusCode Bind() = 0;
		virtual StatusCode Unbind() = 0;
	protected:
		// The slot is the position on the array, this array is sent directly to the device
		vector<ID3D11ShaderResourceView*> LinkedSRVs;
		vector<ID3D11UnorderedAccessView*> LinkedUAVs;
		vector<ID3D11RenderTargetView*> LinkedRTVs;
		ID3D11DepthStencilView* LinkedDSV;
		vector<ID3D11SamplerState*> LinkedSamplers;
		vector<ID3D11Buffer*> LinkedCBs;
		vector<D3D11_VIEWPORT> Viewports;
		Device * OwnerDevice;
		bool IsDSVLinked;
		bool IsRasterStateLinked;
		bool IsDepthStateLinked;
		ID3D11RasterizerState* RasterState;
		ID3D11DepthStencilState* DepthState;
		UINT StencilRefValue;
	};

	// Utility struct to bind a resource on creation and unbind when it goes out of scope
	struct __ScopedBind
	{
		__ScopedBind(Bindable * r)
		{
			Resource = r;
			LogCheck(Resource->Bind(),LogCategory::Error);
		}
		~__ScopedBind()
		{
			LogCheck(Resource->Unbind(),LogCategory::Error);
		}
	private:
		Bindable * Resource;
	};

#define __scopedbind__inner2(resource,counter) FrameDX::__ScopedBind __bound_resource ## counter(&resource)
#define __scopedbind__inner(resource,counter) __scopedbind__inner2(resource,counter)
#define ScopedBind(resource)  __scopedbind__inner(resource,__COUNTER__)
};