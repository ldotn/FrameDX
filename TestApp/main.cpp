#define _CRT_SECURE_NO_WARNINGS
#include "Device/Device.h"
#include "Core/Log.h"
#include <io.h>
#include <corecrt_io.h>
#include <thread>
#include <chrono>
#include <conio.h>
#include "Shader/Shaders.h"
#include "Core/Utils.h"
#include "Mesh/Mesh.h"

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

	DirectX::SimpleMath::Vector2 m_fontPos;
	DirectX::SpriteBatch sprite_batch(dev.GetImmediateContext());
	DirectX::SpriteFont font(dev.GetDevice(),L"calibri.spritefont");

	D3D11_VIEWPORT viewport = {};
	viewport.Height = 1080;
	viewport.Width = 1920;
	viewport.MaxDepth = 1;
	viewport.MinDepth = 0;

	FrameDX::VertexShader test_vs;
	test_vs.CreateFromFile(&dev,L"TestVS.hlsl","main");
	FrameDX::PixelShader test_ps;
	test_ps.CreateFromFile(&dev,L"TestPS.hlsl","main");

	D3D11_RASTERIZER_DESC rs_desc = {};
	rs_desc.FillMode = D3D11_FILL_SOLID;
	rs_desc.CullMode = D3D11_CULL_NONE;

	ID3D11RasterizerState* raster_state;
	dev.GetDevice()->CreateRasterizerState(&rs_desc,&raster_state);


	D3D11_DEPTH_STENCIL_DESC ds_desc = {};
	ds_desc.DepthEnable = true;
	ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	ds_desc.DepthFunc = D3D11_COMPARISON_LESS;
	
	ID3D11DepthStencilState* depth_state;
	dev.GetDevice()->CreateDepthStencilState(&ds_desc,&depth_state);

	FrameDX::Mesh<FrameDX::StandardVertex> dbg_obj;
	dbg_obj.LoadFromOBJ(&dev, "test_obj.obj");
	
	// Create pipeline states
	FrameDX::PipelineState mesh_state;
	mesh_state.Output.Viewports = { viewport };
	mesh_state.Output.RTVs = { dev.GetBackbuffer()->RTV };
	mesh_state.Output.DSV = dev.GetZBuffer()->DSV;
	mesh_state.Mesh = dbg_obj.GetContext();
	mesh_state.Output.DepthStencilState = depth_state;
	mesh_state.Output.RasterState = raster_state;
	mesh_state.Shaders[(size_t)FrameDX::ShaderStage::Vertex].ShaderPtr = &test_vs;
	mesh_state.Shaders[(size_t)FrameDX::ShaderStage::Pixel].ShaderPtr = &test_ps;
	mesh_state.BuildInputLayout(&dev);

	FrameDX::PipelineState cs_state;
	cs_state.Shaders[(size_t)FrameDX::ShaderStage::Compute].ShaderPtr = &test_cs;
	cs_state.Shaders[(size_t)FrameDX::ShaderStage::Compute].ResourcesTable = { tmp.SRV };
	cs_state.Output.ComputeShaderUAVs = { dev.GetBackbuffer()->UAV };

	dev.EnterMainLoop([&](double GlobalTimeNanoseconds)
	{
		// Run compute shader
		dev.BindPipelineState(cs_state);
		dev.GetImmediateContext()->Dispatch(ceilf(dev.GetBackbuffer()->Desc.SizeX / test_cs.GroupSizeX), ceilf(dev.GetBackbuffer()->Desc.SizeY / test_cs.GroupSizeY), 1);

		// Render mesh on top of compute shader result
		dev.BindPipelineState(mesh_state);
		dev.GetImmediateContext()->ClearDepthStencilView(dev.GetZBuffer()->DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
		dev.GetImmediateContext()->DrawIndexed(dbg_obj.Desc.IndexCount, 0, 0);

		{
			
			/*sprite_batch.Begin();
			font.DrawString(&sprite_batch, (L"Global FPS : " + to_wstring(1e9/GlobalTimeNanoseconds) + L" FPS").c_str(), DirectX::g_XMZero,DirectX::Colors::White);
			sprite_batch.End();*/
		}

		dev.GetSwapChain()->Present(0,0);
	});

	// Not releasing any resources here... Should implement that eventually

	return 0;
}