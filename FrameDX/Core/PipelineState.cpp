#include "stdafx.h"
#include "PipelineState.h"
#include "../Device/Device.h"
#include "../Shader/Shaders.h"

using namespace FrameDX;

StatusCode FrameDX::PipelineState::BuildInputLayout(Device * OwnerDev)
{
	return LogCheckAndContinue(OwnerDev->GetDevice()->CreateInputLayout
	(
		Mesh.LayoutDesc->data(),
		Mesh.LayoutDesc->size(),
		Shaders[(size_t)ShaderStage::Vertex].ShaderPtr->GetBlob()->GetBufferPointer(),
		Shaders[(size_t)ShaderStage::Vertex].ShaderPtr->GetBlob()->GetBufferSize(),
		&InputLayout
	), FrameDX::LogCategory::Error);
}
