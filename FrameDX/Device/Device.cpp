#include "stdafx.h"
#include "../Core/Core.h"
#include "../Core/Log.h"
#include "Device.h"

using namespace FrameDX;

LRESULT WINAPI Device::InternalMessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
    {
        case WM_DESTROY:
			 PostQuitMessage( 0 );
			 break;
        case WM_KEYDOWN:
            Device::Description::KeyboardCallback(wParam,KeyAction::Down);
			break;
		case WM_KEYUP:
            Device::Description::KeyboardCallback(wParam,KeyAction::Up);
			break;
    }
	return 0;
}

StatusCode Device::Start(const Device::Description& params)
{
	LogAssertWithReturn((!Desc.KeyboardCallback && !params.KeyboardCallback) || (!Desc.KeyboardCallback && !params.KeyboardCallback),LogCategory::CriticalError,StatusCode::InvalidArgument);

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
		int xBorder = GetSystemMetrics(SM_CXSIZEFRAME);
		int yMenu   = GetSystemMetrics(SM_CYMENU);
		int yBorder = GetSystemMetrics(SM_CYSIZEFRAME);
	
		WindowHandle = CreateWindowA( wc.lpszClassName, Desc.WindowName.c_str(),
										  WS_OVERLAPPEDWINDOW, 0, 0, 
										  Desc.BackbufferDescription.SizeX + 2*xBorder,
										  Desc.BackbufferDescription.SizeY + 2*yBorder+yMenu,
										  NULL, NULL, wc.hInstance, NULL );
		if(!WindowHandle)
			return LAST_ERROR;

		// Show the window
		if(!ShowWindow( WindowHandle, SW_SHOWDEFAULT ))
			return LAST_ERROR;
		if(!UpdateWindow( WindowHandle ))
			return LAST_ERROR;
	}
}