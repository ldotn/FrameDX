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
		auto period = 250ms;
		FrameDX::TimedLoop([&]()
		{
			index += FrameDX::Log.PrintRange(wcout,index);
		}, period);
	});
	log_printer.detach();

	FrameDX::Device::KeyboardCallback = [](WPARAM key, FrameDX::KeyAction action)
	{
		if(key == 'W' && action == FrameDX::KeyAction::Up)
			LogAssert(false,FrameDX::LogCategory::Error);
	};

	FrameDX::Device dev;
	{
		auto desc = FrameDX::Device::Description();
		desc.WindowDescription.SizeX = 1920;
		desc.WindowDescription.SizeY = 1080;
		desc.SwapChainDescription.BackbufferAccessFlags |= DXGI_USAGE_UNORDERED_ACCESS;
		desc.SwapChainDescription.BackbufferAccessFlags |= DXGI_USAGE_SHADER_INPUT;
		LogCheck(dev.Start(desc), FrameDX::LogCategory::CriticalError);
	}

	FrameDX::Texture2D tmp;
	{
		auto tex_desc = FrameDX::Texture2D::Description();
		tex_desc.SizeX = dev.GetBackbuffer()->Desc.SizeX;
		tex_desc.SizeY = dev.GetBackbuffer()->Desc.SizeY;
		tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

		vector<uint8_t> img_data(tex_desc.SizeX*tex_desc.SizeY * 4);
		for (int y = 0; y < tex_desc.SizeY; y++)
		{
			static int i = 0;
			for (int x = 0; x < tex_desc.SizeX; x++)
			{
				img_data[i++] = (x / float(tex_desc.SizeX)) * 255;
				img_data[i++] = (y / float(tex_desc.SizeY)) * 255;
				img_data[i++] = 0;
				img_data[i++] = 255;
			}
		}
		tmp.CreateFromDescription(&dev, tex_desc, img_data);
	}
	
	FrameDX::ComputeShader test_cs;
	test_cs.CreateFromFile(&dev,L"TestCS.hlsl","main");

	D3D11_VIEWPORT viewport = {};
	viewport.Height = 1080;
	viewport.Width = 1920;
	viewport.MaxDepth = 1;
	viewport.MinDepth = 0;

	FrameDX::VertexShader test_vs;
	test_vs.CreateFromFile(&dev,L"TestVS.hlsl","main");
	FrameDX::PixelShader test_ps;
	test_ps.CreateFromFile(&dev,L"TestPS.hlsl","main");

	ID3D11RasterizerState* raster_state;
	{
		D3D11_RASTERIZER_DESC rs_desc = {};
		rs_desc.FillMode = D3D11_FILL_SOLID;
		rs_desc.CullMode = D3D11_CULL_NONE;

		dev.GetDevice()->CreateRasterizerState(&rs_desc, &raster_state);
	}
	
	ID3D11DepthStencilState* depth_state;
	{
		D3D11_DEPTH_STENCIL_DESC ds_desc = {};
		ds_desc.DepthEnable = true;
		ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		ds_desc.DepthFunc = D3D11_COMPARISON_LESS;

		dev.GetDevice()->CreateDepthStencilState(&ds_desc, &depth_state);
	}

	FrameDX::Mesh<FrameDX::StandardVertex> dbg_obj;
	dbg_obj.LoadFromOBJ(&dev, "test_obj.obj");

	// Create constant buffers
	struct MeshCB
	{
		DirectX::XMFLOAT4X4 WVP;
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT3 Color;

		uint32_t padding_;
	};

	struct GlobalCB
	{
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 Proj;
		DirectX::XMFLOAT3 CameraPos;

		uint32_t padding_;
	};

	ID3D11Buffer* cb_buffer_mesh;
	ID3D11Buffer* cb_buffer_global;
	{
		// Ensure 16 byte alignement 
		static_assert(sizeof(MeshCB) % 16 == 0);
		static_assert(sizeof(GlobalCB) % 16 == 0);

		D3D11_BUFFER_DESC cb_desc;
		
		cb_desc.ByteWidth = sizeof(MeshCB);
		cb_desc.Usage = D3D11_USAGE_DYNAMIC;
		cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cb_desc.MiscFlags = 0;
		cb_desc.StructureByteStride = 0;

		LogCheck(dev.GetDevice()->CreateBuffer(&cb_desc, nullptr, &cb_buffer_mesh), FrameDX::LogCategory::CriticalError);

		cb_desc.ByteWidth = sizeof(GlobalCB);
		cb_desc.Usage = D3D11_USAGE_DYNAMIC;
		cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cb_desc.MiscFlags = 0;
		cb_desc.StructureByteStride = 0;

		LogCheck(dev.GetDevice()->CreateBuffer(&cb_desc, nullptr, &cb_buffer_global), FrameDX::LogCategory::CriticalError);
	}

	// Create pipeline states
	FrameDX::PipelineState mesh_state;
	mesh_state.Output.Viewports = { viewport };
	mesh_state.Output.RTVs = { dev.GetBackbuffer()->RTV };
	mesh_state.Output.DSV = dev.GetZBuffer()->DSV;
	mesh_state.Mesh = dbg_obj.GetContext();
	mesh_state.Output.DepthStencilState = depth_state;
	mesh_state.Output.RasterState = raster_state;
	mesh_state.Shaders[(size_t)FrameDX::ShaderStage::Vertex].ConstantBuffersTable = { cb_buffer_mesh, cb_buffer_global};
	mesh_state.Shaders[(size_t)FrameDX::ShaderStage::Vertex].ShaderPtr = &test_vs;
	mesh_state.Shaders[(size_t)FrameDX::ShaderStage::Pixel].ShaderPtr = &test_ps;
	mesh_state.Shaders[(size_t)FrameDX::ShaderStage::Pixel].ConstantBuffersTable = { cb_buffer_mesh, cb_buffer_global };
	mesh_state.BuildInputLayout(&dev);

	FrameDX::PipelineState cs_state;
	cs_state.Shaders[(size_t)FrameDX::ShaderStage::Compute].ShaderPtr = &test_cs;
	cs_state.Shaders[(size_t)FrameDX::ShaderStage::Compute].ResourcesTable = { tmp.SRV };
	cs_state.Output.ComputeShaderUAVs = { dev.GetBackbuffer()->UAV };

	// Update global cbuffer
	DirectX::XMMATRIX view_mat, proj_mat;
	// Define the mouse loop here to update the variables
	FrameDX::Device::MouseCallback = [&](WPARAM wParam,int MouseX,int MouseY)
	{
		using namespace FrameDX;
		GlobalCB cb_data;

		static float angle_v = 0;
		static float angle_h = 0;
		static float div_factor = max(viewport.Width, viewport.Height);

		static int prev_mouse_x = 0;
		static int prev_mouse_y = 0;

		angle_h += 2.0f*(-(MouseX - prev_mouse_x) / div_factor);
		angle_v += 2.0f*( (MouseY - prev_mouse_y) / div_factor);

		prev_mouse_x = MouseX;
		prev_mouse_y = MouseY;

		DirectX::XMVECTOR camera_offset_vec = { 0,0,1,1 };
		camera_offset_vec = DirectX::XMVector3Transform(
				DirectX::XMVector3Transform(camera_offset_vec,
					DirectX::XMMatrixRotationAxis({ 1,0,0 }, -angle_v)),
				DirectX::XMMatrixRotationAxis({ 0,1,0 }, angle_h));

		const float distance = 2.0f;
		auto cam_pos = DirectX::XMLoadFloat3(&cb_data.CameraPos);
		cam_pos = DirectX::XMVectorAdd(cam_pos, DirectX::XMVectorMultiply({ distance,distance,distance,distance }, camera_offset_vec));

		DirectX::XMStoreFloat3(&cb_data.CameraPos, cam_pos);
		view_mat = DirectX::XMMatrixLookAtLH(cam_pos, { 0,0,0 }, { 0,1,0 });
		proj_mat = DirectX::XMMatrixPerspectiveFovLH(90_deg, viewport.Width / viewport.Height, 0.01f, 1000.0f);

		DirectX::XMStoreFloat4x4(&cb_data.View,view_mat);
		DirectX::XMStoreFloat4x4(&cb_data.Proj,proj_mat);

		dev.UpdateBuffer(cb_buffer_global, cb_data);
	};
	// Ensure it's called at least once. Kinda hacky though...
	FrameDX::Device::MouseCallback(0, 0, 0);

	// Main loop starts here
	dev.EnterMainLoop([&](double GlobalTimeNanoseconds)
	{
		float clear_color[4] = { 0,0,0,1 };
		dev.GetImmediateContext()->ClearRenderTargetView(dev.GetBackbuffer()->RTV, clear_color);

		// Run compute shader
		dev.BindPipelineState(cs_state);
		dev.GetImmediateContext()->Dispatch(ceilf(dev.GetBackbuffer()->Desc.SizeX / test_cs.GroupSizeX), ceilf(dev.GetBackbuffer()->Desc.SizeY / test_cs.GroupSizeY), 1);
		

		// Render mesh on top of the compute shader result
		//		Update per-mesh cb
		{
			MeshCB cb_data;

			cb_data.Color = { 1,1,1 };
			auto world_mat = DirectX::XMMatrixIdentity();
			DirectX::XMStoreFloat4x4(&cb_data.World, world_mat);
			DirectX::XMStoreFloat4x4(&cb_data.WVP,
				DirectX::XMMatrixTranspose(
				DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(world_mat, view_mat), proj_mat)));

			dev.UpdateBuffer(cb_buffer_mesh, cb_data);
		}
		dev.BindPipelineState(mesh_state);
		dev.GetImmediateContext()->ClearDepthStencilView(dev.GetZBuffer()->DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
		dev.GetImmediateContext()->DrawIndexed(dbg_obj.Desc.IndexCount, 0, 0);

		dev.GetSwapChain()->Present(0,0);
	});

	// Not releasing any resources here... Should implement that eventually

	return 0;
}