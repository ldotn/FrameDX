#include "stdafx.h"
#include "OutputContext.h"

using namespace FrameDX;

StatusCode FrameDX::OutputContext::Bind()
{
	auto im = OwnerDevice->GetImmediateContext();
	
	auto dsv = LinkedDSV;
	if(!IsDSVLinked)
		im->OMGetRenderTargets(1,nullptr,&dsv);

	UINT dummy;

	if(Viewports.size() > 0)
		im->RSSetViewports(Viewports.size(),Viewports.data());
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
