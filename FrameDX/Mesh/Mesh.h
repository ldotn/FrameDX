#pragma once
#include "stdafx.h"
#include "../Core/Binding.h"

namespace FrameDX
{
	using namespace DirectX::SimpleMath;

	struct StandardVertex
	{
		static const vector<D3D11_INPUT_ELEMENT_DESC> LayoutDesc;

		Vector3 Position;
		Vector3 Normal;
		Vector3 Tangent;
		Vector2 UV;

		bool operator==(const StandardVertex& other) const
		{
			return Position == other.Position &&
				   Normal   == other.Normal   &&
				   Tangent  == other.Tangent  &&
				   UV       == other.UV;
		}
	};


	// Mesh that can be loaded from an obj file
	// It derives from Bindable so ScopedBind can be used
	// Templated on the vertex type
	// The vertex type also needs to specify the "hash" template on std for the type and implement the == comparison operator
	template<typename VertexType>
	class Mesh : public Bindable
	{
	public:
		Mesh()
		{
			ID3D11Buffer* VertexBuffer = nullptr;
			ID3D11Buffer* IndexBuffer = nullptr;
			ID3D11InputLayout* Layout = nullptr;
			uint32_t IndexCount = -1;
		}

		struct Description
		{
			Description()
			{
				VertexCount = -1;
				TriangleCount = -1;
			}

			uint32_t VertexCount;
			uint32_t TriangleCount;
		} Desc;

		// Creates a new mesh by loading an OBJ file
		// Takes two templated lambdas, one returns a vertex from the OBJ data for that vertex
		//		the other is called after the vertex list is generated, to do any post-process necessary
		// There is a default lambda for the StandardVertex type
		// Materials are ignored
		StatusCode LoadFromOBJ( Device * OwnerDev,
								string FilePath,
								function<VertexType(const tinyobj::attrib_t&,const tinyobj::index_t&)> VertexCallback = StandardVertexCallback,
								function<void(vector<VertexType>&,vector<uint32_t>&)> PostprocessCallback = StandardPostprocessCallback )
		{
			vector<VertexType> vertexs;
			vector<uint32_t> indexes;

			tinyobj::attrib_t attrib;
			vector<tinyobj::shape_t> shapes;
			vector<tinyobj::material_t> materials;

			string err;
			bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, FilePath.c_str(), nullptr, true);

			wstring werr(err.begin(), err.end());
			if (!werr.empty())
				LogMsg(werr, LogCategory::Error);

			if (!ret)
				return StatusCode::Failed;

			unordered_map<VertexType, uint32_t> unique_vertices = {};

			size_t reserve_size = 0;
			for (const auto& shape : shapes)
				reserve_size += shape.mesh.indices.size() * 3;
			vertexs.reserve(reserve_size);
			indexes.reserve(reserve_size);

			for (const auto& shape : shapes)
			{
				for (const auto& index : shape.mesh.indices)
				{
					VertexType vertex = VertexCallback(attrib, index);

					if (!unique_vertices.count(vertex))
					{
						unique_vertices[vertex] = static_cast<uint32_t>(vertexs.size());
						vertexs.push_back(vertex);
					}

					indexes.push_back(unique_vertices[vertex]);
				}
			}

			PostprocessCallback(vertexs, indexes);

			ID3D11Buffer * vertex_buffer;
			FrameDX::CreateBufferFromVector(vertexs, *OwnerDev, D3D11_BIND_VERTEX_BUFFER, &vertex_buffer);
			ID3D11Buffer * index_buffer;
			FrameDX::CreateBufferFromVector(indexes, *OwnerDev, D3D11_BIND_INDEX_BUFFER, &index_buffer);

			OwnerDevice = OwnerDev;
			IndexCount = indexes.size();
			Desc.VertexCount = vertexs.size();
			Desc.TriangleCount = IndexCount / 3;

			return StatusCode::Ok;
		}

		// IMPLEMENT THIS!
		virtual StatusCode Bind() final override 
		{ 
			//problema: si pongo esto aca se va a hacer muchas veces al pedo, porque muchos meshes van a usar lo mismo
			//recordar las bindeadas no sirve porque cuando unbindeas el mesh tenes que unbindearlas, entonces el siguiente mesh las va a tener que bindear

			dev.GetImmediateContext()->IASetInputLayout(in_layout);
			dev.GetImmediateContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			
			dev.GetImmediateContext()->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);
			UINT stride = sizeof(Vertex);
			UINT offset = 0;
			dev.GetImmediateContext()->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

			return StatusCode::NotImplemented;
		}
		virtual StatusCode Unbind() final override 
		{ 
			return StatusCode::NotImplemented; 
		}
	private:
		static StandardVertex StandardVertexCallback(const tinyobj::attrib_t& VertexAttributes,const tinyobj::index_t& Indexes)
		{
			StandardVertex vertex;

			vertex.Position = Vector3
			(
				VertexAttributes.vertices[3 * Indexes.vertex_index + 0],
				VertexAttributes.vertices[3 * Indexes.vertex_index + 1],
				VertexAttributes.vertices[3 * Indexes.vertex_index + 2]
			);

			vertex.Normal = Vector3
			(
				VertexAttributes.normals[3 * Indexes.normal_index + 0],
				VertexAttributes.normals[3 * Indexes.normal_index + 1],
				VertexAttributes.normals[3 * Indexes.normal_index + 2]
			);

			// Tangent is computed later

			vertex.UV = Vector2
			(
				VertexAttributes.texcoords[2 * Indexes.texcoord_index + 0],
				VertexAttributes.texcoords[2 * Indexes.texcoord_index + 1]
			);

			return vertex;
		}

		static void StandardPostprocessCallback(vector<StandardVertex>& VertexData, vector<uint32_t>& IndexData)
		{
			// Compute tangents
		}

		ID3D11Buffer* VertexBuffer;
		ID3D11Buffer* IndexBuffer;
		ID3D11InputLayout* Layout;
		uint32_t IndexCount;
	};
}

namespace std
{
	template<> struct hash<FrameDX::StandardVertex>
	{
		size_t operator()(FrameDX::StandardVertex const& vertex) const
		{
			// Hashing the raw bytes interpreted as a string
			return hash<string>()(string((const char *)&vertex,sizeof(FrameDX::StandardVertex)));
		}
	};
}