#include "pch.h"
#include "Explosions.h"
#include "StepTimer.h"
#include "AppContext.h"

#include "utils/Log.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
constexpr float VELOCITY_MIN	= 20.0f;
constexpr float VELOCITY_MAX	= 30.0f;
constexpr float ENERGY_MIN		= 0.8f;		 // Energy controls particle life
constexpr float ENERGY_MAX		= 1.0f;
constexpr float ORIGIN_SPREAD = 2.0f;
constexpr float SCALE_MIN			= 0.5f;
constexpr float SCALE_MAX			= 3.0f;
constexpr float SATURATION		= 0.4f;

using UniRandFloat = std::uniform_real_distribution<float>;
static UniRandFloat thetaRand(0.0f, XM_2PI);
static UniRandFloat velocityRand(VELOCITY_MIN, VELOCITY_MAX);
static UniRandFloat originRand(-ORIGIN_SPREAD, ORIGIN_SPREAD);
static UniRandFloat energyRand(ENERGY_MIN, ENERGY_MAX);

//------------------------------------------------------------------------------
Explosions::Explosions(AppContext& context, ID3D11ShaderResourceView* texture)
		: m_engine(m_device())
		, m_context(context)
{
	TRACE
	m_texture = texture;

	if (texture)
	{
		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		texture->GetResource(resource.GetAddressOf());

		D3D11_RESOURCE_DIMENSION dim;
		resource->GetType(&dim);

		if (dim != D3D11_RESOURCE_DIMENSION_TEXTURE2D)
		{
			throw std::exception("Explosions expects a Texture2D");
		}

		Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2D;
		resource.As(&tex2D);

		D3D11_TEXTURE2D_DESC desc;
		tex2D->GetDesc(&desc);

		m_textureWidth	= desc.Width;
		m_textureHeight = desc.Height;
	}
}

//------------------------------------------------------------------------------
void
Explosions::update(DX::StepTimer const& timer)
{
	TRACE
	float elapsedTimeS = float(timer.GetElapsedSeconds());

	for (auto& p : m_particles)
	{
		if (p.energy == 0.0f)
		{
			continue;
		}

		p.position = p.velocity * elapsedTimeS + p.position;
		p.energy -= elapsedTimeS;
		if (p.energy < 0.0f)
		{
			p.energy = 0.0f;
		}
	}
}

//------------------------------------------------------------------------------
void
Explosions::render(DirectX::SpriteBatch& batch)
{
	TRACE
	XMVECTOR origin = {m_textureWidth / 2.0f, m_textureHeight / 2.0f, 0.0f};
	for (auto& p : m_particles)
	{
		if (p.energy == 0.0f)
		{
			continue;
		}
		float energyRatio = p.energy / (ENERGY_MAX - SATURATION);
		float saturation	= (energyRatio > 1.0f) ? energyRatio - 1.0f : 0.0f;
		Vector4 color			= Colors::Orange;
		color.x += saturation;
		color.y += saturation;
		color.z += saturation;
		color.w = energyRatio;

		float scaleFactor = (energyRatio * (SCALE_MAX - SCALE_MIN)) + SCALE_MIN;
		XMVECTOR scale		= {scaleFactor, scaleFactor, scaleFactor, 1.0f};

		batch.Draw(
			m_texture.Get(),
			XMLoadFloat3(&p.position),
			nullptr,
			color,
			0.f,
			origin,
			scale,
			SpriteEffects_None,
			0.f);
	}
}

//------------------------------------------------------------------------------
void
Explosions::setWindowSize(int screenWidth, int screenHeight)
{
	m_screenWidth	= screenWidth;
	m_screenHeight = screenHeight;
}

//------------------------------------------------------------------------------
void
Explosions::emit(
	const Vector3& origin, const Vector3& baseVelocity, size_t numParticles)
{
	TRACE
	// Hack to allow continue using SpriteBatch with world space objects
	// Generate matrix to invert the default in
	// SpriteBatch::Impl::GetViewportTransform()
	float xScale = m_screenWidth / 2.0f;
	float yScale = m_screenHeight / 2.0f;

	// clang-format off
	Matrix inv{
		xScale,	0,				0,	0,
		0,			-yScale,	0,	0,
		0,			0,				1,	0,
		xScale,	yScale,		0,	1};
	// clang-format on
	Matrix viewProj = m_context.view * m_context.proj * inv;

	for (size_t i = 0; i < numParticles; ++i)
	{
		auto& p = m_particles[m_nextParticleIdx];

		// Position
		float spreadDistance = originRand(m_engine);
		float theta					 = thetaRand(m_engine);
		float sin, cos;
		XMScalarSinCos(&sin, &cos, theta);
		Vector3 spread(cos, sin, 0.0f);
		Vector3 pos = origin + (spread * spreadDistance);
		p.position	= Vector3::Transform(pos, viewProj);

		// Velocity
		float velocity = velocityRand(m_engine);
		p.velocity		 = baseVelocity + (-spread * velocity);

		// Energy
		p.energy = energyRand(m_engine);

		m_nextParticleIdx++;
		if (m_nextParticleIdx >= MAX_NUM_PARTICLES)
		{
			m_nextParticleIdx = 0;
		}
	}
}

//------------------------------------------------------------------------------
