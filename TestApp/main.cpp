#define _CRT_SECURE_NO_WARNINGS 
#include "Device/Device.h"
#include "Core/Log.h"
#include <io.h>
#include <corecrt_io.h>

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR, int)
{
    AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	FrameDX::Device dev;

	auto desc = FrameDX::Device::Description();
	desc.WindowDescription.SizeX = 1920;
	desc.WindowDescription.SizeY = 1080;

	LogCheck(dev.Start(desc),FrameDX::LogCategory::CriticalError);

	HWND hWnd = dev.WindowHandle;
	BOOL bGotMsg;
	MSG  msg;
	msg.message = WM_NULL;

	while( WM_QUIT != msg.message  )
	{
		// Use PeekMessage() so we can use idle time to render the scene
		bGotMsg = ( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) != 0 );

		if( bGotMsg )
		{
			// Translate and dispatch the message
			if( 0 == TranslateAccelerator( hWnd, NULL, &msg ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
		else
		{
			system("cls");
			FrameDX::Log.PrintAll(wcout);
		}
	}
}