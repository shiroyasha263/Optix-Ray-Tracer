// Creating this To tackle the problem that is creating buildInputs, 
// with this I will be able to handle multiple build inputs with a class

#pragma once
#include <memory.h>
#include "Params.h"

using std::shared_ptr;
using std::make_shared;

class buildInput {
public:
	virtual void build(OptixBuildInput& build_input, uint32_t& build_input_flags) const = 0;
	virtual SphereicalMesh get_sphere() const{
		return spherical_mesh;
	}
	virtual TriangularMesh get_triangles() const {
		return triangular_mesh;
	}
	virtual MeshType get_mesh_type() const {
		return meshType;
	}
public:
	SphereicalMesh spherical_mesh;
	TriangularMesh triangular_mesh;
	MeshType	   meshType;
	CUdeviceptr	   d_vertex;
	CUdeviceptr	   d_radius;
	CUdeviceptr	   d_vertices;
	CUdeviceptr	   d_indices;
};

class sphereBuild : public buildInput {
public:
	sphereBuild(float3 center, float radius, MaterialType materialType, float3 emission_color,
		float3 diffusion_color, float fuzz, float eta) {
		CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_vertex), sizeof(float3)));
		CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(d_vertex), &center,
			sizeof(float3), cudaMemcpyHostToDevice));

		CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_radius), sizeof(float)));
		CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(d_radius), &radius,
			sizeof(float), cudaMemcpyHostToDevice));

		spherical_mesh.center = center;
		spherical_mesh.radius = radius;
		spherical_mesh.materialType = materialType;
		spherical_mesh.material.diffuse_color = diffusion_color;
		spherical_mesh.material.emission = emission_color;
		spherical_mesh.material.fuzz = fuzz;
		spherical_mesh.material.eta = eta;
		meshType = SPHERICAL;
	};

	virtual void build(OptixBuildInput& build_input, uint32_t& build_input_flags) const override;
};

class triangleBuild : public buildInput {
public:
	triangleBuild(std::vector<float3>& vertices, std::vector<int3>& indices, MaterialType materialType, float3 emission_color,
		float3 diffusion_color, float fuzz, float eta) {

		CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_vertices), vertices.size() * sizeof(float3)));
		CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(d_vertices), vertices.data(),
			vertices.size() * sizeof(float3), cudaMemcpyHostToDevice));

		CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_indices), indices.size() * sizeof(int3)));
		CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(d_indices), indices.data(),
			indices.size() * sizeof(int3), cudaMemcpyHostToDevice));

		triangular_mesh.vertices = vertices;
		triangular_mesh.indices = indices;
		triangular_mesh.materialType = materialType;
		triangular_mesh.material.diffuse_color = diffusion_color;
		triangular_mesh.material.emission = emission_color;
		triangular_mesh.material.fuzz = fuzz;
		triangular_mesh.material.eta = eta;
		meshType = TRIANGULAR;
	};

	virtual void build(OptixBuildInput& build_input, uint32_t& build_input_flags) const override;
};

class buildInputList {
public:
	buildInputList() {}
	buildInputList(shared_ptr<buildInput> object) { add(object); }

	void clear() { objects.clear(); }
	void add(shared_ptr<buildInput> object) { objects.push_back(object); }

public:
	std::vector<shared_ptr<buildInput>> objects;
};