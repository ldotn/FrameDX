#include "stdafx.h"
#include "Shaders.h"
#include "../Core/Utils.h"
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

		for(auto d : zip(defs,Defines))
		{
			get<0>(d)->Name = get<1>(d)->first.c_str();
			get<0>(d)->Definition = get<1>(d)->second.c_str();
		}
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
            wcout << std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes((char*)error_blob->GetBufferPointer());
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

StatusCode FrameDX::ComputeShader::Bind()
{
	auto im = OwnerDevice->GetImmediateContext();

	im->CSSetShader(Shader,nullptr,0);

	if(LinkedSRVs.size() > 0)
		im->CSSetShaderResources(0,LinkedSRVs.size(),LinkedSRVs.data());
	if(LinkedUAVs.size() > 0)
		im->CSSetUnorderedAccessViews(0,LinkedUAVs.size(),LinkedUAVs.data(),nullptr);
	if(LinkedSamplers.size() > 0)
		im->CSSetSamplers(0,LinkedSamplers.size(),LinkedSamplers.data());
	if(LinkedCBs.size() > 0)
		im->CSSetConstantBuffers(0,LinkedCBs.size(),LinkedCBs.data());

	return StatusCode::Ok;
}

size_t constexpr unbind_size()
{
	return max(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT,max(D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT,max(D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT,D3D11_1_UAV_SLOT_COUNT)));
}

StatusCode FrameDX::ComputeShader::Unbind()
{
	auto im = OwnerDevice->GetImmediateContext();
	im->CSSetShader(Shader,nullptr,0);

	void * unbind_vec[unbind_size()] = {};

	UINT dummy;
	if(LinkedSRVs.size() > 0)
		im->CSSetShaderResources(0,LinkedSRVs.size(),(ID3D11ShaderResourceView**)unbind_vec);
	if(LinkedUAVs.size() > 0)
		im->CSSetUnorderedAccessViews(0,LinkedUAVs.size(),(ID3D11UnorderedAccessView**)unbind_vec,&dummy);
	if(LinkedSamplers.size() > 0)
		im->CSSetSamplers(0,LinkedSamplers.size(),(ID3D11SamplerState**)unbind_vec);
	if(LinkedCBs.size() > 0)
		im->CSSetConstantBuffers(0,LinkedCBs.size(),(ID3D11Buffer**)unbind_vec);

	return StatusCode::Ok;	
}

StatusCode FrameDX::ComputeShader::Dispatch(Device & Dev, uint32_t SizeX, uint32_t SizeY, uint32_t SizeZ, bool IsAbsolute)
{
	uint32_t groups_x,groups_y,groups_z;
	if(IsAbsolute)
	{
		groups_x = SizeX;
		groups_y = SizeY;
		groups_z = SizeZ;
	}
	else
	{
		groups_x = ceil(SizeX,GroupSizeX);
		groups_y = ceil(SizeY,GroupSizeY);
		groups_z = ceil(SizeZ,GroupSizeZ);
	}

	LogAssertWithReturn(groups_x > 0,LogCategory::Error, StatusCode::InvalidArgument);
	LogAssertWithReturn(groups_y > 0,LogCategory::Error, StatusCode::InvalidArgument);
	LogAssertWithReturn(groups_z > 0,LogCategory::Error, StatusCode::InvalidArgument);

	Dev.GetImmediateContext()->Dispatch(groups_x,groups_y,groups_z);

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

StatusCode FrameDX::PixelShader::Bind()
{
	auto im = OwnerDevice->GetImmediateContext();

	im->PSSetShader(Shader,nullptr,0);

	if(LinkedSRVs.size() > 0)
		im->PSSetShaderResources(0,LinkedSRVs.size(),LinkedSRVs.data());
	if(LinkedSamplers.size() > 0)
		im->PSSetSamplers(0,LinkedSamplers.size(),LinkedSamplers.data());
	if(LinkedCBs.size() > 0)
		im->PSSetConstantBuffers(0,LinkedCBs.size(),LinkedCBs.data());

	return StatusCode::Ok;
}

StatusCode FrameDX::PixelShader::Unbind()
{
	auto im = OwnerDevice->GetImmediateContext();
	im->PSSetShader(Shader,nullptr,0);

	void * unbind_vec[unbind_size()] = {};

	if(LinkedSRVs.size() > 0)
		im->PSSetShaderResources(0,LinkedSRVs.size(),(ID3D11ShaderResourceView**)unbind_vec);
	if(LinkedSamplers.size() > 0)
		im->PSSetSamplers(0,LinkedSamplers.size(),(ID3D11SamplerState**)unbind_vec);
	if(LinkedCBs.size() > 0)
		im->PSSetConstantBuffers(0,LinkedCBs.size(),(ID3D11Buffer**)unbind_vec);

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

StatusCode FrameDX::VertexShader::Bind()
{
	auto im = OwnerDevice->GetImmediateContext();

	im->VSSetShader(Shader,nullptr,0);

	if(LinkedSRVs.size() > 0)
		im->VSSetShaderResources(0,LinkedSRVs.size(),LinkedSRVs.data());
	if(LinkedSamplers.size() > 0)
		im->VSSetSamplers(0,LinkedSamplers.size(),LinkedSamplers.data());
	if(LinkedCBs.size() > 0)
		im->VSSetConstantBuffers(0,LinkedCBs.size(),LinkedCBs.data());

	return StatusCode::Ok;
}

StatusCode FrameDX::VertexShader::Unbind()
{
	auto im = OwnerDevice->GetImmediateContext();
	im->VSSetShader(Shader,nullptr,0);

	void * unbind_vec[unbind_size()] = {};

	if(LinkedSRVs.size() > 0)
		im->VSSetShaderResources(0,LinkedSRVs.size(),(ID3D11ShaderResourceView**)unbind_vec);
	if(LinkedSamplers.size() > 0)
		im->VSSetSamplers(0,LinkedSamplers.size(),(ID3D11SamplerState**)unbind_vec);
	if(LinkedCBs.size() > 0)
		im->VSSetConstantBuffers(0,LinkedCBs.size(),(ID3D11Buffer**)unbind_vec);

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

StatusCode FrameDX::GeometryShader::Bind()
{
	auto im = OwnerDevice->GetImmediateContext();

	im->GSSetShader(Shader,nullptr,0);

	if(LinkedSRVs.size() > 0)
		im->GSSetShaderResources(0,LinkedSRVs.size(),LinkedSRVs.data());
	if(LinkedSamplers.size() > 0)
		im->GSSetSamplers(0,LinkedSamplers.size(),LinkedSamplers.data());
	if(LinkedCBs.size() > 0)
		im->GSSetConstantBuffers(0,LinkedCBs.size(),LinkedCBs.data());

	return StatusCode::Ok;
}

StatusCode FrameDX::GeometryShader::Unbind()
{
	auto im = OwnerDevice->GetImmediateContext();
	im->GSSetShader(Shader,nullptr,0);

	void * unbind_vec[unbind_size()] = {};

	if(LinkedSRVs.size() > 0)
		im->GSSetShaderResources(0,LinkedSRVs.size(),(ID3D11ShaderResourceView**)unbind_vec);
	if(LinkedSamplers.size() > 0)
		im->GSSetSamplers(0,LinkedSamplers.size(),(ID3D11SamplerState**)unbind_vec);
	if(LinkedCBs.size() > 0)
		im->GSSetConstantBuffers(0,LinkedCBs.size(),(ID3D11Buffer**)unbind_vec);

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

StatusCode FrameDX::HullShader::Bind()
{
	auto im = OwnerDevice->GetImmediateContext();

	im->HSSetShader(Shader,nullptr,0);

	if(LinkedSRVs.size() > 0)
		im->HSSetShaderResources(0,LinkedSRVs.size(),LinkedSRVs.data());
	if(LinkedSamplers.size() > 0)
		im->HSSetSamplers(0,LinkedSamplers.size(),LinkedSamplers.data());
	if(LinkedCBs.size() > 0)
		im->HSSetConstantBuffers(0,LinkedCBs.size(),LinkedCBs.data());

	return StatusCode::Ok;
}

StatusCode FrameDX::HullShader::Unbind()
{
	auto im = OwnerDevice->GetImmediateContext();
	im->HSSetShader(Shader,nullptr,0);

	void * unbind_vec[unbind_size()] = {};

	if(LinkedSRVs.size() > 0)
		im->HSSetShaderResources(0,LinkedSRVs.size(),(ID3D11ShaderResourceView**)unbind_vec);
	if(LinkedSamplers.size() > 0)
		im->HSSetSamplers(0,LinkedSamplers.size(),(ID3D11SamplerState**)unbind_vec);
	if(LinkedCBs.size() > 0)
		im->HSSetConstantBuffers(0,LinkedCBs.size(),(ID3D11Buffer**)unbind_vec);

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

StatusCode FrameDX::DomainShader::Bind()
{
	auto im = OwnerDevice->GetImmediateContext();

	im->DSSetShader(Shader,nullptr,0);

	if(LinkedSRVs.size() > 0)
		im->DSSetShaderResources(0,LinkedSRVs.size(),LinkedSRVs.data());
	if(LinkedSamplers.size() > 0)
		im->DSSetSamplers(0,LinkedSamplers.size(),LinkedSamplers.data());
	if(LinkedCBs.size() > 0)
		im->DSSetConstantBuffers(0,LinkedCBs.size(),LinkedCBs.data());

	return StatusCode::Ok;
}

StatusCode FrameDX::DomainShader::Unbind()
{
	auto im = OwnerDevice->GetImmediateContext();
	im->DSSetShader(Shader,nullptr,0);

	void * unbind_vec[unbind_size()] = {};

	if(LinkedSRVs.size() > 0)
		im->DSSetShaderResources(0,LinkedSRVs.size(),(ID3D11ShaderResourceView**)unbind_vec);
	if(LinkedSamplers.size() > 0)
		im->DSSetSamplers(0,LinkedSamplers.size(),(ID3D11SamplerState**)unbind_vec);
	if(LinkedCBs.size() > 0)
		im->DSSetConstantBuffers(0,LinkedCBs.size(),(ID3D11Buffer**)unbind_vec);

	return StatusCode::Ok;	
}