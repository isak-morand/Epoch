#pragma once
#include <memory>
#include <CommonUtilities/Color.h>
#include <CommonUtilities/Math/Transform.h>

namespace Epoch
{
	class Window;

	namespace Assets
	{
		class TextureAsset;
		class MeshAsset;
	}
}

namespace Epoch::Rendering
{
	class IRenderer
	{
	public:
		IRenderer();
		virtual ~IRenderer();

		static std::unique_ptr<IRenderer> Create();

		virtual bool Initialize(Window* aWindow) = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void OnResize(uint32_t aWidth, uint32_t aHeight) = 0;

		virtual void SetClearColor(const CU::Color& aColor) = 0;

		//TEMP
		virtual void SetTexture(std::shared_ptr<Assets::TextureAsset> aTexture) = 0;
		virtual void SubmitMesh(std::shared_ptr<Assets::MeshAsset> aMesh, const CU::Matrix4x4f& aTransform) = 0;
	};
}
