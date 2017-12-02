#pragma once
#include "../Core/Core.h"
#include "../Texture/Texture.h"
#include "../Core/Log.h"

namespace FrameDX
{
	class Device
	{
	public:
		Device()
		{
			D3DDevice = nullptr;
			ImmediateContext = nullptr;
			SwapChain = nullptr;
		}

		// There can only be ONE keyboard callback function on the entire program, that's why it's static
		static function<void(WPARAM,KeyAction)> KeyboardCallback;

		struct Description
		{
			Description()
			{
				CullMode = D3D11_CULL_BACK;
				FillMode = D3D11_FILL_SOLID;
				AdapterIndex = 0;
				ComputeOnly = false;

				WindowDescription.Name = L"FrameDX";
				WindowDescription.SizeX = 0;
				WindowDescription.SizeY = 0;
				WindowDescription.Fullscreen = false;

				SwapChainDescription.IsStereo = false;
				SwapChainDescription.BufferCount = 3;
				SwapChainDescription.SwapType = DXGI_SWAP_EFFECT_DISCARD;
				SwapChainDescription.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
				SwapChainDescription.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
				SwapChainDescription.Flags = 0;
				SwapChainDescription.VSync = false;
				SwapChainDescription.ScanlineOrder = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
				SwapChainDescription.ScalingNewAPI = DXGI_SCALING_STRETCH;
				SwapChainDescription.BackbufferAccessFlags = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				SwapChainDescription.BackbufferDescription.MSAACount = 1;
				SwapChainDescription.BackbufferDescription.MSAAQuality = 0;
			}

			D3D11_CULL_MODE CullMode;
			D3D11_FILL_MODE FillMode;
			// If the index is -1 all the devices will be listed on the console and the user must select one
			int AdapterIndex;

			// Compute only devices only create the D3D device and immediate context.
			bool ComputeOnly; 

			// ------------------------
			// The following variables are only valid if ComputeOnly == false
			struct 
			{
				wstring Name;
				// If any of the size variables equals 0 then the size is computed from the screen size
				uint32_t SizeX;
				uint32_t SizeY;
				bool Fullscreen;
			} WindowDescription;

			struct
			{
				// If any of the size variables equals 0 then the size is computed from the window size
				Texture2D::Description BackbufferDescription;

				bool IsStereo;
				uint32_t BufferCount;
				DXGI_SWAP_EFFECT SwapType;
				DXGI_MODE_SCALING Scaling;
				DXGI_SCALING ScalingNewAPI;
				DXGI_ALPHA_MODE AlphaMode;
				DXGI_MODE_SCANLINE_ORDER ScanlineOrder;
				uint32_t Flags;
				bool VSync;

				DXGI_USAGE BackbufferAccessFlags;
			} SwapChainDescription;
		} Desc;

		StatusCode Start(const Description& CreationParameters);

		ID3D11Device * GetDevice() { return D3DDevice; }

#define __GET_DEVICE_DECL(v) ID3D11Device ## v * GetDevice##v(bool LogWrongVersion = true) {\
		if(LogWrongVersion && LogAssertAndContinue(DeviceVersion >= v,LogCategory::Error)) return nullptr;\
		return (ID3D11Device ## v *)D3DDevice; }

		__GET_DEVICE_DECL(1);
		__GET_DEVICE_DECL(2);
		__GET_DEVICE_DECL(3);
		__GET_DEVICE_DECL(4);
		__GET_DEVICE_DECL(5);

		int GetDeviceVersion() { return DeviceVersion; }
		int GetContextVersion() { return ContextVersion; }
		int GetSwapChainVersion() { return SwapChainVersion; }

		ID3D11DeviceContext * GetImmediateContext(){ return ImmediateContext; };

#define __GET_CONTEXT_DECL(v) ID3D11DeviceContext ## v * GetImmediateContext##v(bool LogWrongVersion = true) {\
		if(LogWrongVersion && LogAssertAndContinue(ContextVersion >= v,LogCategory::Error)) return nullptr;\
		return (ID3D11DeviceContext ## v *)ImmediateContext; }

		__GET_CONTEXT_DECL(1);
		__GET_CONTEXT_DECL(2);
		__GET_CONTEXT_DECL(3);
		__GET_CONTEXT_DECL(4);

		IDXGISwapChain * GetSwapChain(){ return SwapChain; };

#define __GET_SWAP_DECL(v) IDXGISwapChain ## v * GetSwapChain##v(bool LogWrongVersion = true) {\
		if(LogWrongVersion && LogAssertAndContinue(SwapChainVersion >= v,LogCategory::Error)) return nullptr;\
		return (IDXGISwapChain ## v *)SwapChain; }

		__GET_SWAP_DECL(1);

		// Wraps a PeekMessage loop, and calls f on idle time
		void EnterMainLoop(function<void()> LoopBody);

		Texture * GetBackbuffer(){ return &Backbuffer; }
		Texture * GetZBuffer(){ return &ZBuffer; }
	private:
		static LRESULT WINAPI InternalMessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		ID3D11Device * D3DDevice;
		ID3D11DeviceContext * ImmediateContext;

		int DeviceVersion;
		int ContextVersion;
		int SwapChainVersion;

		// Only valid if ComputeOnly == false
		IDXGISwapChain * SwapChain;
		Texture2D Backbuffer;
		Texture2D ZBuffer; 
		HWND WindowHandle; 
	};
}

#undef __GET_DEVICE_DECL
#undef __GET_CONTEXT_DECL