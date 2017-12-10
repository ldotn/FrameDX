#pragma once
#include "stdafx.h"
#include "../Core/Core.h"
#include "../Core/Binding.h"

namespace FrameDX
{
	class ComputeShader : public Bindable
	{
	public:
		StatusCode CreateFromFile( Device * OwnerDevice, 
								   wstring FilePath,
								   string EntryPoint,
								   bool FullDebug = false, 
								   vector<pair<string,string>> Defines = {});

		virtual StatusCode Bind() final override;
		virtual StatusCode Unbind() final override;
	private:
		ID3D11ComputeShader * Shader;
	};
}