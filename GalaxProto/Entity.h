#pragma once
#include <pch.h>

struct ModelData
{
	std::unique_ptr<DirectX::Model> model;
	DirectX::BoundingSphere bound;
};

struct Entity
{
	DirectX::SimpleMath::Vector3 position = {};
	DirectX::SimpleMath::Vector3 velocity = {};
	ModelData* model											= nullptr;
	bool isColliding											= false;
	bool isAlive													= false;
};
