#define _CRT_SECURE_NO_WARNINGS 
#include "Device/Device.h"
#include "Core/Log.h"
#include <io.h>
#include <corecrt_io.h>
#include <thread>
#include <chrono>
#include <conio.h>
#include "Shader/Shader.h"
#include "Core/Utils.h"

using namespace std;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR, int)
{
    AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	thread log_printer([]()
	{
		size_t index = 0;
		FrameDX::TimedLoop([&]()
		{
			index += FrameDX::Log.PrintRange(wcout,index);
		},250ms);
	});
	log_printer.detach();

	FrameDX::Device::KeyboardCallback = [](WPARAM key, FrameDX::KeyAction action)
	{
		if(key == 'W' && action == FrameDX::KeyAction::Up)
			LogAssert(false,FrameDX::LogCategory::Error);
	};

	FrameDX::Device dev;

	auto desc = FrameDX::Device::Description();
	desc.WindowDescription.SizeX = 1920;
	desc.WindowDescription.SizeY = 1080;
	desc.SwapChainDescription.BackbufferAccessFlags |= DXGI_USAGE_UNORDERED_ACCESS;
	desc.SwapChainDescription.BackbufferAccessFlags |= DXGI_USAGE_SHADER_INPUT;
	LogCheck(dev.Start(desc),FrameDX::LogCategory::CriticalError);

	FrameDX::Texture2D tmp;
	
	auto tex_desc = FrameDX::Texture2D::Description();
	tex_desc.SizeX = dev.GetBackbuffer()->Desc.SizeX;
	tex_desc.SizeY = dev.GetBackbuffer()->Desc.SizeY;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

	vector<uint8_t> img_data(tex_desc.SizeX*tex_desc.SizeY*4);
	for(int y = 0;y < tex_desc.SizeY;y++)
	{
		static int i = 0;
		for(int x = 0;x < tex_desc.SizeX;x++)
		{
			img_data[i++] = (x/float(tex_desc.SizeX))*255;
			img_data[i++] = (y/float(tex_desc.SizeY))*255;
			img_data[i++] = 0;
			img_data[i++] = 255;
		}
	}
	tmp.CreateFromDescription(&dev,tex_desc,img_data);

	FrameDX::ComputeShader test_cs;
	test_cs.CreateFromFile(&dev,L"TestCS.hlsl","main");

	test_cs.LinkSRV(tmp.SRV,0);
	test_cs.LinkUAV(dev.GetBackbuffer()->UAV,0);
	
	/*DirectX::SimpleMath::Vector2 m_fontPos;
	DirectX::SpriteBatch sprite_batch(dev.GetImmediateContext());
	DirectX::SpriteFont font(dev.GetDevice(),L"calibri.spritefont");*/

	dev.EnterMainLoop([&]()
	{
		{
			ScopedBind(test_cs);
			// This handles the division by group size automatically
			test_cs.Dispatch(dev,dev.GetBackbuffer()->Desc.SizeX,dev.GetBackbuffer()->Desc.SizeY);
		}
		
		/*D3D11_VIEWPORT viewport;
		viewport.Height = 1080;
		viewport.Width = 1920;
		viewport.MaxDepth = 1;
		viewport.MinDepth = 0;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		dev.GetImmediateContext()->RSSetViewports(1,&viewport);
		dev.GetImmediateContext()->OMSetRenderTargets(1,&dev.GetBackbuffer()->RTV,dev.GetBackbuffer()->DSV);

		sprite_batch.Begin();

		font.DrawString(&sprite_batch, to_wstring(10).c_str(), DirectX::g_XMZero,DirectX::Colors::White);

		sprite_batch.End();*/

		dev.GetSwapChain()->Present(0,0);
	});

	return 0;
}