#define _CRT_SECURE_NO_WARNINGS 
#include "Device/Device.h"
#include "Core/Log.h"
#include <io.h>
#include <corecrt_io.h>
#include <thread>
#include <chrono>
#include <conio.h>

using namespace std;

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

	FrameDX::Texture2D tmp;
	
	auto tex_desc = FrameDX::Texture2D::Description();
	tex_desc.SizeX = dev.GetBackbuffer()->Desc.SizeX;//desc.WindowDescription.SizeX;
	tex_desc.SizeY = dev.GetBackbuffer()->Desc.SizeY;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	vector<uint8_t> img_data(tex_desc.SizeX*tex_desc.SizeY*4);
	for(int i = 0;i < img_data.size();)
	{
		img_data[i++] = (3*i/27 + i / 31) % 255;
		img_data[i++] = (5*i/31 + i / 63) % 255;
		img_data[i++] = (7*i/121 + i / 127) % 255;
		img_data[i++] = 255;
	}
		

	tmp.CreateFromDescription(&dev,tex_desc,img_data);

	dev.EnterMainLoop([&]()
	{
		dev.GetBackbuffer()->CopyFrom(&tmp);

		dev.GetSwapChain()->Present(0,0);
	});

	return 0;
}