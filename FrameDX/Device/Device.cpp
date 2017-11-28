#include "stdafx.h"
#include "../Core/Core.h"
#include "../Core/Log.h"
#include "Device.h"

using namespace FrameDX;

function<void(WPARAM,KeyAction)> Device::KeyboardCallback;

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
	return 0;
}

StatusCode Device::Start(const Device::Description& params)
{
	Desc = params;
	
	if(!Desc.ComputeOnly)
	{
		// If no resolution was specified, get it from screen
		if(Desc.BackbufferDescription.SizeX == -1 || Desc.BackbufferDescription.SizeY == -1)
		{   
			RECT desktop;

			const HWND hDesktop = GetDesktopWindow();
			if(!GetWindowRect(hDesktop, &desktop))
				return LAST_ERROR;

			Desc.BackbufferDescription.SizeX = desktop.right;
			Desc.BackbufferDescription.SizeY = desktop.bottom;
		}
		
		// Create window
		// Register the window class
		WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), CS_CLASSDC, Device::InternalMessageProc, 0L, 0L,
								  GetModuleHandleA(NULL), NULL, NULL, NULL, NULL,
								  Desc.WindowName.c_str(), NULL };

		if(!RegisterClassExA( &wc ))
			return LAST_ERROR;

		// Create the application's window
		// This functions doesn't provide failure info with GetLastErro
		int x_border = GetSystemMetrics(SM_CXSIZEFRAME);
		int y_menu   = GetSystemMetrics(SM_CYMENU);
		int y_border = GetSystemMetrics(SM_CYSIZEFRAME);
	
		WindowHandle = CreateWindow( wc.lpszClassName, Desc.WindowName.c_str(),
										  WS_OVERLAPPEDWINDOW, 0, 0, 
										  Desc.BackbufferDescription.SizeX + 2*x_border,
										  Desc.BackbufferDescription.SizeY + 2*y_border + y_menu,
										  NULL, NULL, wc.hInstance, NULL );
		if(!WindowHandle)
			return LAST_ERROR;

		// Show the window
		if(!ShowWindow( WindowHandle, SW_SHOWDEFAULT ))
			return LAST_ERROR;
		if(!UpdateWindow( WindowHandle ))
			return LAST_ERROR;
	}

	DWORD create_device_flags = 0;
#ifdef _DEBUG
	create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL levels[] = 
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	// Find the graphics adapter
	IDXGIAdapter* adapter = nullptr;
	IDXGIFactory* factory = nullptr; 
    
	LogCheckWithReturn(CreateDXGIFactory(__uuidof(IDXGIFactory) ,(void**)&factory),LogCategory::CriticalError);

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

	// Find the feature level by creating a null device
	D3D_FEATURE_LEVEL level;
	LogCheckWithReturn(D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, levels, 2, D3D11_SDK_VERSION, NULL, &level, nullptr ),LogCategory::CriticalError);

}