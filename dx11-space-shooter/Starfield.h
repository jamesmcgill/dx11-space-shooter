#pragma once
#include "pch.h"
#include <array>
#include <random>

struct AppContext;
namespace DX
{
class StepTimer;
}

//------------------------------------------------------------------------------
class StarField
{
public:
	StarField(AppContext& context, ID3D11ShaderResourceView* texture);
	void update(DX::StepTimer const& timer);
	void render(DirectX::SpriteBatch& batch);
	void setWindowSize(float screenWidth, float screenHeight);

	enum SPEED_TimePerScreenWrapMs
	{
		SPEED_Slow	 = 5000,
		SPEED_Medium = 2500,
		SPEED_Fast	 = 1200
	};
	void setSpeed(SPEED_TimePerScreenWrapMs speed)
	{
		m_timePerWrapMs = speed;
	}

private:
	AppContext& m_context;

	struct StarParticle
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 scale;
	};
	static const size_t MAX_NUM_PARTICLES = 256;
	static const size_t NUM_LAYERS				= 6;
	using ParticleLayer = std::array<StarParticle, MAX_NUM_PARTICLES>;
	std::array<ParticleLayer, NUM_LAYERS> m_particleLayers;

	// Random Generators
	std::random_device m_device;
	std::default_random_engine m_engine;
	std::uniform_real_distribution<float> m_xRand;

	SPEED_TimePerScreenWrapMs m_timePerWrapMs = SPEED_Medium;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	int m_textureWidth	= 0;
	int m_textureHeight = 0;

private:
	void initialisePositions();
};

//------------------------------------------------------------------------------
