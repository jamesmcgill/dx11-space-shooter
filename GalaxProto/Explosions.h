#pragma once
#include "pch.h"
#include <array>
#include <random>

namespace DX
{
class StepTimer;
}
struct AppContext;

//------------------------------------------------------------------------------
class Explosions
{
public:
	Explosions(AppContext& context, ID3D11ShaderResourceView* texture);
	void update(DX::StepTimer const& timer);
	void render(DirectX::SpriteBatch& batch);
	void setWindowSize(int screenWidth, int screenHeight);
	void emit(
		const DirectX::SimpleMath::Vector3& origin,
		const DirectX::SimpleMath::Vector3& baseVelocity,
		size_t numParticles = 100);

private:
	struct ExplosionParticle
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 velocity;
	};
	static const size_t MAX_NUM_PARTICLES = 256;
	std::array<ExplosionParticle, MAX_NUM_PARTICLES> m_particles;
	size_t m_nextParticleIdx = 0;

	// Random Generators
	std::random_device m_device;
	std::default_random_engine m_engine;

	int m_screenWidth	= 0;
	int m_screenHeight = 0;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	int m_textureWidth	= 0;
	int m_textureHeight = 0;

	AppContext& m_context;
};

//------------------------------------------------------------------------------