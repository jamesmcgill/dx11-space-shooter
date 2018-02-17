#include "pch.h"
#include "AppResources.h"

#include "utils/Log.h"

//------------------------------------------------------------------------------
void
Texture::CreateFromFile(ID3D11Device* d3dDevice, const wchar_t* fileName)
{
  HRESULT hr = DirectX::CreateDDSTextureFromFile(
    d3dDevice, fileName, nullptr, texture.ReleaseAndGetAddressOf());
  if (FAILED(hr))
  {
    LOG_ERROR("Couldn't load texture from file: %s", fileName);
    throw std::exception("Texture");
  }

  if (texture)
  {
    Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    texture->GetResource(resource.GetAddressOf());

    D3D11_RESOURCE_DIMENSION dim;
    resource->GetType(&dim);

    if (dim != D3D11_RESOURCE_DIMENSION_TEXTURE2D)
    {
      LOG_ERROR("Expected a Texture2D");
      throw std::exception("Texture");
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2D;
    resource.As(&tex2D);

    D3D11_TEXTURE2D_DESC desc;
    tex2D->GetDesc(&desc);

    width  = desc.Width;
    height = desc.Height;
    origin = {width / 2.0f, height / 2.0f, 0.0f};
  }
}

//------------------------------------------------------------------------------
