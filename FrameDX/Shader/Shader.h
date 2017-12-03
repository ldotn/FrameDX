#pragma once
#include "stdafx.h"
#include "../Core/Core.h"
#include "../Device/Device.h"

namespace FrameDX
{
	class ComputeShader
	{
	public:
		StatusCode CreateFromFile(Device * OwnerDevice, std::wstring FilePath);

		StatusCode Bind();
		StatusCode Unbind();
	private:
		ID3D11ComputeShader * Shader;
		Device * OwnerDevice;
	};
}