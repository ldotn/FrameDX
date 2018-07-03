#include "stdafx.h"
#include "Shaders.h"
#include "../Core/Utils.h"
#include "../Device/Device.h"

using namespace FrameDX;

ID3DBlob * ReadFile(ID3D11Device* device,
					vector<pair<string,string>>& Defines,
					bool FullDebug, 
					wstring& FilePath,
					string& EntryPoint,
					const string& Target,
					StatusCode& status)
{
	uint32_t flags = 0;

	vector<D3D_SHADER_MACRO> defs;
	if(Defines.size() > 0)
	{
		defs.resize(Defines.size());

		// This requires the latest standard
		// zip returns references
		for(auto [dx_def,in_def] : zip(defs,Defines))
		{
			dx_def.Name = in_def.first.c_str();
			dx_def.Definition = in_def.second.c_str();
		}

		// Push the null terminator
		defs.push_back({ nullptr,nullptr });
	}
	
	if(FullDebug)
	{
		 flags |= D3DCOMPILE_DEBUG;
	}

#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    flags |= D3DCOMPILE_DEBUG;

	if(flags)
	{
		flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
		flags |= D3DCOMPILE_ENABLE_STRICTNESS;
		flags |= D3DCOMPILE_IEEE_STRICTNESS;
	}
#endif

    ID3DBlob* shader_blob = nullptr;
    ID3DBlob* error_blob = nullptr;
	status = LogCheckAndContinue(D3DCompileFromFile( FilePath.c_str(), defs.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE,
											         EntryPoint.c_str(), Target.c_str(),
													 flags, 0, &shader_blob, &error_blob ),LogCategory::Error);
    if(status != StatusCode::Ok)
	{
        if(error_blob)
        {
			// D3DCompileFromFile returns an ASCII string for the error string
			// To be modern, I'm using UTF-8 on the output
			// That means I need to convert
			// BUT, if one assumes the error string is going to only use 7-bit ASCII characters
			//		(something that seems reasonable)
			// then you can just convert as is, because UTF-8 is backwards compatible with 7-bit ASCII
			wstring error_string;
			error_string.resize(error_blob->GetBufferSize(), L' ');
			char* raw_error_string = (char*)error_blob->GetBufferPointer();

			// Traverse the string and copy the values.
			// Ignore the ones that are outside the 7-bit ASCII range
			// Those characters are replaced by a whitespace
			for (auto [idx, c] : enumerate(error_string))
			{
				auto new_char = raw_error_string[idx];
				if (new_char < 127)
					c = new_char;
			}

			wcout << error_string;
            error_blob->Release();
        }

        if(shader_blob)
           shader_blob->Release();
		return nullptr;
	}

	return shader_blob;
}

StatusCode FrameDX::ComputeShader::CreateFromFile(Device * device, wstring FilePath, string EntryPoint, bool FullDebug,vector<pair<string,string>> Defines)
{
	StatusCode status;
	ID3DBlob * shader_blob = ReadFile(device->GetDevice(),Defines,FullDebug,FilePath,EntryPoint,"cs_5_0",status);
	
	if(status != StatusCode::Ok) return status;

	OwnerDevice = device;
	LogCheckWithReturn(OwnerDevice->GetDevice()->CreateComputeShader(shader_blob->GetBufferPointer(),shader_blob->GetBufferSize(),nullptr,&Shader),LogCategory::Error);

	ID3D11ShaderReflection* reflector = NULL; 
	LogCheckWithReturn(D3DReflect( shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**) &reflector),LogCategory::Error);
	
	reflector->GetThreadGroupSize(&GroupSizeX,&GroupSizeY,&GroupSizeZ);
	return StatusCode::Ok;
}

StatusCode FrameDX::PixelShader::CreateFromFile(Device * device, wstring FilePath, string EntryPoint, bool FullDebug,vector<pair<string,string>> Defines)
{
	StatusCode status;
	ID3DBlob * shader_blob = ReadFile(device->GetDevice(),Defines,FullDebug,FilePath,EntryPoint,"ps_5_0",status);
	
	if(status != StatusCode::Ok) return status;

	OwnerDevice = device;
	LogCheckWithReturn(OwnerDevice->GetDevice()->CreatePixelShader(shader_blob->GetBufferPointer(),shader_blob->GetBufferSize(),nullptr,&Shader),LogCategory::Error);

	return StatusCode::Ok;
}


StatusCode FrameDX::VertexShader::CreateFromFile(Device * device, wstring FilePath, string EntryPoint, bool FullDebug,vector<pair<string,string>> Defines)
{
	StatusCode status;
	Blob = ReadFile(device->GetDevice(),Defines,FullDebug,FilePath,EntryPoint,"vs_5_0",status);
	
	if(status != StatusCode::Ok) return status;

	OwnerDevice = device;
	LogCheckWithReturn(OwnerDevice->GetDevice()->CreateVertexShader(Blob->GetBufferPointer(),Blob->GetBufferSize(),nullptr,&Shader),LogCategory::Error);

	return StatusCode::Ok;
}


StatusCode FrameDX::GeometryShader::CreateFromFile(Device * device, wstring FilePath, string EntryPoint, bool FullDebug,vector<pair<string,string>> Defines)
{
	StatusCode status;
	ID3DBlob * shader_blob = ReadFile(device->GetDevice(),Defines,FullDebug,FilePath,EntryPoint,"gs_5_0",status);
	
	if(status != StatusCode::Ok) return status;

	OwnerDevice = device;
	LogCheckWithReturn(OwnerDevice->GetDevice()->CreateGeometryShader(shader_blob->GetBufferPointer(),shader_blob->GetBufferSize(),nullptr,&Shader),LogCategory::Error);

	return StatusCode::Ok;
}

StatusCode FrameDX::HullShader::CreateFromFile(Device * device, wstring FilePath, string EntryPoint, bool FullDebug,vector<pair<string,string>> Defines)
{
	StatusCode status;
	ID3DBlob * shader_blob = ReadFile(device->GetDevice(),Defines,FullDebug,FilePath,EntryPoint,"hs_5_0",status);
	
	if(status != StatusCode::Ok) return status;

	OwnerDevice = device;
	LogCheckWithReturn(OwnerDevice->GetDevice()->CreateHullShader(shader_blob->GetBufferPointer(),shader_blob->GetBufferSize(),nullptr,&Shader),LogCategory::Error);

	return StatusCode::Ok;
}

StatusCode FrameDX::DomainShader::CreateFromFile(Device * device, wstring FilePath, string EntryPoint, bool FullDebug,vector<pair<string,string>> Defines)
{
	StatusCode status;
	ID3DBlob * shader_blob = ReadFile(device->GetDevice(),Defines,FullDebug,FilePath,EntryPoint,"ds_5_0",status);
	
	if(status != StatusCode::Ok) return status;

	OwnerDevice = device;
	LogCheckWithReturn(OwnerDevice->GetDevice()->CreateDomainShader(shader_blob->GetBufferPointer(),shader_blob->GetBufferSize(),nullptr,&Shader),LogCategory::Error);

	return StatusCode::Ok;
}