#pragma once
#include "stdafx.h"
#include "../Core/Core.h"

namespace FrameDX
{
	class Device;

	class Shader
	{
	public:
		virtual StatusCode CreateFromFile( Device * OwnerDevice,
										   wstring FilePath,
										   string EntryPoint,
										   bool FullDebug = false,
										   vector<pair<string, string>> Defines = {}) = 0;
		virtual void * GetShaderPointer() = 0;
		virtual ID3DBlob* GetBlob() { return nullptr; }
	protected:
		Device * OwnerDevice;
	};

	class ComputeShader : public Shader
	{
	public:
		ComputeShader() 
		{ 
			Shader = nullptr;
		}

		virtual StatusCode CreateFromFile( Device * OwnerDevice,
										   wstring FilePath,
										   string EntryPoint,
										   bool FullDebug = false,
										   vector<pair<string, string>> Defines = {}) override;
		virtual void * GetShaderPointer() { return Shader; }

		// Makes dispatch easier later on
		uint32_t GroupSizeX;
		uint32_t GroupSizeY;
		uint32_t GroupSizeZ;
	private:
		ID3D11ComputeShader * Shader;
	};

	class PixelShader : public Shader
	{
	public:
		PixelShader() 
		{ 
			Shader = nullptr;
		}

		virtual StatusCode CreateFromFile( Device * OwnerDevice,
										   wstring FilePath,
										   string EntryPoint,
										   bool FullDebug = false,
										   vector<pair<string, string>> Defines = {}) override;
		virtual void * GetShaderPointer() override { return Shader; }
	private:
		ID3D11PixelShader * Shader;
	};

	class VertexShader : public Shader
	{
	public:
		VertexShader() 
		{ 
			Shader = nullptr;
		}

		virtual StatusCode CreateFromFile( Device * OwnerDevice,
										   wstring FilePath,
										   string EntryPoint,
										   bool FullDebug = false,
										   vector<pair<string, string>> Defines = {}) override;
		virtual void * GetShaderPointer() override { return Shader; }
		virtual ID3DBlob* GetBlob() override { return Blob; }
	private:
		ID3D11VertexShader * Shader;
		ID3DBlob * Blob; // Used to create the input layout
	};

	class GeometryShader : public Shader
	{
	public:
		GeometryShader() 
		{ 
			Shader = nullptr;
		}

		virtual StatusCode CreateFromFile( Device * OwnerDevice,
										   wstring FilePath,
										   string EntryPoint,
										   bool FullDebug = false,
										   vector<pair<string, string>> Defines = {}) override;
		virtual void * GetShaderPointer() override { return Shader; }
	private:
		ID3D11GeometryShader * Shader;
	};

	class HullShader : public Shader
	{
	public:
		HullShader() 
		{ 
			Shader = nullptr;
		}

		virtual StatusCode CreateFromFile( Device * OwnerDevice,
										   wstring FilePath,
										   string EntryPoint,
										   bool FullDebug = false,
										   vector<pair<string, string>> Defines = {}) override;
		virtual void * GetShaderPointer() override { return Shader; }
	private:
		ID3D11HullShader * Shader;
	};

	class DomainShader : public Shader
	{
	public:
		DomainShader() 
		{ 
			Shader = nullptr;
		}

		virtual StatusCode CreateFromFile( Device * OwnerDevice,
										   wstring FilePath,
										   string EntryPoint,
										   bool FullDebug = false,
										   vector<pair<string, string>> Defines = {}) override;
		virtual void * GetShaderPointer() override { return Shader; }
	private:
		ID3D11DomainShader * Shader;
	};
}