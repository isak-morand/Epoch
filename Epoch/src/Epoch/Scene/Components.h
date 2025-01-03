#pragma once
#include <string>
#include <vector>
#include <CommonUtilities/Math/Transform.h>
#include <CommonUtilities/Gradient.h>
#include "Epoch/Core/UUID.h"
#include "Epoch/Assets/Asset.h"
#include "Epoch/Scene/SceneCamera.h"
#include "Epoch/Rendering/Material.h"
#include "Epoch/Rendering/ParticleSystemModules.h"
#include "Epoch/Rendering/PostProcessingVolumeOverrides.h"
#include "Epoch/Physics/PhysicsTypes.h"
#include "Epoch/Script/GCManager.h"

namespace Epoch
{
#pragma region Base Components

	struct IDComponent
	{
		UUID id;

		IDComponent() = default;
		IDComponent(UUID aID) : id(aID) {}
		IDComponent(const IDComponent&) = default;
	};

	struct ActiveComponent
	{
		bool isActive = true;

		ActiveComponent() = default;
		ActiveComponent(const ActiveComponent&) = default;
	};

	struct NameComponent
	{
		std::string name;

		NameComponent() = default;
		NameComponent(const std::string& aName) : name(aName) {}
		NameComponent(const NameComponent&) = default;
	};

	struct RelationshipComponent
	{
		UUID parentHandle = 0;
		std::vector<UUID> children;

		RelationshipComponent() = default;
		RelationshipComponent(UUID aParent) : parentHandle(aParent) {}
		RelationshipComponent(const RelationshipComponent&) = default;
	};

	struct PrefabComponent
	{
		UUID prefabID = 0;
		UUID entityID = 0;
	};

	struct TransformComponent
	{
		CU::Transform transform;

		TransformComponent() = default;
		TransformComponent(const CU::Vector3f& aPosition, const CU::Vector3f& aRotation = CU::Vector3f::Zero, const CU::Vector3f& aScale = CU::Vector3f::One) : transform(CU::Transform(aPosition, aRotation, aScale)) {}
		TransformComponent(const TransformComponent&) = default;

		const CU::Matrix4x4f& GetMatrix() { return transform.GetMatrix(); }
	};

#pragma endregion

#pragma region Graphics Components

	struct CameraComponent
	{
		bool isActive = true;

		SceneCamera camera;
		bool primary = false; // TODO: Think about moving to Scene

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct SkyLightComponent
	{
		bool isActive = true;

		AssetHandle environment = 0;
		float intensity = 1.0f;
		bool physicallyBasedSky = false;

		SkyLightComponent() = default;
		SkyLightComponent(const SkyLightComponent&) = default;
	};

	struct DirectionalLightComponent
	{
		bool isActive = true;

		CU::Color color = CU::Color::White;
		float intensity = 1.0f;
		bool castsShadows = false;

		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
	};

	struct SpotlightComponent
	{
		bool isActive = true;

		CU::Color color = CU::Color::White;
		float intensity = 1.0f;
		bool castsShadows = false;
		float range = 1000.0f;
		float outerSpotAngle = 15.0f * CU::Math::ToRad;
		float innerSpotAngle = 12.0f * CU::Math::ToRad;
		AssetHandle cookieTexture = 0;

		SpotlightComponent() = default;
		SpotlightComponent(const SpotlightComponent&) = default;
	};

	struct PointLightComponent
	{
		bool isActive = true;

		CU::Color color = CU::Color::White;
		float intensity = 1.0f;
		bool castsShadows = false;
		float range = 250.0f;
		AssetHandle cookieTexture = 0;

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;
	};

	struct MeshRendererComponent
	{
		bool isActive = true;

		AssetHandle mesh = 0;
		std::shared_ptr<MaterialTable> materialTable = std::make_shared<MaterialTable>();
		bool castsShadows = true;

		MeshRendererComponent() = default;
		MeshRendererComponent(AssetHandle aMesh) : mesh(aMesh) {}
		MeshRendererComponent(const MeshRendererComponent& aOther) :
			isActive(aOther.isActive), mesh(aOther.mesh), castsShadows(aOther.castsShadows),
			materialTable(std::make_shared<MaterialTable>(aOther.materialTable)) { }
	};

	struct SkinnedMeshRendererComponent
	{
		bool isActive = true;

		AssetHandle mesh = 0;
		bool castsShadows = true;
		std::vector<UUID> boneEntityIds;

		SkinnedMeshRendererComponent() = default;
		SkinnedMeshRendererComponent(AssetHandle aMesh) : mesh(aMesh) {}
		SkinnedMeshRendererComponent(const SkinnedMeshRendererComponent&) = default;
	};

	struct SpriteRendererComponent
	{
		bool isActive = true;

		AssetHandle texture = 0;
		CU::Color tint = CU::Color::White;
		bool flipX = false;
		bool flipY = false;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(AssetHandle aTexture) : texture(aTexture) {}
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
	};
	
	struct VideoPlayerComponent
	{
		bool isActive = true;

		AssetHandle video = 0;
		bool playOnAwake = true;
		bool loop = false;
		float playbackSpeed = 1.0f;

		VideoPlayerComponent() = default;
		VideoPlayerComponent(AssetHandle aVideo) : video(aVideo) {}
		VideoPlayerComponent(const VideoPlayerComponent&) = default;
	};

	enum class Alignment
	{
		UpperLeft,
		UpperCenter,
		UpperRight,
		MiddleLeft,
		MiddleCenter,
		MiddleRight,
		LowerLeft,
		LowerCenter,
		LowerRight
	};

	struct TextRendererComponent
	{
		bool isActive = true;

		std::string text = "Text";
		AssetHandle font = 0;
		CU::Color color = CU::Color::White;

		//Alignment alignment = Alignment::MiddleCenter;
		bool centered = false;
		float letterSpacing = 0.0f; // Kerning
		float lineSpacing = 0.0f;
		float maxWidth = 10.0f;

		TextRendererComponent() = default;
		TextRendererComponent(const TextRendererComponent&) = default;
	};

	struct ParticleSystemComponent
	{
		bool isActive = true;

		float duration = 1.0f;
		bool looping = true;
		bool prewarm = false;

		float startLifetime = 1.0f;
		float startSpeed = 1.0f;
		float startSize = 1.0f;
		CU::Color startColor = CU::Color::White;

		float gravityMultiplier = 1.0f;
		bool playOnAwake = true;

		ParticleSystem::Emission emission;
		ParticleSystem::Shape shape;

		ParticleSystem::VelocityOverLifetime velocityOverLifetime;
		ParticleSystem::ColorOverLifetime colorOverLifetime;
		ParticleSystem::SizeOverLifetime sizeOverLifetime;

		ParticleSystemComponent() = default;
		ParticleSystemComponent(const ParticleSystemComponent&) = default;
	};

	struct VolumeComponent
	{
		bool isActive = true;

		PostProcessing::Tonemapping tonemapping;
		PostProcessing::ColorGrading colorGrading;
		PostProcessing::Vignette vignette;
		PostProcessing::DistanceFog distanceFog;
		PostProcessing::Posterization posterization;

		VolumeComponent() = default;
		VolumeComponent(const VolumeComponent&) = default;
	};

	struct TerrainComponent
	{
		AssetHandle heightmap = 0;

		TerrainComponent() = default;
		TerrainComponent(const TerrainComponent&) = default;
	};

#pragma endregion

#pragma region Physics Components
	
	struct RigidbodyComponent
	{
		float mass = 1.0f;
		float linearDrag = 0.01f;
		float angularDrag = 0.05f;
		bool useGravity = true;
		bool isKinematic = false;

		Physics::Axis constraints = Physics::Axis::None;

		CU::Vector3f initialLinearVelocity;
		CU::Vector3f initialAngularVelocity;

		RigidbodyComponent() = default;
		RigidbodyComponent(const RigidbodyComponent&) = default;
	};

	struct BoxColliderComponent
	{
		CU::Vector3f halfSize = CU::Vector3f::One * 50.0f;
		CU::Vector3f offset;

		bool isTrigger = false;

		BoxColliderComponent() = default;
		BoxColliderComponent(const BoxColliderComponent&) = default;
	};

	struct SphereColliderComponent
	{
		float radius = 50.0f;
		CU::Vector3f offset;

		bool isTrigger = false;

		SphereColliderComponent() = default;
		SphereColliderComponent(const SphereColliderComponent&) = default;
	};
	
	struct CapsuleColliderComponent
	{
		float radius = 50.0f;
		float height = 200.0f;
		CU::Vector3f offset;

		bool isTrigger = false;

		CapsuleColliderComponent() = default;
		CapsuleColliderComponent(const CapsuleColliderComponent&) = default;
	};

	struct CharacterControllerComponent
	{
		float slopeLimit = 45.0f;
		float stepOffset = 30.0f;
		float radius = 50.0f;
		float height = 200.0f;
		CU::Vector3f offset;

		CharacterControllerComponent() = default;
		CharacterControllerComponent(const CharacterControllerComponent&) = default;
	};

#pragma endregion
	
#pragma region Script Component
	
	struct ScriptComponent
	{
		bool isActive = true;

		AssetHandle scriptClassHandle = 0;
		GCHandle managedInstance = nullptr;
		
		std::vector<uint32_t> fieldIDs;

		bool isRuntimeInitialized = false;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
		ScriptComponent(AssetHandle aScriptClassHandle) : scriptClassHandle(aScriptClassHandle) {}
	};

#pragma endregion

	template<typename... Component>
	struct ComponentGroup {};

	using AllComponents = ComponentGroup<
		TransformComponent, ActiveComponent, RelationshipComponent, PrefabComponent, ScriptComponent,
		CameraComponent, SkyLightComponent, DirectionalLightComponent, SpotlightComponent, PointLightComponent,
		MeshRendererComponent, SkinnedMeshRendererComponent, SpriteRendererComponent, VideoPlayerComponent, TextRendererComponent, TerrainComponent,
		RigidbodyComponent, BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent,
		CharacterControllerComponent, ParticleSystemComponent, VolumeComponent>;

	// Can´t include relationship component as the children needs to be copied as well in duplicate
	using AllComponentsDuplicate = ComponentGroup<
		TransformComponent, ActiveComponent, PrefabComponent, ScriptComponent,
		CameraComponent, SkyLightComponent, DirectionalLightComponent, SpotlightComponent, PointLightComponent,
		MeshRendererComponent, SkinnedMeshRendererComponent, SpriteRendererComponent, VideoPlayerComponent, TextRendererComponent, TerrainComponent,
		RigidbodyComponent, BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent,
		CharacterControllerComponent, ParticleSystemComponent, VolumeComponent>;
}
