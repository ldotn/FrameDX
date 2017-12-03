#define _CRT_SECURE_NO_WARNINGS 
#include "Device/Device.h"
#include "Core/Log.h"
#include <io.h>
#include <corecrt_io.h>
#include <thread>
#include <chrono>
#include <conio.h>
#include "Shader/Shader.h"

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
		auto v = [](int idx)
		{
			return 3 * (idx % 2) +
				   1 * (idx % 4) +
				   2 * (idx % 8) +
				   7 * (idx % 16) +
				   27 * (idx % 32) +
				   2 * (idx % 64) +
				   -12 * (idx % 128) +
				   39 * (idx % 256) +
				   20 * (idx % 512) +
				   -21 * (idx % 1024) +
				   7 * (idx % 2048);
		};

		img_data[i++] = v(i/321);
		img_data[i++] = v(i/456);
		img_data[i++] = v(i/789);
		img_data[i++] = 255;
	}

	tmp.CreateFromDescription(&dev,tex_desc,img_data);


	FrameDX::ComputeShader dbg;

	{
		FrameDX::ScopedBind(&dbg);
	}
	

	dev.EnterMainLoop([&]()
	{
		dev.GetBackbuffer()->CopyFrom(&tmp);

		dev.GetSwapChain()->Present(0,0);
	});

	return 0;
}