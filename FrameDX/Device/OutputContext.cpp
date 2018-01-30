#include "stdafx.h"
#include "OutputContext.h"

using namespace FrameDX;

StatusCode FrameDX::OutputContext::Bind()
{
	auto im = OwnerDevice->GetImmediateContext();
	
	auto dsv = LinkedDSV;
	if(!IsDSVLinked)
		//hay que llamar a release porque el get le suma al ref count
		im->OMGetRenderTargets(1,nullptr,&dsv);

	// 5 pipeline stages that can use SRVs : vertex, hull, domain, geometry and pixel
	bool srvs_to_unbind[Device::BindInfo::StagesNum][D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
	bool unbind_needed[Device::BindInfo::StagesNum] = {};

	// Check for conflicts and unbind if necessary
	if (BuildUnbindFlagVector(OwnerDevice,LinkedRTVs,srvs_to_unbind,unbind_needed) ||
		BuildUnbindFlagVector(OwnerDevice,LinkedUAVs,srvs_to_unbind,unbind_needed) ||
		(IsDSVLinked && BuildUnbindFlagVector(OwnerDevice,vector<decltype(dsv)>{ dsv },srvs_to_unbind,unbind_needed)))
	{
		function<void(UINT,UINT, ID3D11ShaderResourceView **)> gets[] =
		{
			[im](auto StartSlot, auto NumViews, auto SRVs) { return im->VSGetShaderResources(StartSlot, NumViews, SRVs); },
			[im](auto StartSlot, auto NumViews, auto SRVs) { return im->HSGetShaderResources(StartSlot, NumViews, SRVs); },
			[im](auto StartSlot, auto NumViews, auto SRVs) { return im->DSGetShaderResources(StartSlot, NumViews, SRVs); },
			[im](auto StartSlot, auto NumViews, auto SRVs) { return im->GSGetShaderResources(StartSlot, NumViews, SRVs); },
			[im](auto StartSlot, auto NumViews, auto SRVs) { return im->PSGetShaderResources(StartSlot, NumViews, SRVs); }
		};

		function<void(UINT, UINT, ID3D11ShaderResourceView **)> sets[] =
		{
			[im](auto StartSlot, auto NumViews, auto SRVs) { return im->VSSetShaderResources(StartSlot, NumViews, SRVs); },
			[im](auto StartSlot, auto NumViews, auto SRVs) { return im->HSSetShaderResources(StartSlot, NumViews, SRVs); },
			[im](auto StartSlot, auto NumViews, auto SRVs) { return im->DSSetShaderResources(StartSlot, NumViews, SRVs); },
			[im](auto StartSlot, auto NumViews, auto SRVs) { return im->GSSetShaderResources(StartSlot, NumViews, SRVs); },
			[im](auto StartSlot, auto NumViews, auto SRVs) { return im->PSSetShaderResources(StartSlot, NumViews, SRVs); }
		};

		for(int i = 0;i < Device::BindInfo::StagesNum;i++)
			if (unbind_needed[i])
				SparseUnbindSRVs( srvs_to_unbind[i], gets[i], sets[i]);
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

	if (!IsDSVLinked)
		dsv->Release();

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
