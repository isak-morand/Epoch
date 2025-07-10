#include "epch.h"
#include "Renderer.h"
#include "DeviceManager.h"
#include "SwapChain.h"
#include "RenderContext.h"
#include "Resources/Texture.h"
#include "Resources/Mesh.h"
#include "Resources/VertexBuffer.h"
#include "Resources/IndexBuffer.h"
#include "Resources/ConstantBuffer.h"

#include <EpochCore/Window/Window.h>

//TEMP
#include <nvrhi/utils.h>
#include "Shaders/ShaderCompiler.h"
#include "Framebuffer.h"
#include <EpochCore/Input/Input.h>
#include <EpochDataTypes/Vertex.h>
#include <CommonUtilities/Math/Transform.h>
#include <CommonUtilities/Timer.h>
#include <EpochAssets/Assets/TextureAsset.h>
#include <EpochAssets/Assets/MeshAsset.h>

namespace Epoch::Rendering
{
	Renderer::Renderer() = default;
	Renderer::~Renderer() = default;

	bool Renderer::Initialize(Window* aWindow)
	{
		myDeviceManager = Rendering::DeviceManager::Create();
		if (!myDeviceManager->Initialize())
		{
			EPOCH_ASSERT(false, "Failed to initialize the device manager!");
			return false;
		}
		RenderContext::Get().DeviceManager = myDeviceManager.get();

		mySwapChain = Rendering::SwapChain::Create();
		if (!mySwapChain->Initialize(aWindow->GetNativeWindow(), aWindow->GetWidth(), aWindow->GetHeight()))
		{
			EPOCH_ASSERT(false, "Failed to initialize the swap chain!");
			return false;
		}
		RenderContext::Get().SwapChain = mySwapChain.get();

		//Render resources
		{
			TextureSpecification spec;
			spec.ImageSpec.Format = ImageFormat::RGBA;
			spec.ImageSpec.Width = 1;
			spec.ImageSpec.Height = 1;

			constexpr uint32_t whiteTextureData = 0xffffffff;
			spec.ImageSpec.DebugName = "White Texture";
			spec.ImageSpec.InitialData = Core::Buffer::Copy(Core::Buffer(&whiteTextureData, sizeof(uint32_t)));
			myRendererResources.WhiteTexture = std::make_shared<Texture2D>(spec);

			constexpr uint32_t blackTextureData = 0xff000000;
			spec.ImageSpec.DebugName = "Black Texture";
			spec.ImageSpec.InitialData = Core::Buffer::Copy(Core::Buffer(&blackTextureData, sizeof(uint32_t)));
			myRendererResources.BlackTexture = std::make_shared<Texture2D>(spec);

			constexpr uint32_t flatNormalTextureData = 0xffff7f7f;
			spec.ImageSpec.DebugName = "Flat Normal Texture";
			spec.ImageSpec.InitialData = Core::Buffer::Copy(Core::Buffer(&flatNormalTextureData, sizeof(uint32_t)));
			myRendererResources.FlatNormalTexture = std::make_shared<Texture2D>(spec);
		}

		//TEMP
		myCommandList = RenderContext::Get().DeviceManager->GetDeviceHandle()->createCommandList();

		auto sc = ShaderCompiler::Create();
		//sc->Compile("Resources/Shaders/Standard.shader");
		sc->Compile("Resources/Shaders/Test.shader");

		myTestShader = std::make_shared<Shader>("Test", sc->GetBinaries());


		FramebufferSpecification fbSpec;
		fbSpec.SwapChainTarget = true;

		VertexBufferLayout vertexLayout =
		{
			{ ShaderDataType::Float3, "POSITION" },
			{ ShaderDataType::Float3, "NORMAL" },
			{ ShaderDataType::Float3, "TANGENT" },
			{ ShaderDataType::Float2, "UV" },
			{ ShaderDataType::Float3, "COLOR" }
		};

		PipelineStateSpecification psSpec;
		psSpec.Shader = myTestShader;
		psSpec.DebugName = "TestPipelineState";
		psSpec.VertexLayouts.push_back(vertexLayout);
		psSpec.TargetFramebuffer = std::make_shared<Framebuffer>(fbSpec);
		myTestPipelineState = std::make_shared<PipelineState>(psSpec);


		ConstantBufferSpecification cbs;
		cbs.SizeInBytes = sizeof(CU::Matrix4x4f);
		myTestCamBuffer = std::make_shared<ConstantBuffer>(cbs);


		return true;
	}

	void Renderer::BeginFrame()
	{
		EPOCH_PROFILE_FUNC();

		mySwapChain->BeginFrame();

		//TEMP: Camera
		{
			static CU::Timer timer;

			CU::Matrix4x4f proj = CU::Matrix4x4f::CreatePerspectiveProjection(80 * CU::Math::ToRad, 1, 25000, (float)mySwapChain->GetWidth() / (float)mySwapChain->GetHeight());
			static CU::Transform camTrans;

			float deltaTime = timer.Elapsed();
			timer.Reset();

			if (Input::IsMouseButtonHeld(MouseButton::Right))
			{
				float x = 0.0f; float y = 0.0f; float z = 0.0f;

				if (Input::IsKeyHeld(KeyCode::W)) z += 1.0f;
				if (Input::IsKeyHeld(KeyCode::S)) z -= 1.0f;

				if (Input::IsKeyHeld(KeyCode::D)) x += 1.0f;
				if (Input::IsKeyHeld(KeyCode::A)) x -= 1.0f;

				if (Input::IsKeyHeld(KeyCode::E)) y += 1.0f;
				if (Input::IsKeyHeld(KeyCode::Q)) y -= 1.0f;

				CU::Vector3f dir = camTrans.GetRight() * x + camTrans.GetForward() * z + camTrans.GetUp() * y ;
				dir.Normalize();

				camTrans.Translate(dir * 400.0f * deltaTime);

				auto mouseDelta = -Input::GetMouseDelta() * 0.002f;
				camTrans.Rotate(CU::Vector3f(mouseDelta.y, mouseDelta.x, 0));
			}

			CU::Matrix4x4f viewProj = camTrans.GetMatrix().GetFastInverse() * proj;

			myTestCamBuffer->SetData({ (void*)&viewProj, sizeof(CU::Matrix4x4f) });
		}

		//TEMP
		auto layoutDesc = nvrhi::BindingLayoutDesc()
			.setVisibility(nvrhi::ShaderType::Vertex | nvrhi::ShaderType::Pixel)
			.addItem(nvrhi::BindingLayoutItem::VolatileConstantBuffer(0)) // constants at b0
			.addItem(nvrhi::BindingLayoutItem::Texture_SRV(0))
			.addItem(nvrhi::BindingLayoutItem::Sampler(0));

		nvrhi::BindingLayoutHandle bindingLayout = RenderContext::Get().DeviceManager->GetDeviceHandle()->createBindingLayout(layoutDesc);

		auto bindingSetDesc = nvrhi::BindingSetDesc()
			.addItem(nvrhi::BindingSetItem::ConstantBuffer(0, myTestCamBuffer->GetHandle()))
			.addItem(nvrhi::BindingSetItem::Texture_SRV(0, myTestTexture->GetImage()->GetHandle()))
			.addItem(nvrhi::BindingSetItem::Sampler(0, myTestTexture->GetSampler()->GetHandle()));

		nvrhi::BindingSetHandle bindingSet = RenderContext::Get().DeviceManager->GetDeviceHandle()->createBindingSet(bindingSetDesc, bindingLayout);


		myCommandList->open();

		nvrhi::utils::ClearColorAttachment(myCommandList, mySwapChain->GetFrameBuffer(), 0, nvrhi::Color(myClearColor.r, myClearColor.g, myClearColor.b, myClearColor.a));
		nvrhi::utils::ClearDepthStencilAttachment(myCommandList, mySwapChain->GetFrameBuffer(), 1.0f, 0);

		myCommandList->beginMarker("Render");

		auto graphicsState = nvrhi::GraphicsState();
		graphicsState.setPipeline(myTestPipelineState->GetHandle());
		graphicsState.addBindingSet(bindingSet);
		graphicsState.setFramebuffer(myTestPipelineState->GetTargetFrameBuffer()->GetHandle());
		graphicsState.viewport.addViewportAndScissorRect(myTestPipelineState->GetTargetFrameBuffer()->GetHandle()->getFramebufferInfo().getViewport());
		
		nvrhi::VertexBufferBinding vbb = nvrhi::VertexBufferBinding()
			.setBuffer(myTestMesh->GetVertexBuffer()->GetHandle())
			.setSlot(0)
			.setOffset(0);
		graphicsState.addVertexBuffer(vbb);

		nvrhi::IndexBufferBinding ibb = nvrhi::IndexBufferBinding()
			.setBuffer(myTestMesh->GetIndexBuffer()->GetHandle())
			.setFormat(nvrhi::Format::R32_UINT)
			.setOffset(0);
		graphicsState.setIndexBuffer(ibb);
		
		myCommandList->setGraphicsState(graphicsState);

		const auto& subMeshses = myTestMesh->GetSubMeshes();
		for (const auto& subMesh : subMeshses)
		{
			auto drawArguments = nvrhi::DrawArguments()
				.setVertexCount(subMesh.IndexCount)
				.setStartIndexLocation(subMesh.IndexOffset);
			myCommandList->drawIndexed(drawArguments);
		}

		myCommandList->endMarker();

		myCommandList->close();

		RenderContext::Get().DeviceManager->GetDeviceHandle()->executeCommandList(myCommandList);
	}

	void Renderer::EndFrame()
	{
		EPOCH_PROFILE_FUNC();

		mySwapChain->EndFrame();

		myDeviceManager->GetDeviceHandle()->runGarbageCollection();
	}

	void Renderer::OnResize(uint32_t aWidth, uint32_t aHeight)
	{
		mySwapChain->Resize(aWidth, aHeight);

		//TEMP: Should be deferred
		myTestPipelineState->GetTargetFrameBuffer()->Resize(aWidth, aHeight);
	}

	void Renderer::SetClearColor(const CU::Color& aColor)
	{
		myClearColor = aColor;
	}

	void Renderer::SetTexture(std::shared_ptr<Assets::TextureAsset> aTexture)
	{
		const auto& textureData = aTexture->GetData();

		TextureSpecification spec;

		spec.ImageSpec.Format = textureData.Format;
		spec.ImageSpec.Width = textureData.Width;
		spec.ImageSpec.Height = textureData.Height;
		spec.ImageSpec.GenerateMipmaps = textureData.GenerateMips;

		spec.SamplerSpec.SetFilterMode(textureData.FilterMode);
		spec.SamplerSpec.SetWrapMode(textureData.WrapMode);

		spec.ImageSpec.InitialData = Core::Buffer::Copy(textureData.Data);
		myTestTexture = std::make_shared<Texture2D>(spec);
	}
	
	void Renderer::SetMesh(std::shared_ptr<Assets::MeshAsset> aMesh)
	{
		const auto& data = aMesh->GetData();

		myTestMesh = std::make_shared<Mesh>("Test Mesh", data.Vertices, data.Indices, data.SubMeshes);
	}
}
