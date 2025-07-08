#pragma once
#include "EpochRendering/IRenderer.h"
#include "DeviceManager.h"
#include "SwapChain.h"

//TEMP
#include "Shaders/Shader.h"
#include "PipelineState.h"

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
	class Mesh;
	class Texture2D;
	class ConstantBuffer;
	class VertexBuffer;
	class IndexBuffer;

	class Renderer : public IRenderer
	{
	public:
		Renderer();
		virtual ~Renderer();

		bool Initialize(Window* aWindow) override;
		void BeginFrame() override;
		void EndFrame() override;

		void OnResize(uint32_t aWidth, uint32_t aHeight) override;

		void SetClearColor(const CU::Color& aColor) override;

		//TEMP
		void SetTexture(std::shared_ptr<Assets::TextureAsset> aTexture) override;
		void SetMesh(std::shared_ptr<Assets::MeshAsset> aMesh) override;

	private:
		std::unique_ptr<DeviceManager> myDeviceManager;
		std::unique_ptr<SwapChain> mySwapChain;

		struct RendererResources
		{
			std::shared_ptr<Texture2D> WhiteTexture;
			std::shared_ptr<Texture2D> BlackTexture;
			std::shared_ptr<Texture2D> FlatNormalTexture;
		} myRendererResources;

		//TEMP
		CU::Color myClearColor = CU::Color::Black;

		nvrhi::CommandListHandle myCommandList;

		std::shared_ptr<Shader> myTestShader;
		std::shared_ptr<PipelineState> myTestPipelineState;

		std::shared_ptr<Mesh> myTestMesh;
		std::shared_ptr<ConstantBuffer> myTestCamBuffer;
		std::shared_ptr<Texture2D> myTestTexture;
	};
}
