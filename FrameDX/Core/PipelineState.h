#pragma once
#include "stdafx.h"
#include "Core.h"
#include "Log.h"

namespace FrameDX
{
	enum class ShaderStage { Vertex, Hull, Domain, Geometry, Pixel, Compute, _count };

	// Stores the context for one shader
	// It consists on the shader, an srv table, a cb table, a sampler table
	// It assumes all srvs and cb are bound with contiguous indexes, starting at 0
	class Shader;
	struct ShaderContext
	{
		ShaderContext() :
			ShaderPtr(nullptr)
		{}

		Shader * ShaderPtr;
		vector<ID3D11ShaderResourceView*> ResourcesTable;
		vector<ID3D11Buffer*> ConstantBuffersTable;
		vector<ID3D11SamplerState*> SamplersTable;
	};

	// Stores the context for one mesh
	// It consists on an index buffer, a vertex buffer, the input layout, the primitive type and the index size
	// It assumes a single vertex buffer, and that all offsets are 0
	struct MeshContext
	{
		MeshContext() :
			IndexBuffer(nullptr),
			VertexBuffer(nullptr),
			PrimitiveType(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST),
			IndexFormat(DXGI_FORMAT_R32_UINT)
		{}

		ID3D11Buffer* IndexBuffer;
		ID3D11Buffer* VertexBuffer;
		UINT VertexStride;
		const vector<D3D11_INPUT_ELEMENT_DESC>* LayoutDesc; // Needs to always be valid. Usually points to a static resource
		D3D11_PRIMITIVE_TOPOLOGY PrimitiveType;
		DXGI_FORMAT IndexFormat;
	};

	// Stores the info to fully configure outputs
	// It consists on the RTVs, UAVs, DSV, viewports, depth state, blend state, culling state, raster state
	// UAVs are bound right after RTVs
	// Sample mask is assumed at -1
	struct OutputContext
	{
		OutputContext() : 
			DSV(nullptr),
			DepthStencilState(nullptr),
			BlendState(nullptr),
			RasterState(nullptr),
			BlendFactors { 1.0f, 1.0f, 1.0f, 1.0f }
		{}

		vector<ID3D11RenderTargetView*> RTVs;
		vector<ID3D11UnorderedAccessView*> UAVs;
		vector<ID3D11UnorderedAccessView*> ComputeShaderUAVs;
		vector<D3D11_VIEWPORT> Viewports;
		ID3D11DepthStencilView * DSV;
		ID3D11DepthStencilState * DepthStencilState;
		ID3D11BlendState * BlendState;
		ID3D11RasterizerState* RasterState;
		UINT StencilRef;
		float BlendFactors[4];
	};

	class PipelineState
	{
	public:
		ShaderContext Shaders[(size_t)ShaderStage::_count];
		MeshContext Mesh;
		OutputContext Output;
		
		// This depends on both the Mesh and the VS, so it's created by this object
		ID3D11InputLayout * InputLayout;
		StatusCode BuildInputLayout(class Device * OwnerDev);
	};
}
