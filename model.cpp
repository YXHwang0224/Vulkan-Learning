#include "model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace FF {

	void Model::loadModel(const std::string& path, const Wrapper::Device::Ptr& device) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
		std::string warn;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
			throw std::runtime_error("Error: failed to load model");
		}

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				//先取出顶点位置数据
				myPositions.push_back(attrib.vertices[3 * index.vertex_index + 0]);
				myPositions.push_back(attrib.vertices[3 * index.vertex_index + 1]);
				myPositions.push_back(attrib.vertices[3 * index.vertex_index + 2]);

				//取出uv值
				myUVs.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
				myUVs.push_back(1.0f - attrib.texcoords[2 * index.texcoord_index + 1]);

				//索引
				myIndexDatas.push_back(myIndexDatas.size());
			}
		}

		myPositionBuffers = Wrapper::Buffer::createVertexBuffer(device, myPositions.size() * sizeof(float), myPositions.data());

		myUVBuffers = Wrapper::Buffer::createVertexBuffer(device, myUVs.size() * sizeof(float), myUVs.data());

		myIndexBuffers = Wrapper::Buffer::createIndexBuffer(device, myIndexDatas.size() * sizeof(float), myIndexDatas.data());
	}
}