#include "stdafx.h"
#include "Mesh.h"
#include "../Core/Utils.h"

using namespace FrameDX;

const vector<D3D11_INPUT_ELEMENT_DESC> StandardVertex::LayoutDesc =
{
	{"Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,                           0,D3D11_INPUT_PER_VERTEX_DATA,0},
	{"Normal"  ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
	{"Tangent" ,0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
	{"UV"      ,0,DXGI_FORMAT_R32G32_FLOAT   ,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
};


/*
ID3D11InputLayout * in_layout;

LogCheckWithReturn(Dev->GetDevice()->CreateInputLayout
(
	VertexType::LayoutDesc.data(),
	VertexType::LayoutDesc.size(),
	Blob->GetBufferPointer(),
	Blob->GetBufferSize(),
	OutLayout
), FrameDX::LogCategory::Error);*/