#pragma once
#include "stdafx.h"
#include "../Core/PipelineState.h"
#include "../Core/Utils.h"

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
	// Templated on the vertex type
	// The vertex type also needs to specify the "hash" template on std for the type and implement the == comparison operator
	template<typename VertexType>
	class Mesh
	{
	public:
		Mesh()
		{
			Data.LayoutDesc = &VertexType::LayoutDesc;
			Data.PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			Data.IndexFormat = DXGI_FORMAT_R32_UINT;
			Data.VertexStride = sizeof(VertexType);
		}

		~Mesh()
		{
			Data.IndexBuffer->Release();
			Data.VertexBuffer->Release();
		}

		struct Description
		{
			Description()
			{
				VertexCount = -1;
				TriangleCount = -1;
				IndexCount = -1;
			}

			uint32_t VertexCount;
			uint32_t TriangleCount;
			uint32_t IndexCount;
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

			string err, warn;
			bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, FilePath.c_str(), nullptr, true);

			wstring werr(err.begin(), err.end());
			if (!werr.empty())
				LogMsg(werr, LogCategory::Error);

			wstring wwarn(warn.begin(), warn.end());
			if (!wwarn.empty())
				LogMsg(wwarn, LogCategory::Warning);

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

			FrameDX::CreateBufferFromVector(vertexs, *OwnerDev, D3D11_BIND_VERTEX_BUFFER, &Data.VertexBuffer);
			FrameDX::CreateBufferFromVector(indexes, *OwnerDev, D3D11_BIND_INDEX_BUFFER, &Data.IndexBuffer);

			Desc.IndexCount = indexes.size();
			Desc.VertexCount = vertexs.size();
			Desc.TriangleCount = Desc.IndexCount / 3;
			
			return StatusCode::Ok;
		}

		MeshContext GetContext() { return Data; }
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

		MeshContext Data;
		
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