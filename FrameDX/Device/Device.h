#pragma once
#include "../Core/Core.h"
#include "../Texture/Texture.h"
#include "../Core/Log.h"
#include "../Core/PipelineState.h"

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
		~Device();

		// There can only be ONE keyboard callback function on the entire program, that's why it's static
		static function<void(WPARAM,KeyAction)> KeyboardCallback;

		struct Description
		{
			Description()
			{
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
		
		uint32_t GetDeviceVersion() { return DeviceVersion; }
		uint32_t GetContextVersion() { return ContextVersion; }
		uint32_t GetSwapChainVersion() { return SwapChainVersion; }

		// Wraps a PeekMessage loop, and calls f on idle time
		void EnterMainLoop(function<void(double)> LoopBody);

		Texture2D * GetBackbuffer(){ return &Backbuffer; }
		Texture2D * GetZBuffer(){ return &ZBuffer; }

		struct BindInfo
		{
			enum { Input, Output } Usage;
			uint32_t Slot;
			enum { Vertex, Hull, Domain, Geometry, Pixel, StagesNum } ShaderStage;
		};

		// Registers/Removes a resource without any kind of check
		// IF IT ALREADY EXISTS IT WILL OVERWRITE THE RECORD
		void RegisterBoundResource(uintptr_t Resource, BindInfo Info) 
		{ 
			BoundResources[Resource] = { true, Info }; 
		}
		void UnregisterBoundResource(uintptr_t Resource) 
		{ 
			auto v = BoundResources.find(Resource);
			if (v != BoundResources.end())
				v->second.first = false;
			else
				// TODO: Use the debug names here, much more useful than just a big number
				LogMsg(wstring(L"Resource ") + to_wstring(Resource) + wstring(L" wasn't registered before  trying to unregister"), LogCategory::Warning);
		}
		void RemoveBoundResource(uintptr_t Resource) { BoundResources.erase(Resource); }

		// Check if a resource is registered in the bound resources map
		// Optionally it takes a pointer to a BindType variable, and if it's found, it fills the pointer with the resource usage (in/out)
		bool IsResourceBound(uintptr_t Resource, pair<bool,BindInfo>* UsagePtr = nullptr)
		{
			auto v = BoundResources.find(Resource);
			if (v != BoundResources.end())
			{
				if(UsagePtr)
					*UsagePtr =  v->second;
				return true;
			}
			else
				return false;
		}

		// Binds a new state
		// Automatically unbinds resources that have an in/out conflict (bound before as uav and then as srv or vice versa)
		// It stores the state to check if it changed, so it won't detect changes done directly.
		// This DOES NOT prevent in/out conflicts in the provided pipeline state, it ONLY prevents it compared to the old state
		void BindPipelineState(const PipelineState& NewState);

		PipelineState GetCurrentPipelineStateCopy() { return CurrentPipelineState;  }
	private:
		// Used to keep track of the bound state
		enum class OutputType { ComputeUAV, PixelRTV, PixelUAV };
		map<ID3D11Resource*, ShaderStage> InputBoundResources;
		map<ID3D11Resource*, OutputType> OutputBoundResources;

		PipelineState CurrentPipelineState;

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

		std::chrono::time_point<std::chrono::high_resolution_clock> last_call_time;
	};
}

#undef __GET_DEVICE_DECL
#undef __GET_CONTEXT_DECL
#undef __GET_SWAP_DECL