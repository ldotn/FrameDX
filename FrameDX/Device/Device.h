#pragma once
#include "../Core/Core.h"
#include "../Texture/Texture.h"

namespace FrameDX
{
	class Device
	{
	public:
		// There can only be ONE keyboard callback function on the entire program, that's why it's static
		static function<void(WPARAM,KeyAction)> KeyboardCallback;

		struct Description
		{
			D3D11_CULL_MODE CullMode;
			D3D11_FILL_MODE FillMode;
			// If the index is -1 all the devices will be listed on the console and the user must select one
			int AdapterIndex;

			// Compute only devices only create the D3D device and immediate context.
			bool ComputeOnly; 

			// ------------------------
			// The following variables are only valid if ComputeOnly == false
			string WindowName;

			// If any of the size variables equals -1 then the size is computed from the screen size
			Texture::Description BackbufferDescription;
		} Desc;

		StatusCode Start(const Description& CreationParameters);
	private:
		static LRESULT WINAPI InternalMessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		ID3D11Device * D3DDevice;
		ID3D11DeviceContext * ImmediateContext;

		// Only valid if ComputeOnly == false
		IDXGISwapChain * SwapChain;
		Texture * Backbuffer; 
		HWND WindowHandle; 
	};
}
