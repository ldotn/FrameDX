#pragma once
#include "stdafx.h"
#include "../Core/Core.h"
#include "../Core/Binding.h"

namespace FrameDX
{
	class ComputeShader : public Bindable
	{
	public:
		ComputeShader() 
		{ 
			LinkFlags = CanLinkSRV | CanLinkUAV | CanLinkSampler | CanLinkCB;
		}

		StatusCode CreateFromFile( Device * OwnerDevice, 
								   wstring FilePath,
								   string EntryPoint,
								   bool FullDebug = false, 
								   vector<pair<string,string>> Defines = {});

		virtual StatusCode Bind() final override;
		virtual StatusCode Unbind() final override;

		// Dispatches the shader and if IsAbsolute is false (default) it divides the size by GroupSize
		StatusCode Dispatch(Device& Dev,uint32_t SizeX,uint32_t SizeY,uint32_t SizeZ = 1,bool IsAbsolute = false);
	private:
		ID3D11ComputeShader * Shader;
		uint32_t GroupSizeX;
		uint32_t GroupSizeY;
		uint32_t GroupSizeZ;
	};

	class PixelShader : public Bindable
	{
	public:
		PixelShader() 
		{ 
			LinkFlags = CanLinkSRV | CanLinkSampler | CanLinkCB;
		}

		StatusCode CreateFromFile( Device * OwnerDevice, 
								   wstring FilePath,
								   string EntryPoint,
								   bool FullDebug = false, 
								   vector<pair<string,string>> Defines = {});

		virtual StatusCode Bind() final override;
		virtual StatusCode Unbind() final override;
	private:
		ID3D11PixelShader * Shader;
	};

	class VertexShader : public Bindable
	{
	public:
		VertexShader() 
		{ 
			LinkFlags = CanLinkSRV | CanLinkSampler | CanLinkCB;
		}

		StatusCode CreateFromFile( Device * OwnerDevice, 
								   wstring FilePath,
								   string EntryPoint,
								   bool FullDebug = false, 
								   vector<pair<string,string>> Defines = {});

		virtual StatusCode Bind() final override;
		virtual StatusCode Unbind() final override;
	private:
		ID3D11VertexShader * Shader;
		ID3DBlob * Blob; // Used to create the input layout
	};

	class GeometryShader : public Bindable
	{
	public:
		GeometryShader() 
		{ 
			LinkFlags = CanLinkSRV | CanLinkSampler | CanLinkCB;
		}

		StatusCode CreateFromFile( Device * OwnerDevice, 
								   wstring FilePath,
								   string EntryPoint,
								   bool FullDebug = false, 
								   vector<pair<string,string>> Defines = {});

		virtual StatusCode Bind() final override;
		virtual StatusCode Unbind() final override;
	private:
		ID3D11GeometryShader * Shader;
	};

	class HullShader : public Bindable
	{
	public:
		HullShader() 
		{ 
			LinkFlags = CanLinkSRV | CanLinkSampler | CanLinkCB;
		}

		StatusCode CreateFromFile( Device * OwnerDevice, 
								   wstring FilePath,
								   string EntryPoint,
								   bool FullDebug = false, 
								   vector<pair<string,string>> Defines = {});

		virtual StatusCode Bind() final override;
		virtual StatusCode Unbind() final override;
	private:
		ID3D11HullShader * Shader;
	};

	class DomainShader : public Bindable
	{
	public:
		DomainShader() 
		{ 
			LinkFlags = CanLinkSRV | CanLinkSampler | CanLinkCB;
		}

		StatusCode CreateFromFile( Device * OwnerDevice, 
								   wstring FilePath,
								   string EntryPoint,
								   bool FullDebug = false, 
								   vector<pair<string,string>> Defines = {});

		virtual StatusCode Bind() final override;
		virtual StatusCode Unbind() final override;
	private:
		ID3D11DomainShader * Shader;
	};
}