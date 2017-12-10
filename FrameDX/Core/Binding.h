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
	public:
		StatusCode LinkSRV(ID3D11ShaderResourceView* SRV,uint32_t Slot)
		{
			LogAssertWithReturn(Slot < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT,LogCategory::Error,StatusCode::InvalidArgument);

			if(LinkedSRVs.size() <= Slot)
				LinkedSRVs.resize(Slot + 1);

			LinkedSRVs[Slot] = SRV;
			return StatusCode::Ok;
		}

		StatusCode LinkUAV(ID3D11UnorderedAccessView* UAV,uint32_t Slot)
		{ 
			LogAssertWithReturn(Slot < ((OwnerDevice->GetDeviceVersion() > 0) ? D3D11_1_UAV_SLOT_COUNT : D3D11_PS_CS_UAV_REGISTER_COUNT),LogCategory::Error,StatusCode::InvalidArgument);

			if(LinkedUAVs.size() <= Slot)
				LinkedUAVs.resize(Slot + 1);

			LinkedUAVs[Slot] = UAV;
			return StatusCode::Ok;
		}

		StatusCode LinkRTV(ID3D11RenderTargetView* RTV,uint32_t Slot)
		{
			LogAssertWithReturn(Slot < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,LogCategory::Error,StatusCode::InvalidArgument);

			if(LinkedRTVs.size() <= Slot)
				LinkedRTVs.resize(Slot + 1);

			LinkedRTVs[Slot] = RTV;
			return StatusCode::Ok;
		}

		StatusCode LinkDSV(ID3D11DepthStencilView* DSV)
		{
			LinkedDSV = DSV;
			return StatusCode::Ok;
		}

		StatusCode LinkSampler(ID3D11SamplerState* Sampler,uint32_t Slot)
		{
			LogAssertWithReturn(Slot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT,LogCategory::Error,StatusCode::InvalidArgument);

			if(LinkedSamplers.size() <= Slot)
				LinkedSamplers.resize(Slot + 1);

			LinkedSamplers[Slot] = Sampler;
			return StatusCode::Ok;
		}

		StatusCode LinkConstantBuffer(ID3D11Buffer* CB,uint32_t Slot)
		{
			LogAssertWithReturn(Slot < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT,LogCategory::Error,StatusCode::InvalidArgument);

			if(LinkedCBs.size() <= Slot)
				LinkedCBs.resize(Slot + 1);

			LinkedCBs[Slot] = CB;
			return StatusCode::Ok;
		}

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
		Device * OwnerDevice;
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

#define ScopedBind(resource) FrameDX::__ScopedBind ___ ## __COUNTER__ ## bound_resource___(&resource)
	//inline __ScopedBind ScopedBind(Bindable * ptr){ return __ScopedBind(ptr); }
};