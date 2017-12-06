#include "pch.h"
#include "Starfield.h"
#include "AppContext.h"
#include "StepTimer.h"

#include "utils/Log.h"

const float ZBOUNDMIN = 1.0f;
const float ZBOUNDMAX = 8.5f;
const float SCALE_MIN = 0.2f;
const float SCALE_MAX = 0.5f;

const float MILLISECS_PER_SEC = 1000.0f;

using namespace DirectX;
//------------------------------------------------------------------------------
StarField::StarField(AppContext& context, ID3D11ShaderResourceView* texture)
		: m_context(context)
		, m_engine(m_device())
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
			throw std::exception("StarField expects a Texture2D");
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
StarField::initialisePositions()
{
	TRACE
	std::uniform_real_distribution<float> yRand(
		static_cast<float>(-m_textureHeight), m_context.screenHeight);

	std::uniform_real_distribution<float> zRand(ZBOUNDMIN, ZBOUNDMAX);
	std::uniform_real_distribution<float> sizeRand(SCALE_MIN, SCALE_MAX);

	for (auto& l : m_particleLayers)
	{
		for (auto& p : l)
		{
			p.position.x = m_xRand(m_engine);
			p.position.y = yRand(m_engine);
			p.position.z = zRand(m_engine);

			float scale = sizeRand(m_engine);
			p.scale.x		= scale;
			p.scale.y		= scale;
			p.scale.z		= scale;
			p.scale.w		= scale;
		}
	}
}

//------------------------------------------------------------------------------
void
StarField::update(DX::StepTimer const& timer)
{
	TRACE
	float speed = m_context.screenHeight / (m_timePerWrapMs / MILLISECS_PER_SEC);
	const float layerSpeedOffset = speed / (NUM_LAYERS + 2);

	int layerCount = 0;
	for (auto& l : m_particleLayers)
	{
		speed -= layerSpeedOffset;

		for (auto& p : l)
		{
			// Particle Motion
			p.position.y += static_cast<float>(speed * timer.GetElapsedSeconds());

			// Regenerate old particles
			if (p.position.y > m_context.screenHeight)
			{
				p.position.y = static_cast<float>(-m_textureHeight)
											 + (p.position.y - m_context.screenHeight);
				p.position.x = m_xRand(m_engine);
			}
		}
		++layerCount;
	}
}

//------------------------------------------------------------------------------
void
StarField::render(DirectX::SpriteBatch& batch)
{
	TRACE
	XMVECTOR origin = {0.0f, 0.0f, 0.0f};
	for (auto& l : m_particleLayers)
	{
		for (auto& p : l)
		{
			batch.Draw(
				m_texture.Get(),
				XMLoadFloat3(&p.position),
				nullptr,
				Colors::White,
				0.f,
				origin,
				XMLoadFloat4(&p.scale),
				SpriteEffects_None,
				0.f);
		}
	}
}

//------------------------------------------------------------------------------
void
StarField::setWindowSize(float screenWidth, float screenHeight)
{
	UNREFERENCED_PARAMETER(screenHeight);

	TRACE
	m_xRand = std::uniform_real_distribution<float>(
		static_cast<float>(-m_textureWidth), screenWidth);
	initialisePositions();
}

//------------------------------------------------------------------------------