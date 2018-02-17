#pragma once
#include "pch.h"

namespace DX
{
class StepTimer;
}
struct AppContext;
struct Texture;

//------------------------------------------------------------------------------
class Explosions
{
public:
  Explosions(AppContext& context, Texture& texture);

  void reset();
  void update(DX::StepTimer const& timer);
  void render(DirectX::SpriteBatch& batch);
  void emit(
    const DirectX::SimpleMath::Vector3& origin,
    const DirectX::SimpleMath::Vector3& baseVelocity,
    size_t numParticles = 200);

private:
  AppContext& m_context;
  Texture& m_texture;

  struct ExplosionParticle
  {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 velocity;
    float energy = 0.0f;
  };
  static const size_t MAX_NUM_PARTICLES = 2048;
  std::array<ExplosionParticle, MAX_NUM_PARTICLES> m_particles;
  size_t m_nextParticleIdx = 0;

  // Random Generators
  std::random_device m_device;
  std::default_random_engine m_engine;
};

//------------------------------------------------------------------------------
