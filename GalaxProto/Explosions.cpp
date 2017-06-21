#include "pch.h"
#include "Explosions.h"
#include "StepTimer.h"
#include "AppContext.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
Explosions::Explosions(AppContext& context, ID3D11ShaderResourceView* texture)
		: m_engine(m_device())
		, m_context(context)
{
	m_texture = texture;

	if (texture) {
		Microsoft::WRL::ComPtr<ID3D11Resource> resource;
		texture->GetResource(resource.GetAddressOf());

		D3D11_RESOURCE_DIMENSION dim;
		resource->GetType(&dim);

		if (dim != D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
			throw std::exception("ScrollingBackground expects a Texture2D");
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
	float elapsedTimeS = float(timer.GetElapsedSeconds());

	// Vector3 frictionNormal = -player.velocity;
	// frictionNormal.Normalize();
	// m_context.playerAccel += PLAYER_FRICTION * frictionNormal;

	const Vector3 accel = Vector3();
	for (auto& p : m_particles)
	{
		p.position = 0.5f * accel * (elapsedTimeS * elapsedTimeS)
								 + p.velocity * elapsedTimeS + p.position;
		// p.velocity = accel * elapsedTimeS + p.velocity;

		// TODO:: Kill old particles
	}
}

//------------------------------------------------------------------------------
void
Explosions::render(DirectX::SpriteBatch& batch)
{
	XMVECTOR origin = {m_textureWidth / 2.0f, m_textureHeight / 2.0f, 0.0f};
	XMVECTOR scale	= {5.0f, 5.0f, 5.0f, 1.0f};
	for (auto& p : m_particles)
	{
		batch.Draw(
			m_texture.Get(),
			XMLoadFloat3(&p.position),
			nullptr,
			Colors::Orange,
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

	std::uniform_real_distribution<float> thetaRand(0.0f, XM_2PI);

	static const float MIN_VELOCITY = 100.0f;
	static const float MAX_VELOCITY = 100.0f;
	std::uniform_real_distribution<float> velocityRand(MIN_VELOCITY, MAX_VELOCITY);

	

	for (size_t i = 0; i < numParticles; ++i)
	{
		auto& p = m_particles[m_nextParticleIdx];
		p.position = Vector3::Transform(origin, viewProj);

		float theta = thetaRand(m_engine);
		float sin, cos;
		XMScalarSinCos(&sin, &cos, theta);

		float velocity = velocityRand(m_engine);
		p.velocity = baseVelocity;
		p.velocity.x += cos * velocity;
		p.velocity.y += sin * velocity;

		m_nextParticleIdx++;
		if (m_nextParticleIdx >= MAX_NUM_PARTICLES) {
			m_nextParticleIdx = 0;
		}
	}
}

//------------------------------------------------------------------------------
