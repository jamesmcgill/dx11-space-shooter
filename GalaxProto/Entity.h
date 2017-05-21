#pragma once
#include <pch.h>

struct ModelData
{
	std::unique_ptr<DirectX::Model> model;
	DirectX::BoundingSphere bound;
};

struct Entity
{
	enum class Type
	{
		Player,
		Enemy
	};

	DirectX::SimpleMath::Vector3 position = {};
	DirectX::SimpleMath::Vector3 velocity = {};
	ModelData* model											= nullptr;
	Type type															= Type::Player;
	bool isColliding											= false;
};
