#pragma once

struct Entity
{
	enum class Type {
		Player,
		Enemy
	};

	DirectX::SimpleMath::Vector3 m_position = {};
	DirectX::SimpleMath::Vector3 m_velocity = {};
	Type type = Type::Player;
};

