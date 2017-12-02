#include "stdafx.h"
#include "../Core/Core.h"
#include "../Core/Log.h"
#include "Device.h"

using namespace FrameDX;

function<void(WPARAM,KeyAction)> Device::KeyboardCallback;

void FrameDX::Device::EnterMainLoop(function<void()> LoopBody)
{
	MSG msg;
	msg.message = WM_NULL;

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
			LoopBody();
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

		wcout << L"Select GPU : " << endl;
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
	}
	
	// Create backbuffer texure
	Backbuffer.CreateFromSwapChain(this);

	// Create depth buffer
	
	return StatusCode::Ok;
}