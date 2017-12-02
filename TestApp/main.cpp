#define _CRT_SECURE_NO_WARNINGS 
#include "Device/Device.h"
#include "Core/Log.h"
#include <io.h>
#include <corecrt_io.h>
#include <thread>
#include <chrono>
#include <conio.h>

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR, int)
{
    AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	FrameDX::Device::KeyboardCallback = [](WPARAM key, FrameDX::KeyAction action)
	{
		if(key == 'W' && action == FrameDX::KeyAction::Up)
			LogAssert(false,FrameDX::LogCategory::Error);
	};

	FrameDX::Device dev;

	auto desc = FrameDX::Device::Description();
	desc.WindowDescription.SizeX = 1920;
	desc.WindowDescription.SizeY = 1080;
	
	LogCheck(dev.Start(desc),FrameDX::LogCategory::CriticalError);

	thread log_printer([]()
	{
		while(true)
		{
			system("cls");
			FrameDX::Log.PrintAll(wcout);
			this_thread::sleep_for(250ms);
		}
	});
	log_printer.detach();


	dev.EnterMainLoop([&]()
	{
		ID3D11RenderTargetView* rt_array[] = {dev.GetBackbuffer()->RTV};
		float clear_color[] = { 1.0, 0.0, 1.0, 1.0 };
		dev.GetImmediateContext()->ClearRenderTargetView(dev.GetBackbuffer()->RTV,clear_color);
		dev.GetImmediateContext()->OMSetRenderTargets(1,rt_array,dev.GetZBuffer()->DSV);
		
		dev.GetSwapChain()->Present(0,0);
	});

	return 0;
}