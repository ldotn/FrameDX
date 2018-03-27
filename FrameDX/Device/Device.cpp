#include "stdafx.h"
#include "../Core/Core.h"
#include "../Core/Log.h"
#include "Device.h"

using namespace FrameDX;

function<void(WPARAM,KeyAction)> Device::KeyboardCallback;

void FrameDX::Device::EnterMainLoop(function<void(double)> LoopBody)
{
	MSG msg;
	msg.message = WM_NULL;
	last_call_time = chrono::high_resolution_clock::now();

	while(msg.message != WM_QUIT)
	{
		// Use PeekMessage() render on idle time
		if(PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ))
		{
			// Translate and dispatch the message
			if(!TranslateAccelerator( WindowHandle, NULL, &msg ))
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
		else
		{
			// Measure time between now and the last call
			auto time = chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - last_call_time);
			last_call_time = chrono::high_resolution_clock::now();
			LoopBody(time.count());
		}
	}
}

LRESULT WINAPI Device::InternalMessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
    {
        case WM_DESTROY:
			 PostQuitMessage( 0 );
			 break;
        case WM_KEYDOWN:
            Device::KeyboardCallback(wParam,KeyAction::Down);
			break;
		case WM_KEYUP:
            Device::KeyboardCallback(wParam,KeyAction::Up);
			break;
    }

	return DefWindowProc( hWnd, msg, wParam, lParam );
}

StatusCode Device::Start(const Device::Description& params)
{
	Desc = params;
	
	if(!Desc.ComputeOnly)
	{
		// If no resolution was specified, get it from screen
		if(Desc.WindowDescription.SizeX == 0 || Desc.WindowDescription.SizeY == 0)
		{   
			RECT desktop;

			const HWND hDesktop = GetDesktopWindow();
			if(!GetWindowRect(hDesktop, &desktop))
				return LAST_ERROR;

			Desc.WindowDescription.SizeX = desktop.right;
			Desc.WindowDescription.SizeY = desktop.bottom;
		}
		
		// Create window
		// Register the window class
		WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, Device::InternalMessageProc, 0L, 0L,
								  GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
								  Desc.WindowDescription.Name.c_str(), NULL };

		if(!RegisterClassEx( &wc ))
			return LAST_ERROR;

		int x_border = GetSystemMetrics(SM_CXSIZEFRAME);
		int y_menu   = GetSystemMetrics(SM_CYMENU);
		int y_border = GetSystemMetrics(SM_CYSIZEFRAME);

		// Create the application's window
		// This functions doesn't provide failure info with GetLastErro
		WindowHandle = CreateWindow( wc.lpszClassName, Desc.WindowDescription.Name.c_str(),
										  Desc.WindowDescription.Fullscreen ? WS_POPUP : WS_OVERLAPPEDWINDOW, 0, 0, 
										  Desc.WindowDescription.SizeX + 2*x_border,
										  Desc.WindowDescription.SizeY + 2*y_border + y_menu,
										  NULL, NULL, wc.hInstance, NULL );
		if(!WindowHandle)
			return LAST_ERROR;

		// Show the window
		ShowWindow(WindowHandle, SW_SHOWDEFAULT);
		if(!UpdateWindow( WindowHandle ))
			return LAST_ERROR;
	}

	DWORD device_flags = 0;
#ifdef _DEBUG
	device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Find the graphics adapter
	IDXGIAdapter* adapter = nullptr;
	IDXGIFactory* factory = nullptr; 
    bool UsingFactory2 = true;

	UINT dxgi2_flags = 0;
#ifdef _DEBUG
	dxgi2_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	if(CreateDXGIFactory2(dxgi2_flags,__uuidof(IDXGIFactory2),(void**)&factory) != S_OK)
	{
		UsingFactory2 = false;
		LogCheckWithReturn(CreateDXGIFactory(__uuidof(IDXGIFactory) ,(void**)&factory),LogCategory::CriticalError);
	}

	if(Desc.AdapterIndex == -1)
	{       
		for ( UINT i = 0;factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND;i++ )
		{
			DXGI_ADAPTER_DESC desc;
			adapter->GetDesc(&desc);

			wcout << L"GPU : " << i << L" , " << desc.Description << endl;
		} 

		wcout << L"Select GPU : ";
		int adapterIDX = 0;
		cin >> Desc.AdapterIndex;
	}

	factory->EnumAdapters(Desc.AdapterIndex, &adapter);

	// Try to create a device with feature level 11.1 (DX 11.2, 11.3, 11.4, etc, also use feature level 11.1)
	D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_1;
	if(LogCheckAndContinue(D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, device_flags, &level, 1, D3D11_SDK_VERSION, &D3DDevice, nullptr, &ImmediateContext ),
		LogCategory::CriticalError) == StatusCode::InvalidArgument)
	{
		// If create device returned invalid argument, it's because DirectX 11.1 is not supported, try with 11.0
		// Can't do the usual way of sending the array of feature levels and creating a null device because D3D11CreateDevice returns E_INVALIDARG if 11.1 is not supported
		LogCheckWithReturn(D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, device_flags, &level, 1, D3D11_SDK_VERSION, &D3DDevice, nullptr, &ImmediateContext ),LogCategory::CriticalError);
	}
	
	// Check if the device can be casted to anything higher than 11.0
	ID3D11Device* new_device;
	// Device5 crashes creating a render target on debug for some reason https://stackoverflow.com/questions/46633120/how-can-i-properly-create-a-rendertarget-using-id3d11device5
	// Even more weird, it doesn't happen on my desktop, only on the notebook. Maybe an optimus/driver bug or something to do with Home vs Pro versions?
#ifndef _DEBUG
	if(D3DDevice->QueryInterface( __uuidof(ID3D11Device5), (void**)&new_device ) == S_OK)
	{
		D3DDevice = new_device;
		DeviceVersion = 5;
	}
	else 
#endif 
		if(D3DDevice->QueryInterface( __uuidof(ID3D11Device4), (void**)&new_device ) == S_OK)
	{
		D3DDevice = new_device;
		DeviceVersion = 4;
	}
	else if(D3DDevice->QueryInterface( __uuidof(ID3D11Device3), (void**)&new_device ) == S_OK)
	{
		D3DDevice = new_device;
		DeviceVersion = 3;
	}
	else if(D3DDevice->QueryInterface( __uuidof(ID3D11Device2), (void**)&new_device ) == S_OK)
	{
		D3DDevice = new_device;
		DeviceVersion = 2;
	}
	else if(D3DDevice->QueryInterface( __uuidof(ID3D11Device1), (void**)&new_device ) == S_OK)
	{
		D3DDevice = new_device;
		DeviceVersion = 1;
	}
	else
		DeviceVersion = 0;

	// Check if the immediate context can be casted to anything higher than 11.0
	ID3D11DeviceContext* new_context;
	if(ImmediateContext->QueryInterface( __uuidof(ID3D11DeviceContext4), (void**)&new_context ) == S_OK)
	{
		ImmediateContext = new_context;
		ContextVersion = 4;
	}
	else if(ImmediateContext->QueryInterface( __uuidof(ID3D11DeviceContext3), (void**)&new_context ) == S_OK)
	{
		ImmediateContext = new_context;
		ContextVersion = 3;
	}
	else if(ImmediateContext->QueryInterface( __uuidof(ID3D11DeviceContext2), (void**)&new_context ) == S_OK)
	{
		ImmediateContext = new_context;
		ContextVersion = 2;
	}
	else if(ImmediateContext->QueryInterface( __uuidof(ID3D11DeviceContext1), (void**)&new_context ) == S_OK)
	{
		ImmediateContext = new_context;
		ContextVersion = 1;
	}
	else
		ContextVersion = 0;

	// While it's supposed that sending a size of 0 makes DXGI get the size directly from the window, it ends up a little bit smaller than expected
	// Also, i need to make sure the same size as the backbuffer is sent to the DSV, so if no size is provided manually, it's set to the window size
	if(Desc.SwapChainDescription.BackbufferDescription.SizeX == 0 || Desc.SwapChainDescription.BackbufferDescription.SizeY == 0)
	{
		Desc.SwapChainDescription.BackbufferDescription.SizeX = Desc.WindowDescription.SizeX;
		Desc.SwapChainDescription.BackbufferDescription.SizeY = Desc.WindowDescription.SizeY;
	}

	// If the device is not compute only, create a swap chain
	if(!Desc.ComputeOnly)
	{
		DEVMODE lpDevMode;
		memset(&lpDevMode, 0, sizeof(DEVMODE));
		lpDevMode.dmSize = sizeof(DEVMODE);
		lpDevMode.dmDriverExtra = 0;

		EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &lpDevMode);

		if(UsingFactory2)
		{
			SwapChainVersion = 1;

			DXGI_SWAP_CHAIN_DESC1 desc;
			desc.Width = Desc.SwapChainDescription.BackbufferDescription.SizeX; // Can be different than the window size
			desc.Height = Desc.SwapChainDescription.BackbufferDescription.SizeY;
			desc.BufferUsage = Desc.SwapChainDescription.BackbufferAccessFlags;
			desc.SampleDesc.Count = Desc.SwapChainDescription.BackbufferDescription.MSAACount;
			desc.SampleDesc.Quality = Desc.SwapChainDescription.BackbufferDescription.MSAAQuality;
			desc.BufferCount = Desc.SwapChainDescription.BufferCount;
			desc.SwapEffect = Desc.SwapChainDescription.SwapType;
			desc.Flags = Desc.SwapChainDescription.Flags;
			desc.Format = Desc.SwapChainDescription.BackbufferDescription.Format;
			desc.Scaling = Desc.SwapChainDescription.ScalingNewAPI;
			desc.Stereo = Desc.SwapChainDescription.IsStereo;
			desc.AlphaMode = Desc.SwapChainDescription.AlphaMode;
			
			DXGI_SWAP_CHAIN_FULLSCREEN_DESC fdesc;
			if(Desc.WindowDescription.Fullscreen)
			{
				if(Desc.SwapChainDescription.VSync)
				{
					fdesc.RefreshRate.Numerator = lpDevMode.dmDisplayFrequency;
					fdesc.RefreshRate.Denominator = 1;
				}
				else
				{
					fdesc.RefreshRate.Numerator = 0;
					fdesc.RefreshRate.Denominator = 1;
				}

				fdesc.Scaling = Desc.SwapChainDescription.Scaling;
				fdesc.ScanlineOrdering = Desc.SwapChainDescription.ScanlineOrder;
				fdesc.Windowed = false;
			}
		
			LogCheckWithReturn(((IDXGIFactory2*)factory)->CreateSwapChainForHwnd(D3DDevice,WindowHandle,&desc,
				Desc.WindowDescription.Fullscreen ? &fdesc : nullptr,nullptr,(IDXGISwapChain1**)&SwapChain),LogCategory::CriticalError);
		}
		else
		{
			SwapChainVersion = 0;

			DXGI_SWAP_CHAIN_DESC desc;
			desc.BufferDesc.Format = Desc.SwapChainDescription.BackbufferDescription.Format;
			desc.BufferDesc.Width = Desc.SwapChainDescription.BackbufferDescription.SizeX; // Can be different than the window size
			desc.BufferDesc.Height = Desc.SwapChainDescription.BackbufferDescription.SizeY;
			desc.BufferDesc.Scaling = Desc.SwapChainDescription.Scaling;
			desc.BufferDesc.ScanlineOrdering = Desc.SwapChainDescription.ScanlineOrder;
			desc.Windowed = Desc.WindowDescription.Fullscreen;
			desc.BufferUsage = (DXGI_USAGE)Desc.SwapChainDescription.BackbufferAccessFlags;
			desc.SampleDesc.Count = Desc.SwapChainDescription.BackbufferDescription.MSAACount;
			desc.SampleDesc.Quality = Desc.SwapChainDescription.BackbufferDescription.MSAAQuality;
			desc.BufferCount = Desc.SwapChainDescription.BufferCount;
			desc.SwapEffect = Desc.SwapChainDescription.SwapType;
			desc.Flags = Desc.SwapChainDescription.Flags;

			if(Desc.SwapChainDescription.VSync)
			{
				desc.BufferDesc.RefreshRate.Numerator = lpDevMode.dmDisplayFrequency;
				desc.BufferDesc.RefreshRate.Denominator = 1;
			}
			else
			{
				desc.BufferDesc.RefreshRate.Numerator = 0;
				desc.BufferDesc.RefreshRate.Denominator = 1;
			}

			LogCheckWithReturn(factory->CreateSwapChain(D3DDevice,&desc,&SwapChain),LogCategory::CriticalError);
		}

		
		// Create backbuffer texure
		Backbuffer.CreateFromBackbuffer(this);

		// Create depth buffer
		auto z_desc = FrameDX::Texture2D::Description();
		z_desc.SizeX = Desc.SwapChainDescription.BackbufferDescription.SizeX;
		z_desc.SizeY = Desc.SwapChainDescription.BackbufferDescription.SizeY;
		z_desc.MipLevels = 1;
		z_desc.Format = DXGI_FORMAT_R24G8_TYPELESS; //DXGI_FORMAT_D24_UNORM_S8_UINT, but using typeless to bind the buffer as a texture later
		z_desc.MSAACount = Desc.SwapChainDescription.BackbufferDescription.MSAACount;
		z_desc.MSAAQuality = Desc.SwapChainDescription.BackbufferDescription.MSAAQuality;
		z_desc.Usage = D3D11_USAGE_DEFAULT;
		z_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		z_desc.AccessFlags = 0;
		z_desc.MiscFlags = 0;	
		// Need to create the views separately, as they use a different format
		LogCheckWithReturn(ZBuffer.CreateFromDescription(this,z_desc,{},0),LogCategory::Error);

		D3D11_DEPTH_STENCIL_VIEW_DESC zbuffer_dsv_desc;
		ZBuffer.FillDSVDescription(&zbuffer_dsv_desc);
		zbuffer_dsv_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		LogCheckWithReturn(ZBuffer.CreateDSV(&zbuffer_dsv_desc),LogCategory::Error);

		auto srv_version = ZBuffer.GetBestViewVersion<Texture::ViewType::SRV>();
		if(srv_version == 1)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC1 zbuffer_srv_desc;
			ZBuffer.FillSRVDescription1(&zbuffer_srv_desc);
			zbuffer_srv_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			LogCheckWithReturn(ZBuffer.CreateSRV(&zbuffer_srv_desc),LogCategory::Error);
		}
		else if(srv_version == 0)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC zbuffer_srv_desc;
			ZBuffer.FillSRVDescription(&zbuffer_srv_desc);
			zbuffer_srv_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			LogCheckWithReturn(ZBuffer.CreateSRV(&zbuffer_srv_desc),LogCategory::Error);
		}
		else 
			return StatusCode::InvalidArgument;
	}

	
	return StatusCode::Ok;
}

void Device::BindPipelineState(const PipelineState& NewState)
{
#define changed(v) CurrentPipelineState.v != NewState.v
#define update(v) CurrentPipelineState.v = NewState.v

	// If any step needs unbinds, just set the resources count for that type to 0 and be done with it in one call
	// The old "sparse unbind" is useless 
	bool needs_srv_unbind[(size_t)ShaderStage::_count]{}; // one per stage
	bool needs_uav_unbind = false;
	bool needs_cs_uav_unbind = false;
	bool needs_rtv_unbind = false;

	bool needs_srv_bind[(size_t)ShaderStage::_count]{}; // one per stage
	bool needs_uav_bind = false;
	bool needs_cs_uav_bind = false;
	bool needs_rtv_bind = false;

	// Mesh
	if (changed(Mesh.IndexBuffer) || changed(Mesh.IndexFormat))
	{
		ImmediateContext->IASetIndexBuffer(NewState.Mesh.IndexBuffer, NewState.Mesh.IndexFormat, 0);

		update(Mesh.IndexBuffer);
		update(Mesh.IndexFormat);
	}

	if (changed(Mesh.VertexBuffer))
	{
		ImmediateContext->IASetVertexBuffers(0, 1, &NewState.Mesh.VertexBuffer, nullptr, nullptr);
		update(Mesh.VertexBuffer);
	}

	if (changed(Mesh.InputLayout))
	{
		ImmediateContext->IASetInputLayout(NewState.Mesh.InputLayout);
		update(Mesh.InputLayout);
	}

	if (changed(Mesh.PrimitiveType))
	{
		ImmediateContext->IASetPrimitiveTopology(NewState.Mesh.PrimitiveType);
		update(Mesh.PrimitiveType);
	}

	// Output Context
	if (changed(Output.Viewports))
	{
		ImmediateContext->RSSetViewports(NewState.Output.Viewports.size(), NewState.Output.Viewports.data());
		update(Output.Viewports);
	}

	if (changed(Output.DepthStencilState) || changed(Output.StencilRef))
	{
		ImmediateContext->OMSetDepthStencilState(NewState.Output.DepthStencilState, NewState.Output.StencilRef);
		update(Output.DepthStencilState);
		update(Output.StencilRef);
	}

	if ( changed(Output.BlendState) || 
		 changed(Output.BlendFactors[0]) || 
		 changed(Output.BlendFactors[1]) ||
		 changed(Output.BlendFactors[2]) ||
		 changed(Output.BlendFactors[3]) 
	   )
	{
		ImmediateContext->OMSetBlendState(NewState.Output.BlendState, NewState.Output.BlendFactors, -1);
		update(Output.BlendState);
		update(Output.BlendFactors[0]);
		update(Output.BlendFactors[1]);
		update(Output.BlendFactors[2]);
		update(Output.BlendFactors[3]);
	}

	if (changed(Output.RasterState))
	{
		ImmediateContext->RSSetState(NewState.Output.RasterState);
		update(Output.RasterState);
	}

	if (changed(Output.DSV))
	{
		ID3D11Resource * resource;
		NewState.Output.DSV->GetResource(&resource);

		auto entry = InputBoundResources.find(resource);
		if (entry != InputBoundResources.end())
		{
			// Flag that an unbind is needed, and remove it from the bound resources
			needs_srv_unbind[(size_t)entry->second] = true;

			// Not calling release here, as one map removes the value but the other grabs it
			InputBoundResources.erase(entry);
		}

		// DSVs are unbinded at the same time as RTVs
		OutputBoundResources.insert({ resource, OutputType::PixelRTV });

		needs_rtv_bind = true;
		update(Output.DSV);
	}

	if (changed(Output.RTVs))
	{
		for (auto & rtv : NewState.Output.RTVs)
		{
			ID3D11Resource * resource;
			rtv->GetResource(&resource);

			auto entry = InputBoundResources.find(resource);
			if (entry != InputBoundResources.end())
			{
				// Flag that an unbind is needed, and remove it from the bound resources
				needs_srv_unbind[(size_t)entry->second] = true;

				entry->first->Release();
				InputBoundResources.erase(entry);
				
			}

			OutputBoundResources.insert({ resource, OutputType::PixelRTV });
		}
		
		needs_rtv_bind = true;
		update(Output.RTVs);
	}

	if (changed(Output.UAVs))
	{
		for (auto & uav : NewState.Output.UAVs)
		{
			ID3D11Resource * resource;
			uav->GetResource(&resource);

			auto entry = InputBoundResources.find(resource);
			if (entry != InputBoundResources.end())
			{
				// Flag that an unbind is needed, and remove it from the bound resources
				needs_srv_unbind[(size_t)entry->second] = true;

				entry->first->Release();
				InputBoundResources.erase(entry);

			}

			OutputBoundResources.insert({ resource, OutputType::PixelUAV });
		}

		needs_uav_bind = true;
		update(Output.UAVs);
	}

	if (changed(Output.ComputeShaderUAVs))
	{
		for (auto & uav : NewState.Output.ComputeShaderUAVs)
		{
			ID3D11Resource * resource;
			uav->GetResource(&resource);

			auto entry = InputBoundResources.find(resource);
			if (entry != InputBoundResources.end())
			{
				// Flag that an unbind is needed, and remove it from the bound resources
				needs_srv_unbind[(size_t)entry->second] = true;

				entry->first->Release();
				InputBoundResources.erase(entry);

			}

			OutputBoundResources.insert({ resource, OutputType::ComputeUAV });
		}

		needs_uav_bind = true;
		update(Output.ComputeShaderUAVs);
	}

	// Shaders
#define update_shader(t,tf) if (changed(Shaders[(size_t)ShaderStage::t].ShaderPtr)) {\
		ImmediateContext->tf##SetShader((ID3D11##t##Shader*)NewState.Shaders[(size_t)ShaderStage::t].ShaderPtr, nullptr, 0);\
		update(Shaders[(size_t)ShaderStage::t].ShaderPtr); }
#define update_shader_cb(t,tf) if (changed(Shaders[(size_t)ShaderStage::t].ConstantBuffersTable)) {\
		ImmediateContext->tf##SetConstantBuffers(0,NewState.Shaders[(size_t)ShaderStage::t].ConstantBuffersTable.size(), NewState.Shaders[(size_t)ShaderStage::t].ConstantBuffersTable.data());\
		update(Shaders[(size_t)ShaderStage::t].ShaderPtr); }

	update_shader(Vertex  , VS);
	update_shader(Hull    , HS);
	update_shader(Domain  , DS);
	update_shader(Geometry, GS);
	update_shader(Pixel   , PS);
	update_shader(Compute , CS);

	update_shader_cb(Vertex  , VS);
	update_shader_cb(Hull    , HS);
	update_shader_cb(Domain  , DS);
	update_shader_cb(Geometry, GS);
	update_shader_cb(Pixel   , PS);
	update_shader_cb(Compute , CS);

	for (size_t i = 0; i < (size_t)ShaderStage::_count; i++)
	{
		if (changed(Shaders[i].ResourcesTable))
		{
			for (auto & srv : NewState.Shaders[i].ResourcesTable)
			{
				ID3D11Resource * resource;
				srv->GetResource(&resource);

				auto entry = OutputBoundResources.find(resource);
				if (entry != OutputBoundResources.end())
				{
					// Flag that an unbind is needed, and remove it from the bound resources
					if (entry->second == OutputType::ComputeUAV)
						needs_cs_uav_unbind = true;
					else if (entry->second == OutputType::PixelRTV)
						needs_rtv_unbind = true;
					else
						needs_uav_unbind = true;

					entry->first->Release();
					OutputBoundResources.erase(entry);
				}

				InputBoundResources.insert({ resource, (ShaderStage)i });
			}

			needs_srv_bind[i] = true;
			update(Shaders[i].ResourcesTable);
		}
	}

	// Finished with the setup
	// At this point CurrentPipelineState is fully updated
	// -----------------------------------------
	
	// Unbind if needed
	if (needs_cs_uav_unbind)
		ImmediateContext->CSSetUnorderedAccessViews(0, 0, nullptr, nullptr);

	if (needs_rtv_unbind)
	{
		if (needs_uav_unbind)
			ImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0, 0, nullptr, nullptr);
		else
			ImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);
	}

	if (needs_srv_unbind[(size_t)ShaderStage::Vertex])
		ImmediateContext->VSSetShaderResources(0, 0, nullptr);
	if (needs_srv_unbind[(size_t)ShaderStage::Hull])
		ImmediateContext->HSSetShaderResources(0, 0, nullptr);
	if (needs_srv_unbind[(size_t)ShaderStage::Domain])
		ImmediateContext->DSSetShaderResources(0, 0, nullptr);
	if (needs_srv_unbind[(size_t)ShaderStage::Geometry])
		ImmediateContext->GSSetShaderResources(0, 0, nullptr);
	if (needs_srv_unbind[(size_t)ShaderStage::Pixel])
		ImmediateContext->PSSetShaderResources(0, 0, nullptr);
	if (needs_srv_unbind[(size_t)ShaderStage::Compute])
		ImmediateContext->CSSetShaderResources(0, 0, nullptr);

	// Now bind if needed
	UINT dummy = -1;

	if (needs_rtv_bind)
	{
		if (needs_uav_bind)
			ImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews
				(
					CurrentPipelineState.Output.RTVs.size(), 
					CurrentPipelineState.Output.RTVs.data(), 
					CurrentPipelineState.Output.DSV, 
					CurrentPipelineState.Output.RTVs.size(), 
					CurrentPipelineState.Output.UAVs.size(), 
					CurrentPipelineState.Output.UAVs.data(),
					&dummy
				);
		else
			ImmediateContext->OMSetRenderTargets
				(
					CurrentPipelineState.Output.RTVs.size(), 
					CurrentPipelineState.Output.RTVs.data(), 
					CurrentPipelineState.Output.DSV
				);
	}

	if (needs_cs_uav_bind)
		ImmediateContext->CSSetUnorderedAccessViews(0, CurrentPipelineState.Output.ComputeShaderUAVs.size(), CurrentPipelineState.Output.ComputeShaderUAVs.data(), &dummy);

	if (needs_srv_bind[(size_t)ShaderStage::Vertex])
		ImmediateContext->VSSetShaderResources(0, CurrentPipelineState.Shaders[(size_t)ShaderStage::Vertex].ResourcesTable.size(), CurrentPipelineState.Shaders[(size_t)ShaderStage::Vertex].ResourcesTable.data());
	if (needs_srv_bind[(size_t)ShaderStage::Hull])
		ImmediateContext->HSSetShaderResources(0, CurrentPipelineState.Shaders[(size_t)ShaderStage::Hull].ResourcesTable.size(), CurrentPipelineState.Shaders[(size_t)ShaderStage::Hull].ResourcesTable.data());
	if (needs_srv_bind[(size_t)ShaderStage::Domain])
		ImmediateContext->DSSetShaderResources(0, CurrentPipelineState.Shaders[(size_t)ShaderStage::Domain].ResourcesTable.size(), CurrentPipelineState.Shaders[(size_t)ShaderStage::Domain].ResourcesTable.data());
	if (needs_srv_bind[(size_t)ShaderStage::Geometry])
		ImmediateContext->GSSetShaderResources(0, CurrentPipelineState.Shaders[(size_t)ShaderStage::Geometry].ResourcesTable.size(), CurrentPipelineState.Shaders[(size_t)ShaderStage::Geometry].ResourcesTable.data());
	if (needs_srv_bind[(size_t)ShaderStage::Pixel])
		ImmediateContext->PSSetShaderResources(0, CurrentPipelineState.Shaders[(size_t)ShaderStage::Pixel].ResourcesTable.size(), CurrentPipelineState.Shaders[(size_t)ShaderStage::Pixel].ResourcesTable.data());
	if (needs_srv_bind[(size_t)ShaderStage::Compute])
		ImmediateContext->CSSetShaderResources(0, CurrentPipelineState.Shaders[(size_t)ShaderStage::Compute].ResourcesTable.size(), CurrentPipelineState.Shaders[(size_t)ShaderStage::Compute].ResourcesTable.data());

#undef changed
#undef update
}

Device::~Device()
{
	for (auto& r : InputBoundResources)
		if(r.first) r.first->Release();
	for (auto& r : OutputBoundResources)
		if(r.first) r.first->Release();

	D3DDevice->Release();
	ImmediateContext->Release();
	SwapChain->Release();
}