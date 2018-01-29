#include "stdafx.h"
#include "OutputContext.h"

using namespace FrameDX;

StatusCode FrameDX::OutputContext::Bind()
{
	auto im = OwnerDevice->GetImmediateContext();
	
	auto dsv = LinkedDSV;
	if(!IsDSVLinked)
		hay que llamar a release porque el get le suma al ref count
		im->OMGetRenderTargets(1,nullptr,&dsv);

	// 5 pipeline stages that can use SRVs : vertex, hull, domain, geometry and pixel
	bool srvs_to_unbind[5][D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
	bool unbind_needed[5] = {};

	auto needs_unbind = [&](auto& Resources)
	{
		bool unbind_found = false;

		for (int i = 0; i < Resources.size(); i++)
		{
			auto& r = Resources[i];

			// Check there are no in-out conflicts
			auto id = GetResourceID(r);
			pair<bool, Device::BindInfo> info;
			if (OwnerDevice->IsResourceBound(id, &info) && info.second.Usage == Device::BindInfo::Input)
			{
				// Check if the binding is active
				if (info.first)
					// In-out conflict, log a warning
					LogMsg(wstring(L"The resource on slot ") + to_wstring(i) + wstring(L"is still bound for input when trying to bound it for output"), LogCategory::Warning);
				else
				{
					// It's bound for input, but it's not active, so mark it on the unbind array
					srvs_to_unbind[info.second.ShaderStage][info.second.Slot] = true;
					unbind_found = true;
					unbind_needed[info.second.ShaderStage] = true;
				}
			}
		}

		return unbind_found;
	};

	// Check for conflicts and unbind if necessary
	if (needs_unbind(LinkedRTVs) ||
		needs_unbind(LinkedUAVs) ||
		(IsDSVLinked && needs_unbind(vector<decltype(dsv)>{ dsv })))
	{
		if(unbind_needed[Device::BindInfo::Vertex])
		{
			ID3D11ShaderResourceView* buff[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
			im->VSGetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, buff);

			int last_index = 0;
			for(int i = 0; i < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; i++)
			{
				if (srvs_to_unbind[i])
				{
					// The Get increases the ref counter, need to call release
					if (buff[i])
						buff[i]->Release();
					buff[i] = nullptr;

					if (buff[i] && i > last_index)
						last_index = i;
				}
			}

			im->VSSetShaderResources(0, last_index + 1, buff);

			for (int i = 0; i < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; i++)
				if (buff[i]) buff[i]->Release();
		}
	}

	if(Viewports.size() > 0)
		im->RSSetViewports(Viewports.size(),Viewports.data());

	UINT dummy;
	if(LinkedRTVs.size() > 0)
	{
		if(LinkedUAVs.size() > 0)
			im->OMSetRenderTargetsAndUnorderedAccessViews(LinkedRTVs.size(),LinkedRTVs.data(),dsv,LinkedRTVs.size(),LinkedUAVs.size(),LinkedUAVs.data(),&dummy);
		else
			im->OMSetRenderTargets(LinkedRTVs.size(),LinkedRTVs.data(),dsv);
	}
	else if(LinkedUAVs.size() > 0)
		im->OMSetRenderTargetsAndUnorderedAccessViews(0,nullptr,dsv,0,LinkedUAVs.size(),LinkedUAVs.data(),&dummy);

	if(IsRasterStateLinked)
		im->RSSetState(RasterState);
	if(IsDepthStateLinked)
		im->OMSetDepthStencilState(DepthState,StencilRefValue);

	return StatusCode::Ok;
}

StatusCode FrameDX::OutputContext::Unbind()
{
	auto im = OwnerDevice->GetImmediateContext();

	ID3D11DepthStencilView* dsv = nullptr;
	if(!IsDSVLinked) // If it's not linked, need this to make sure the old one remains
		im->OMGetRenderTargets(1,nullptr,&dsv);
	
	UINT dummy;

	const int count = max(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
	void * unbind_vec[count] = {};

	if(Viewports.size() > 0)
		im->RSSetViewports(Viewports.size(),(D3D11_VIEWPORT*)unbind_vec);
	if(LinkedRTVs.size() > 0)
	{
		if(LinkedUAVs.size() > 0)
			im->OMSetRenderTargetsAndUnorderedAccessViews(LinkedRTVs.size(),(ID3D11RenderTargetView**)unbind_vec,dsv,LinkedRTVs.size(),LinkedUAVs.size(),(ID3D11UnorderedAccessView**)unbind_vec,&dummy);
		else
			im->OMSetRenderTargets(LinkedRTVs.size(),(ID3D11RenderTargetView**)unbind_vec,dsv);
	}
	else if(LinkedUAVs.size() > 0)
	{
		// Get all render targets to not override them in case they arent linked
		ID3D11RenderTargetView* rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
		im->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,rtvs,nullptr);

		int count = 0;
		while(rtvs[count] && count < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT) count++;

		im->OMSetRenderTargetsAndUnorderedAccessViews(count,rtvs,dsv,0,LinkedUAVs.size(),LinkedUAVs.data(),&dummy);
	}

	if(IsRasterStateLinked)
		im->RSSetState(nullptr);
	if(IsDepthStateLinked)
		im->OMSetDepthStencilState(nullptr,-1);
		
	return StatusCode::Ok;
}
