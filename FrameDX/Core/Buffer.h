#pragma once
#include "Utils.h"

namespace FrameDX
{
	template<typename T>
	class StructuredBuffer
	{
		static_assert(sizeof(T) % 16 == 0, "T needs to be 16-bytes aligned");

		vector<T> Data;
		ID3D11Buffer * Buffer;
		ID3D11ShaderResourceView * SRV;
		ID3D11UnorderedAccessView * UAV;
		
	public:
		StructuredBuffer()
		{
			Buffer = nullptr;
			SRV = nullptr;
			UAV = nullptr;
		}
		~StructuredBuffer()
		{
			if(Buffer)
				Buffer->Release();
			if(SRV)
				SRV->Release();
			if(UAV)
				UAV->Release();
		}
		StructuredBuffer(const StructuredBuffer&) = delete;
		StructuredBuffer(StructuredBuffer&& rhs) :
			Data(move(rhs.Data)),
			Buffer(rhs.Buffer),
			SRV(rhs.SRV),
			UAV(rhs.UAV)
		{
			rhs.Buffer = nullptr;
			rhs.SRV = nullptr;
			rhs.UAV = nullptr;
		}
			

		StructuredBuffer& operator=(const StructuredBuffer&) = delete;
		StructuredBuffer& operator=(StructuredBuffer&& rhs)
		{
			Data = move(rhs);
			Buffer = rhs.Buffer;
			SRV = rhs.SRV;
			UAV = rhs.UAV;

			rhs.Buffer = nullptr;
			rhs.SRV = nullptr;
			rhs.UAV = nullptr;
		}

		auto GetBuffer() const { return Buffer; }
		auto GetSRV() const  { return SRV; }
		auto GetUAV() const  { return UAV; }
		const vector<T>& GetRawData() const  { return Data; }

		StatusCode Build(size_t Size, Device& Dev, vector<T> InData = {}, D3D11_USAGE Usage = D3D11_USAGE_IMMUTABLE, bool NeedsUAV = false)
		{
			// Can't have an UAV and Dynamic usage
			// [https://docs.microsoft.com/es-es/windows/desktop/api/d3d11/ne-d3d11-d3d11_usage]
			if (Usage == D3D11_USAGE_DYNAMIC && NeedsUAV)
				return StatusCode::InvalidArgument;

			Data = move(InData);
			UINT bind_flags = NeedsUAV ?
				D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS : D3D11_BIND_SHADER_RESOURCE;

			UINT CPUAccessFlags = 0;
			if (Usage == D3D11_USAGE_DYNAMIC)
				CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			else if (Usage == D3D11_USAGE_STAGING)
				CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			
			auto status = LogCheckAndContinue(CreateBuffer<T>(Size, Dev, bind_flags, &Buffer, Data, Usage, "", D3D11_RESOURCE_MISC_FLAG::D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, CPUAccessFlags), LogCategory::Error);
			if (status != StatusCode::Ok)
				return status;

			D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
			desc.Buffer.NumElements = Data.size();

			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			
			status = LogCheckAndContinue(Dev.GetDevice()->CreateShaderResourceView(Buffer, &desc, &SRV), LogCategory::Error);
			if (status != StatusCode::Ok)
				return status;

			if (NeedsUAV)
			{
				D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
				uav_desc.Buffer.NumElements = Data.size();
				uav_desc.Format = DXGI_FORMAT_UNKNOWN;
				uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

				status = LogCheckAndContinue(Dev.GetDevice()->CreateUnorderedAccessView(Buffer, &uav_desc, &UAV), LogCategory::Error);
				if (status != StatusCode::Ok)
					return status;
			}

			return StatusCode::Ok;
		}
	};

	template<typename T>
	struct ConstantBuffer
	{
		static_assert(sizeof(T) % 16 == 0, "T needs to be 16-bytes aligned");

		ID3D11Buffer * Buffer;
		
		ConstantBuffer() 
		{
			Buffer = nullptr;
		}
		~ConstantBuffer()
		{
			if(Buffer)
				Buffer->Release();
		}
		ConstantBuffer(const ConstantBuffer&) = delete;
		ConstantBuffer(ConstantBuffer&& rhs) :
			Buffer(rhs.Buffer)
		{
			rhs.Buffer = nullptr;
		}
			

		ConstantBuffer& operator=(const ConstantBuffer&) = delete;
		ConstantBuffer& operator=(ConstantBuffer&& rhs)
		{
			Buffer = rhs.Buffer;

			rhs.Buffer = nullptr;
		}

		StatusCode Build(Device& Dev)
		{
			D3D11_BUFFER_DESC cb_desc;

			cb_desc.ByteWidth = sizeof(T);
			cb_desc.Usage = D3D11_USAGE_DYNAMIC;
			cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cb_desc.MiscFlags = 0;
			cb_desc.StructureByteStride = 0;

			return LogCheckAndContinue(Dev.GetDevice()->
				CreateBuffer(&cb_desc, nullptr, &Buffer), FrameDX::LogCategory::CriticalError);
		}

		// Updates the buffer with Data
		// Warning : Costly operation
		T Data;
		StatusCode Update(Device& Dev)
		{
			return Dev.UpdateBuffer(Buffer, Data);
		}
	};
}