#include "pch.h"
#include "Explosions.h"
#include "StepTimer.h"
#include "AppContext.h"
#include "AppResources.h"

#include "utils/Log.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//------------------------------------------------------------------------------
constexpr float VELOCITY_MIN  = 20.0f;
constexpr float VELOCITY_MAX  = 30.0f;
constexpr float ENERGY_MIN    = 0.8f;    // Energy controls particle life
constexpr float ENERGY_MAX    = 1.0f;
constexpr float ORIGIN_SPREAD = 2.0f;
constexpr float SCALE_MIN     = 0.5f;
constexpr float SCALE_MAX     = 3.0f;
constexpr float SATURATION    = 0.4f;

using UniRandFloat = std::uniform_real_distribution<float>;
static UniRandFloat thetaRand(0.0f, XM_2PI);
static UniRandFloat velocityRand(VELOCITY_MIN, VELOCITY_MAX);
static UniRandFloat originRand(-ORIGIN_SPREAD, ORIGIN_SPREAD);
static UniRandFloat energyRand(ENERGY_MIN, ENERGY_MAX);

//------------------------------------------------------------------------------
Explosions::Explosions(AppContext& context, Texture& texture)
    : m_context(context)
    , m_texture(texture)
    , m_engine(m_device())
{
}

//------------------------------------------------------------------------------
void
Explosions::reset()
{
  TRACE
  for (auto& p : m_particles)
  {
    p.energy = 0.0f;
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
  XMVECTOR origin = {m_texture.width / 2.0f, m_texture.height / 2.0f, 0.0f};
  for (auto& p : m_particles)
  {
    if (p.energy == 0.0f)
    {
      continue;
    }
    float energyRatio = p.energy / (ENERGY_MAX - SATURATION);
    float saturation  = (energyRatio > 1.0f) ? energyRatio - 1.0f : 0.0f;
    Vector4 color(Colors::Orange);
    color.x += saturation;
    color.y += saturation;
    color.z += saturation;
    color.w = energyRatio;

    float scaleFactor = (energyRatio * (SCALE_MAX - SCALE_MIN)) + SCALE_MIN;
    XMVECTOR scale    = {scaleFactor, scaleFactor, scaleFactor, 1.0f};

    batch.Draw(
      m_texture.texture.Get(),
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
Explosions::emit(
  const Vector3& origin, const Vector3& baseVelocity, size_t numParticles)
{
  TRACE
  // Hack to allow using SpriteBatch with world space objects.
  // SpriteBatch requires everything in x-right y-down screen PIXEL coordinates
  // and does it's own orthographic projection internally.
  // See SpriteBatch::Impl::GetViewportTransform()
  Matrix worldToScreen = m_context.worldToView * m_context.viewToProjection
                         * m_context.projectionToPixels;

  for (size_t i = 0; i < numParticles; ++i)
  {
    auto& p = m_particles[m_nextParticleIdx];

    // Position
    const float spreadDistance = originRand(m_engine);
    const float theta          = thetaRand(m_engine);
    float sin, cos;
    XMScalarSinCos(&sin, &cos, theta);
    Vector3 spread(cos, sin, 0.0f);
    Vector3 pos = origin + (spread * spreadDistance);
    p.position  = Vector3::Transform(pos, worldToScreen);

    // Velocity
    const float velocity = velocityRand(m_engine);
    p.velocity           = baseVelocity + (-spread * velocity);

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
