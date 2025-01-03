#include "SceneHierarchyPanel.h"
#include <CommonUtilities/StringUtils.h>
#include <Epoch/Debug/Log.h>
#include <Epoch/Debug/Instrumentor.h>
#include <Epoch/Scene/Components.h>
#include <Epoch/Scene/Prefab.h>
#include <Epoch/Physics/PhysicsTypes.h>
#include <Epoch/Rendering/Mesh.h>
#include <Epoch/Rendering/Font.h>
#include <Epoch/Rendering/Environment.h>
#include <Epoch/Script/ScriptEngine.h>
#include <Epoch/Script/ScriptAsset.h>
#include <Epoch/Editor/PanelIDs.h>

namespace Epoch
{
#define ToMeters(x) x * 0.01f
#define ToCentimeters(x) x * 100.0f

	static bool staticPopupHovered = false;
	static bool staticOpenPopup = false;

	static int staticRowIndex = -1; //TODO: Remove

	SceneHierarchyPanel::SceneHierarchyPanel()
	{
		myComponentCopyScene = std::make_shared<Scene>();
		myComponentCopyEntity = myComponentCopyScene->CreateEntity();
	}

	void Epoch::SceneHierarchyPanel::OnImGuiRender(bool& aIsOpen)
	{
		EPOCH_PROFILE_FUNC();

		ImGui::Begin(SCENE_HIERARCHY_PANEL_ID);
		ImRect windowRect = { ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax() };

		if (myContext)
		{
			static std::string searchString;

			ImGui::SetNextItemWidth(windowRect.GetWidth());
			ImGui::InputTextWithHint("##EntitySearch", "Search...", &searchString);
			UI::Spacing();
			ImGui::Separator();

			const size_t entityCount = myContext->GetAllEntitiesWith<IDComponent>().size();
			ImGui::TextDisabled(("Entity Count: " + std::to_string(entityCount)).c_str());

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
			ImGui::BeginChild("Entity List");

			//ImRect listRect = UI::GetItemRect();
			//listRect.Max.y += ImGui::GetContentRegionAvail().y;

			//ImGui::PushClipRect(listRect.Min, listRect.Max, false);

			{
				EPOCH_PROFILE_SCOPE("SceneHierarchyPanel::OnImGuiRender::EntityList");

				staticRowIndex = -1;
				for (auto entityID : myContext->GetAllEntitiesWith<IDComponent, RelationshipComponent>())
				{
					Entity entity{ entityID, myContext.get() };
					if (entity.GetParentUUID() == 0)
					{
						DrawEntityNode(entity, searchString);
					}
				}
			}

			//ImGui::PopClipRect();

			if (ImGui::IsWindowHovered())
			{
				if (!ImGui::IsAnyItemHovered())
				{
					if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
					{
						SelectionManager::DeselectAll(SelectionContext::Entity);
					}
					else if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
					{
						SelectionManager::DeselectAll(SelectionContext::Entity);
						staticOpenPopup = true;
					}
				}
			}

			if (staticOpenPopup)
			{
				staticOpenPopup = false;
				ImGui::OpenPopup("Hierarchy Popup");
			}

			HierarchyPopup();

			ImGui::EndChild();
			ImGui::PopStyleColor();


			if (ImGui::BeginDragDropTargetCustom(windowRect, ImGui::GetCurrentWindow()->ID))
			{
				{
					const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_entity_hierarchy", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

					if (payload)
					{
						size_t count = payload->DataSize / sizeof(UUID);

						for (size_t i = 0; i < count; i++)
						{
							UUID entityID = *(((UUID*)payload->Data) + i);
							Entity entity = myContext->GetEntityWithUUID(entityID);
							myContext->UnparentEntity(entity);
						}
					}
				}

				HandleAssetDrop(Entity());

				ImGui::EndDragDropTarget();
			}
		}

		ImGui::End();

		
		ImGui::Begin(std::format("{}  Properties", EP_ICON_SLIDERS).c_str());

		DrawComponents(SelectionManager::GetSelections(SelectionContext::Entity));

		ImGui::End();
	}

	void SceneHierarchyPanel::OnSceneChanged(const std::shared_ptr<Scene>& aScene)
	{
		myContext = aScene;
	}

	bool SceneHierarchyPanel::ChildMatchesSearchRecursively(Entity aEntity, const std::string& aSearchFilter)
	{
		if (aSearchFilter.empty())
		{
			return false;
		}

		for (auto childId : aEntity.Children())
		{
			Entity child = myContext->GetEntityWithUUID(childId);
			if (UI::IsMatchingSearch(child.GetName(), aSearchFilter))
			{
				return true;
			}

			bool found = ChildMatchesSearchRecursively(child, aSearchFilter);
			if (found)
			{
				return true;
			}
		}

		return false;
	}

	bool SceneHierarchyPanel::IsAnyDescendantSelected(Entity aEntity, bool aFirst)
	{
		if (!aFirst && SelectionManager::IsSelected(SelectionContext::Entity, aEntity.GetUUID()))
		{
			return true;
		}

		if (!aEntity.Children().empty())
		{
			for (auto& childEntityID : aEntity.Children())
			{
				Entity childEntity = myContext->GetEntityWithUUID(childEntityID);
				if (IsAnyDescendantSelected(childEntity))
				{
					return true;
				}
			}
		}

		return false;
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity aEntity, const std::string& aSearchFilter)
	{
		staticRowIndex++;

		std::string& entityName = aEntity.GetComponent<NameComponent>().name;

		const bool childMatchesSearch = ChildMatchesSearchRecursively(aEntity, aSearchFilter);
		if (!childMatchesSearch && !UI::IsMatchingSearch(entityName, aSearchFilter)) return;

		bool isSelected = SelectionManager::IsSelected(SelectionContext::Entity, aEntity.GetUUID());

		ImGuiTreeNodeFlags flags = ((isSelected) ? ImGuiTreeNodeFlags_Selected : 0)/* | ImGuiTreeNodeFlags_SpanAvailWidth*/;
		aEntity.Children().empty() ? flags |= ImGuiTreeNodeFlags_Leaf : flags |= ImGuiTreeNodeFlags_OpenOnArrow;

		const bool anyDescendantSelected = IsAnyDescendantSelected(aEntity, true);

		if ((anyDescendantSelected || (!aSearchFilter.empty() && childMatchesSearch)) && (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::IsMouseReleased(ImGuiMouseButton_Left)))
		{
			ImGui::SetNextItemOpen(true);
		}

		bool isPrefab = aEntity.HasComponent<PrefabComponent>();
		bool isValid = true;
		{
			if (isPrefab && !AssetManager::IsAssetHandleValid(aEntity.GetComponent<PrefabComponent>().prefabID))
			{
				isValid = false;
			}

			if (aEntity.HasComponent<SpriteRendererComponent>())
			{
				auto& src = aEntity.GetComponent<SpriteRendererComponent>();
				if (src.texture != 0 && !AssetManager::IsAssetHandleValid(src.texture))
				{
					isValid = false;
				}
			}

			if (aEntity.HasComponent<MeshRendererComponent>())
			{
				if (!AssetManager::IsAssetHandleValid(aEntity.GetComponent<MeshRendererComponent>().mesh))
				{
					isValid = false;
				}
			}

			if (aEntity.HasComponent<SkinnedMeshRendererComponent>())
			{
				auto& smrc = aEntity.GetComponent<SkinnedMeshRendererComponent>();
				if (!AssetManager::IsAssetHandleValid(smrc.mesh) || smrc.boneEntityIds.empty() || !myContext->TryGetEntityWithUUID(smrc.boneEntityIds[0]))
				{
					isValid = false;
				}
			}

			if (aEntity.HasComponent<TerrainComponent>())
			{
				if (!AssetManager::IsAssetHandleValid(aEntity.GetComponent<TerrainComponent>().heightmap))
				{
					isValid = false;
				}
			}

			if (aEntity.HasComponent<ScriptComponent>())
			{
				if (!ScriptEngine::IsModuleValid(aEntity.GetComponent<ScriptComponent>().scriptClassHandle))
				{
					isValid = false;
				}
			}
		}

		if (!isValid)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, aEntity.IsActive() ? Colors::Theme::invalid : Colors::Theme::disabledInvalid);
			//if (aEntity.IsActive())
			//{
			//	ImGui::PushStyleColor(ImGuiCol_Text, Colors::Theme::invalid);
			//}
			//else
			//{
			//	ImGui::PushStyleColor(ImGuiCol_Text, Colors::Theme::disabledInvalid);
			//}
		}
		else if (isPrefab)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, aEntity.IsActive() ? Colors::Theme::blue : Colors::Theme::disabledBlue);
			//if (aEntity.IsActive())
			//{
			//	ImGui::PushStyleColor(ImGuiCol_Text, Colors::Theme::blue);
			//}
			//else
			//{
			//	ImGui::PushStyleColor(ImGuiCol_Text, Colors::Theme::disabledBlue);
			//}
		}
		else if (!aEntity.IsActive())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::Theme::disabled);
		}

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)aEntity, flags, entityName.c_str());

		if (!isValid || isPrefab || !aEntity.IsActive())
		{
			ImGui::PopStyleColor();
		}

		if (staticRowIndex >= myFirstSelectedRow && staticRowIndex <= myLastSelectedRow && !SelectionManager::IsSelected(SelectionContext::Entity, aEntity.GetUUID()) && myShiftSelectionRunning)
		{
			SelectionManager::Select(SelectionContext::Entity, aEntity.GetUUID());

			if (SelectionManager::GetSelectionCount(SelectionContext::Entity) == (myLastSelectedRow - myFirstSelectedRow) + 1)
			{
				myShiftSelectionRunning = false;
			}
		}

		if (!staticPopupHovered && ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly))
		{
			bool leftUp = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
			bool rightUp = ImGui::IsMouseReleased(ImGuiMouseButton_Right);
			bool ctrlDown = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
			bool shiftDown = ImGui::IsKeyDown(ImGuiKey_LeftShift);
			if (leftUp || rightUp)
			{
				if (shiftDown && SelectionManager::GetSelectionCount(SelectionContext::Entity) > 0)
				{
					SelectionManager::DeselectAll(SelectionContext::Entity);

					if (staticRowIndex < myFirstSelectedRow)
					{
						myLastSelectedRow = myFirstSelectedRow;
						myFirstSelectedRow = staticRowIndex;
					}
					else
					{
						myLastSelectedRow = staticRowIndex;
					}

					myShiftSelectionRunning = true;
				}
				else if (!ctrlDown || shiftDown)
				{
					SelectionManager::DeselectAll(SelectionContext::Entity);
					SelectionManager::Select(SelectionContext::Entity, aEntity.GetUUID());
					myFirstSelectedRow = staticRowIndex;
					myLastSelectedRow = -1;
				}
				else
				{
					if (isSelected)
					{
						SelectionManager::Deselect(SelectionContext::Entity, aEntity.GetUUID());
					}
					else
					{
						SelectionManager::Select(SelectionContext::Entity, aEntity.GetUUID());
					}
				}

				if (rightUp)
				{
					staticOpenPopup = true;
				}
			}
		}

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			const auto& selectedEntities = SelectionManager::GetSelections(SelectionContext::Entity);
			UUID entityID = aEntity.GetUUID();

			if (!SelectionManager::IsSelected(SelectionContext::Entity, entityID))
			{
				ImGui::Text(aEntity.GetName().c_str());
				ImGui::SetDragDropPayload("scene_entity_hierarchy", &entityID, 1 * sizeof(UUID));
			}
			else
			{
				for (const auto& selectedEntity : selectedEntities)
				{
					Entity entity = myContext->GetEntityWithUUID(selectedEntity);
					ImGui::Text(entity.GetName().c_str());
				}

				ImGui::SetDragDropPayload("scene_entity_hierarchy", selectedEntities.data(), selectedEntities.size() * sizeof(UUID));
			}

			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_entity_hierarchy", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

			if (payload)
			{
				size_t count = payload->DataSize / sizeof(UUID);

				for (size_t i = 0; i < count; i++)
				{
					UUID droppedEntityID = *(((UUID*)payload->Data) + i);
					Entity droppedEntity = myContext->GetEntityWithUUID(droppedEntityID);
					myContext->ParentEntity(droppedEntity, aEntity);
				}
			}

			HandleAssetDrop(aEntity);

			ImGui::EndDragDropTarget();
		}

		if (opened)
		{
			for (auto child : aEntity.Children())
			{
				DrawEntityNode(myContext->GetEntityWithUUID(child), aSearchFilter);
			}

			ImGui::TreePop();
		}
	}

	static bool DrawVec3Control(const std::string& aLabel, CU::Vector3f& aValues, float aSpeed, bool& aManuallyEdited, float aResetValue = 0.0f, uint32_t aRenderMultiSelectAxes = 0)
	{
		bool modified = false;

		ImGui::PushID(aLabel.c_str());

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		auto drawControl = [&](
			const std::string& label, float& value,
			const ImVec4& colorN,
			const ImVec4& colorH,
			const ImVec4& colorA,
			bool renderMultiSelect)
			{
				{
					UI::ScopedColor buttonCol(ImGuiCol_Button, colorN);
					UI::ScopedColor hoveredButtonCol(ImGuiCol_ButtonHovered, colorH);
					UI::ScopedColor activeButtonCol(ImGuiCol_ButtonActive, colorA);
					UI::ScopedFont boldFont("Bold");
					UI::ShiftCursorY(-4.0f);
					if (ImGui::Button(label.c_str(), buttonSize))
					{
						value = aResetValue;
						modified = true;
					}
				}

				ImGui::SameLine();
				UI::ShiftCursorY(-4.0f);
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, renderMultiSelect);
				bool wasTempInputActive = ImGui::TempInputIsActive(ImGui::GetID(("##" + label).c_str()));
				modified |= UI::DragFloat(("##" + label).c_str(), &value, aSpeed, 0.0f, 0.0f, "%.3f");

				if (modified && ImGui::IsKeyDown(ImGuiKey_Tab))
				{
					aManuallyEdited = true;
				}

				if (ImGui::TempInputIsActive(ImGui::GetID(("##" + label).c_str())))
				{
					modified = false;
				}

				ImGui::PopItemWidth();
				ImGui::PopItemFlag();

				if (wasTempInputActive)
				{
					aManuallyEdited |= ImGui::IsItemDeactivatedAfterEdit();
				}
			};

		//UI::ShiftCursor(10.0f, 9.0f);
		UI::ShiftCursor(10.0f, 3.0f);
		ImGui::Text(aLabel.c_str());
		ImGui::SameLine(125.0f);

		ImGui::PushMultiItemsWidths(3, ImGui::GetContentRegionAvail().x - 85.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		drawControl("X", aValues.x, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f }, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f }, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f }, aRenderMultiSelectAxes & (uint32_t)VectorAxis::X);
		ImGui::SameLine();
		drawControl("Y", aValues.y, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f }, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f }, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f }, aRenderMultiSelectAxes & (uint32_t)VectorAxis::Y);
		ImGui::SameLine();
		drawControl("Z", aValues.z, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f }, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f }, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f }, aRenderMultiSelectAxes & (uint32_t)VectorAxis::Z);

		ImGui::PopStyleVar();

		UI::ShiftCursorY(4.0f);

		ImGui::PopID();

		return modified || aManuallyEdited;
	}

	void SceneHierarchyPanel::DrawComponents(const std::vector<UUID>& aEntityIDs)
	{
		if (aEntityIDs.size() == 0) return;

		EPOCH_PROFILE_FUNC();

		const bool isMultiSelect = aEntityIDs.size() > 1;
		Entity firstEntity = myContext->GetEntityWithUUID(aEntityIDs[0]);
		
		// Name Field
		{
			const bool inconsistentActiveState = IsInconsistentPrimitive<bool, ActiveComponent>([](const ActiveComponent& aActiveComponent) { return aActiveComponent.isActive; });
			bool state = (isMultiSelect && inconsistentActiveState) ? false : firstEntity.IsActive();
			if (isMultiSelect && inconsistentActiveState) ImGui::BeginDisabled();
			if (ImGui::Checkbox("##ActiveState", &state))
			{
				for (auto entityID : aEntityIDs)
				{
					Entity entity = myContext->GetEntityWithUUID(entityID);
					//entity.GetComponent<ActiveComponent>().isActive = state;
					entity.SetIsActive(state);
				}
			}
			if (isMultiSelect && inconsistentActiveState)  ImGui::EndDisabled();

			ImGui::SameLine();

			const bool inconsistentName = IsInconsistentPrimitive<std::string, NameComponent>([](const NameComponent& aNameComponent) { return aNameComponent.name; });
			std::string name = (isMultiSelect && inconsistentName) ? "---" : firstEntity.GetName();

			if (ImGui::InputText("##Name", &name, ImGuiInputTextFlags_AutoSelectAll))
			{
				if (!name.empty())
				{
					for (auto entityID : aEntityIDs)
					{
						Entity entity = myContext->GetEntityWithUUID(entityID);
						entity.GetComponent<NameComponent>().name = name;
					}
				}
			}

			//const bool inconsistentID = IsInconsistentPrimitive<std::string, IDComponent>([](const IDComponent& aIDComponent) { return std::to_string(aIDComponent.id); });
			//std::string uuid = "UUID: " + ((isMultiSelect && inconsistentID) ? "---" : std::to_string(firstEntity.GetUUID()));
			//ImGui::TextDisabled(uuid.c_str());
			//if (!inconsistentID && ImGui::IsItemClicked())
			//{
			//	ImGui::SetClipboardText(uuid.c_str());
			//}
		}

		//DONE - Multi Edit
		DrawComponent<TransformComponent>("Transform", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					UI::ScopedStyle itemSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
					UI::ScopedStyle framePadding(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));

					bool translationManuallyEdited = false;
					bool rotationManuallyEdited = false;
					bool scaleManuallyEdited = false;

					if (aIsMultiEdit)
					{
						uint32_t translationAxes = GetInconsistentVectorAxis<CU::Vector3f, TransformComponent>([](const TransformComponent& other) { return other.transform.GetTranslation(); });
						uint32_t rotationAxes = GetInconsistentVectorAxis<CU::Vector3f, TransformComponent>([](const TransformComponent& other) { return other.transform.GetRotation(); });
						uint32_t scaleAxes = GetInconsistentVectorAxis<CU::Vector3f, TransformComponent>([](const TransformComponent& other) { return other.transform.GetScale(); });

						CU::Vector3f translation = aFirstComponent.transform.GetTranslation() * 0.01f;
						CU::Vector3f rotation = aFirstComponent.transform.GetRotation() * CU::Math::ToDeg;
						CU::Vector3f scale = aFirstComponent.transform.GetScale();

						CU::Vector3f oldTranslation = translation;
						CU::Vector3f oldRotation = rotation;
						CU::Vector3f oldScale = scale;

						bool changed = false;

						changed |= DrawVec3Control("Translation", translation, 0.02f, translationManuallyEdited, 0.0f, translationAxes);
						changed |= DrawVec3Control("Rotation", rotation, 0.1f, rotationManuallyEdited, 0.0f, rotationAxes);
						changed |= DrawVec3Control("Scale", scale, 0.02f, scaleManuallyEdited, 1.0f, scaleAxes);

						if (changed)
						{
							if (translationManuallyEdited || rotationManuallyEdited || scaleManuallyEdited)
							{
								translationAxes = GetInconsistentVectorAxis(translation, oldTranslation);
								rotationAxes = GetInconsistentVectorAxis(rotation, oldRotation);
								scaleAxes = GetInconsistentVectorAxis(scale, oldScale);

								for (auto& entityID : aEntities)
								{
									Entity entity = myContext->GetEntityWithUUID(entityID);
									auto& component = entity.GetComponent<TransformComponent>();

									if ((translationAxes & (uint32_t)VectorAxis::X) != 0)
									{
										component.transform.GetTranslation().x = translation.x * 100.0f;
									}
									if ((translationAxes & (uint32_t)VectorAxis::Y) != 0)
									{
										component.transform.GetTranslation().y = translation.y * 100.0f;
									}
									if ((translationAxes & (uint32_t)VectorAxis::Z) != 0)
									{
										component.transform.GetTranslation().z = translation.z * 100.0f;
									}
									component.transform.SetTranslation(component.transform.GetTranslation());

									if ((rotationAxes & (uint32_t)VectorAxis::X) != 0)
									{
										component.transform.GetRotation().x = rotation.x * CU::Math::ToRad;
									}
									if ((rotationAxes & (uint32_t)VectorAxis::Y) != 0)
									{
										component.transform.GetRotation().y = rotation.y * CU::Math::ToRad;
									}
									if ((rotationAxes & (uint32_t)VectorAxis::Z) != 0)
									{
										component.transform.GetRotation().z = rotation.z * CU::Math::ToRad;
									}
									component.transform.SetRotation(component.transform.GetRotation());

									if ((scaleAxes & (uint32_t)VectorAxis::X) != 0)
									{
										component.transform.GetScale().x = scale.x;
									}
									if ((scaleAxes & (uint32_t)VectorAxis::Y) != 0)
									{
										component.transform.GetScale().y = scale.y;
									}
									if ((scaleAxes & (uint32_t)VectorAxis::Z) != 0)
									{
										component.transform.GetScale().z = scale.z;
									}
									component.transform.SetScale(component.transform.GetScale());
								}
							}
							else
							{
								CU::Vector3f translationDiff = translation - oldTranslation;
								CU::Vector3f rotationDiff = rotation - oldRotation;
								CU::Vector3f scaleDiff = scale - oldScale;

								for (auto& entityID : aEntities)
								{
									Entity entity = myContext->GetEntityWithUUID(entityID);
									auto& component = entity.GetComponent<TransformComponent>();

									component.transform.Translate(translationDiff * 100.0f);
									component.transform.Rotate(rotationDiff * CU::Math::ToRad);
									component.transform.SetScale(component.transform.GetScale() + scaleDiff);
								}
							}
						}
					}
					else
					{
						CU::Vector3f translation = aFirstComponent.transform.GetTranslation() * 0.01f;
						if (DrawVec3Control("Translation", translation, 0.02f, translationManuallyEdited))
						{
							aFirstComponent.transform.SetTranslation(translation * 100.0f);
						}

						CU::Vector3f rotation = aFirstComponent.transform.GetRotation() * CU::Math::ToDeg;
						if (DrawVec3Control("Rotation", rotation, 0.1f, rotationManuallyEdited))
						{
							aFirstComponent.transform.SetRotation(rotation * CU::Math::ToRad);
						}

						CU::Vector3f scale = aFirstComponent.transform.GetScale();
						if (DrawVec3Control("Scale", scale, 0.02f, scaleManuallyEdited, 1.0f))
						{
							aFirstComponent.transform.SetScale(scale);
						}
					}
				}
			}, EditorResources::TransformIcon);

		//DONE - Multi Edit
		DrawComponent<VolumeComponent>("Volume", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
		{
			{
				if (UI::SubHeaderWithCheckbox("Tonemapping", &aFirstComponent.tonemapping.enabled, false))
				{
					UI::BeginPropertyGrid();

					if (!aFirstComponent.tonemapping.enabled)
					{
						ImGui::BeginDisabled();
					}

					const char* optionsStrings[] = { "None", "Unreal", "Lottes", "ACES" };
					int currentOption = (int)aFirstComponent.tonemapping.tonemap;
					const bool inconsistentOption = IsInconsistentPrimitive<int, VolumeComponent>([](const VolumeComponent& aOther) { return (int)aOther.tonemapping.tonemap; });
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && inconsistentOption);
					if (UI::Property_Dropdown("Tonemap", optionsStrings, 4, currentOption, &inconsistentOption))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							entity.GetComponent<VolumeComponent>().tonemapping.tonemap = (PostProcessing::Tonemap)currentOption;
						}
					}
					ImGui::PopItemFlag();

					if (!aFirstComponent.tonemapping.enabled)
					{
						ImGui::EndDisabled();
					}

					UI::EndPropertyGrid();
				}

				if (UI::SubHeaderWithCheckbox("Color Grading", &aFirstComponent.colorGrading.enabled, false))
				{
					UI::BeginPropertyGrid();

					if (!aFirstComponent.colorGrading.enabled)
					{
						ImGui::BeginDisabled();
					}

					AssetHandle assetHandle = aFirstComponent.colorGrading.lut;
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<AssetHandle, VolumeComponent>([](const VolumeComponent& aOther) { return aOther.colorGrading.lut; }));
					if (UI::Property_AssetReference<Texture2D>("LUT", assetHandle))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& vc = entity.GetComponent<VolumeComponent>();
							vc.colorGrading.lut = assetHandle;
						}
					}
					ImGui::PopItemFlag();

					if (!aFirstComponent.colorGrading.enabled)
					{
						ImGui::EndDisabled();
					}

					UI::EndPropertyGrid();
				}

				if (UI::SubHeaderWithCheckbox("Vignette", &aFirstComponent.vignette.enabled, false))
				{
					UI::BeginPropertyGrid();

					if (!aFirstComponent.vignette.enabled)
					{
						ImGui::BeginDisabled();
					}

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<CU::Vector3f, VolumeComponent>([](const VolumeComponent& aOther) { return aOther.vignette.color; }));
					CU::Color color(aFirstComponent.vignette.color);
					if (UI::Property_ColorEdit3("Color", color))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& vc = entity.GetComponent<VolumeComponent>();
							vc.vignette.color = color.GetVector3();
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<CU::Vector2f, VolumeComponent>([](const VolumeComponent& aOther) { return aOther.vignette.center; }));
					if (UI::Property_DragFloat2("Center", aFirstComponent.vignette.center, 0.02f, 0.0f, 1.0f))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& vc = entity.GetComponent<VolumeComponent>();
							vc.vignette.center = aFirstComponent.vignette.center;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, VolumeComponent>([](const VolumeComponent& aOther) { return aOther.vignette.size; }));
					if (UI::Property_DragFloat("Size", aFirstComponent.vignette.size))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& vc = entity.GetComponent<VolumeComponent>();
							vc.vignette.size = aFirstComponent.vignette.size;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, VolumeComponent>([](const VolumeComponent& aOther) { return aOther.vignette.intensity; }));
					if (UI::Property_DragFloat("Intensity", aFirstComponent.vignette.intensity))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& vc = entity.GetComponent<VolumeComponent>();
							vc.vignette.intensity = aFirstComponent.vignette.intensity;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, VolumeComponent>([](const VolumeComponent& aOther) { return aOther.vignette.smoothness; }));
					if (UI::Property_DragFloat("Smoothness", aFirstComponent.vignette.smoothness))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& vc = entity.GetComponent<VolumeComponent>();
							vc.vignette.smoothness = aFirstComponent.vignette.smoothness;
						}
					}
					ImGui::PopItemFlag();

					if (!aFirstComponent.vignette.enabled)
					{
						ImGui::EndDisabled();
					}

					UI::EndPropertyGrid();
				}

				if (UI::SubHeaderWithCheckbox("Distance Fog", &aFirstComponent.distanceFog.enabled, false))
				{
					UI::BeginPropertyGrid();

					if (!aFirstComponent.distanceFog.enabled)
					{
						ImGui::BeginDisabled();
					}

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<CU::Vector3f, VolumeComponent>([](const VolumeComponent& aOther) { return aOther.distanceFog.color.GetVector3(); }));
					CU::Color color(aFirstComponent.distanceFog.color);
					if (UI::Property_ColorEdit3("Color", color))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& vc = entity.GetComponent<VolumeComponent>();
							vc.distanceFog.color = color.GetVector3();
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, VolumeComponent>([](const VolumeComponent& aOther) { return aOther.distanceFog.density; }));
					if (UI::Property_DragFloat("Density", aFirstComponent.distanceFog.density, 0.02f, 0.0f, FLT_MAX))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& vc = entity.GetComponent<VolumeComponent>();
							vc.distanceFog.density = aFirstComponent.distanceFog.density;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, VolumeComponent>([](const VolumeComponent& aOther) { return aOther.distanceFog.offset; }));
					if (UI::Property_DragFloat("Offset", aFirstComponent.distanceFog.offset))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& vc = entity.GetComponent<VolumeComponent>();
							vc.distanceFog.offset = aFirstComponent.distanceFog.offset;
						}
					}
					ImGui::PopItemFlag();

					if (!aFirstComponent.distanceFog.enabled)
					{
						ImGui::EndDisabled();
					}

					UI::EndPropertyGrid();
				}

				if (UI::SubHeaderWithCheckbox("Posterization", &aFirstComponent.posterization.enabled, false))
				{
					UI::BeginPropertyGrid();

					if (!aFirstComponent.posterization.enabled)
					{
						ImGui::BeginDisabled();
					}

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<uint32_t, VolumeComponent>([](const VolumeComponent& aOther) { return aOther.posterization.steps; }));
					if (UI::Property_DragUInt("Steps", aFirstComponent.posterization.steps))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& vc = entity.GetComponent<VolumeComponent>();
							vc.posterization.steps = aFirstComponent.posterization.steps;
						}
					}
					ImGui::PopItemFlag();

					if (!aFirstComponent.posterization.enabled)
					{
						ImGui::EndDisabled();
					}

					UI::EndPropertyGrid();
				}
			}
		});

		//DONE - Multi Edit
		DrawComponent<CameraComponent>("Camera", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					UI::BeginPropertyGrid();

					SceneCamera& camera = aFirstComponent.camera;
					
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<bool, CameraComponent>([](const CameraComponent& aOther) { return aOther.primary; }));
					if (UI::Property_Checkbox("Primary", aFirstComponent.primary))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							entity.GetComponent<CameraComponent>().primary = aFirstComponent.primary;
						}
					}
					ImGui::PopItemFlag();

					const char* projTypeStrings[] = { "Perspective", "Orthographic" };
					int currentProj = (int)camera.GetProjectionType();
					const bool inconsistentProjType = IsInconsistentPrimitive<int, CameraComponent>([](const CameraComponent& aOther) { return (int)aOther.camera.GetProjectionType(); });
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && inconsistentProjType);
					if (UI::Property_Dropdown("Projection", projTypeStrings, 2, currentProj, &inconsistentProjType))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);	
							entity.GetComponent<CameraComponent>().camera.SetProjectionType((SceneCamera::ProjectionType)currentProj);
						}
					}
					ImGui::PopItemFlag();

					if (!inconsistentProjType)
					{
						if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
						{
							float perspectiveFov = camera.GetPerspectiveFOV() * CU::Math::ToDeg;
							ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, CameraComponent>([](const CameraComponent& aOther) { return aOther.camera.GetPerspectiveFOV(); }));
							if (UI::Property_DragFloat("FOV", perspectiveFov, 1.0f, 5.0f, 140.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp))
							{
								for (auto& entityID : aEntities)
								{
									Entity entity = myContext->GetEntityWithUUID(entityID);
									entity.GetComponent<CameraComponent>().camera.SetPerspectiveFOV(perspectiveFov * CU::Math::ToRad);
								}
							}
							ImGui::PopItemFlag();

							float perspectiveNear = camera.GetPerspectiveNearPlane() * 0.01f;
							ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, CameraComponent>([](const CameraComponent& aOther) { return aOther.camera.GetPerspectiveNearPlane(); }));
							if (UI::Property_DragFloat("Near", perspectiveNear, 0.3f, 0.0f, camera.GetPerspectiveFarPlane() * 0.01f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
							{
								for (auto& entityID : aEntities)
								{
									Entity entity = myContext->GetEntityWithUUID(entityID);
									entity.GetComponent<CameraComponent>().camera.SetPerspectiveNearPlane(perspectiveNear * 100.0f);
								}
							}
							ImGui::PopItemFlag();

							float perspectiveFar = camera.GetPerspectiveFarPlane() * 0.01f;
							ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, CameraComponent>([](const CameraComponent& aOther) { return aOther.camera.GetPerspectiveFarPlane(); }));
							if (UI::Property_DragFloat("Far", perspectiveFar, 0.3f, camera.GetPerspectiveNearPlane() * 0.01f, FLT_MAX, "%.2f", ImGuiSliderFlags_AlwaysClamp))
							{
								for (auto& entityID : aEntities)
								{
									Entity entity = myContext->GetEntityWithUUID(entityID);
									entity.GetComponent<CameraComponent>().camera.SetPerspectiveFarPlane(perspectiveFar * 100.0f);
								}
							}
							ImGui::PopItemFlag();
						}
						else if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
						{
							float orthoSize = camera.GetOrthographicSize() * 0.01f;
							ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, CameraComponent>([](const CameraComponent& aOther) { return aOther.camera.GetOrthographicSize(); }));
							if (UI::Property_DragFloat("Size", orthoSize, 0.1f, (float)INT32_MIN, FLT_MAX, "%.2f", ImGuiSliderFlags_AlwaysClamp))
							{
								for (auto& entityID : aEntities)
								{
									Entity entity = myContext->GetEntityWithUUID(entityID);
									entity.GetComponent<CameraComponent>().camera.SetOrthographicSize(orthoSize * 100.0f);
								}
							}
							ImGui::PopItemFlag();

							float orthoNear = camera.GetOrthographicNearPlane() * 0.01f;
							ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, CameraComponent>([](const CameraComponent& aOther) { return aOther.camera.GetOrthographicNearPlane(); }));
							if (UI::Property_DragFloat("Near", orthoNear, 0.3f, (float)INT32_MIN, camera.GetOrthographicFarPlane() * 0.01f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
							{
								for (auto& entityID : aEntities)
								{
									Entity entity = myContext->GetEntityWithUUID(entityID);
									entity.GetComponent<CameraComponent>().camera.SetOrthographicNearPlane(orthoNear * 100.0f);
								}
							}
							ImGui::PopItemFlag();

							float orthoFar = camera.GetOrthographicFarPlane() * 0.01f;
							ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, CameraComponent>([](const CameraComponent& aOther) { return aOther.camera.GetOrthographicFarPlane(); }));
							if (UI::Property_DragFloat("Far", orthoFar, 0.3f, camera.GetOrthographicNearPlane() * 0.01f, FLT_MAX, "%.2f", ImGuiSliderFlags_AlwaysClamp))
							{
								for (auto& entityID : aEntities)
								{
									Entity entity = myContext->GetEntityWithUUID(entityID);
									entity.GetComponent<CameraComponent>().camera.SetOrthographicFarPlane(orthoFar * 100.0f);
								}
							}
							ImGui::PopItemFlag();
						}
					}

					UI::EndPropertyGrid();
				}
			}, EditorResources::CameraIcon);

		DrawComponent<ScriptComponent>("Script", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					//UI::BeginPropertyGrid();

					const bool inconsistentScriptClass = IsInconsistentPrimitive<AssetHandle, ScriptComponent>([](const ScriptComponent& aOther) { return aOther.scriptClassHandle; });
					//ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && inconsistentScriptClass);
					
					bool isError = !ScriptEngine::IsModuleValid(aFirstComponent.scriptClassHandle);

					UI::BeginPropertyGrid();
					ImGui::BeginDisabled();
					AssetHandle oldAssetHandle = 0;
					UI::Property_AssetReference<ScriptAsset>("Script Class", aFirstComponent.scriptClassHandle, "", { true, true });
					ImGui::EndDisabled();
					UI::EndPropertyGrid();

					//AssetHandle oldAssetHandle = 0;
					//if (UI::Property_AssetReference<ScriptAsset>("Script Class", aFirstComponent.scriptClassHandle, true))
					//{
					//	isError = !ScriptEngine::IsModuleValid(aFirstComponent.scriptClassHandle);
					//
					//	for (auto entityID : aEntities)
					//	{
					//		Entity entity = myContext->GetEntityWithUUID(entityID);
					//		auto& sc = entity.GetComponent<ScriptComponent>();
					//		sc.scriptClassHandle = aFirstComponent.scriptClassHandle;
					//
					//		if (isError)
					//		{
					//			bool wasCleared = sc.scriptClassHandle == 0;
					//			if (wasCleared)
					//			{
					//				sc.scriptClassHandle = oldAssetHandle;
					//			}
					//
					//			ScriptEngine::ShutdownScriptEntity(entity);
					//
					//			if (wasCleared)
					//			{
					//				sc.scriptClassHandle = 0;
					//			}
					//		}
					//		else
					//		{
					//			ScriptEngine::InitializeScriptEntity(entity);
					//		}
					//	}
					//}
					//
					//ImGui::PopItemFlag();
					//
					//UI::EndPropertyGrid();

					if (!isError && !inconsistentScriptClass && aFirstComponent.fieldIDs.size() > 0)
					{
						//UI::Spacing();
						UI::BeginPropertyGrid();

						Entity firstEntity = myContext->GetEntityWithUUID(aEntities[0]);

						bool inCollapsibleHeader = false;
						bool collapsibleHeaderOpen = false;
						std::string currentHeaderName = "";
						for (size_t i = 0; i < aFirstComponent.fieldIDs.size(); i++)
						{
							FieldInfo* field = ScriptCache::GetFieldByID(aFirstComponent.fieldIDs[i]);
							std::shared_ptr<FieldStorageBase> storage = ScriptEngine::GetFieldStorage(firstEntity, field->id);

							bool addSpace = field->spacing > 0;

							if (field->header != currentHeaderName)
							{
								UI::EndPropertyGrid();
								if (addSpace)
								{
									UI::Spacing(field->spacing);
								}

								//if (inCollapsibleHeader && collapsibleHeaderOpen)
								//{
								//	ImGui::TreePop();
								//}

								if (field->header.empty())
								{
									inCollapsibleHeader = false;
									currentHeaderName = "";
								}
								else
								{
									inCollapsibleHeader = true;
									currentHeaderName = field->header;

									if (i == 0)
									{
										UI::Spacing();
									}

									collapsibleHeaderOpen = UI::SubHeader(field->header.c_str(), true);
								}

								UI::BeginPropertyGrid();
							}
							else if (addSpace)
							{
								UI::EndPropertyGrid();
								UI::Spacing(field->spacing);
								UI::BeginPropertyGrid();
							}

							std::string fieldName = "";
							{
								fieldName = CU::SubStr(field->name, field->name.find(':') + 1);

								fieldName = CU::SplitAtUpperCase(fieldName);

								if (CU::StartsWith(fieldName, "my ") || CU::StartsWith(fieldName, "m_ "))
								{
									fieldName.erase(0, 3);
								}
								else if (CU::StartsWith(fieldName, "_ "))
								{
									fieldName.erase(0, 2);
								}
								else if (CU::StartsWith(fieldName, "_"))
								{
									fieldName.erase(0, 1);
								}

								fieldName[0] = toupper(fieldName[0]);
							}

							if ((inCollapsibleHeader && collapsibleHeaderOpen) || !inCollapsibleHeader)
							{
								if (field->IsArray())
								{
									UI::EndPropertyGrid();

									auto arrayFieldStorage = std::dynamic_pointer_cast<ArrayFieldStorage>(storage);
									UI::DrawFieldArray(myContext, fieldName, arrayFieldStorage, field->tooltip.c_str());

									UI::BeginPropertyGrid();
								}
								else
								{
									auto fieldStorage = std::dynamic_pointer_cast<FieldStorage>(storage);
									UI::DrawFieldValue(myContext, fieldName, fieldStorage, field->tooltip.c_str());
								}
							}

							if (inCollapsibleHeader && i == aFirstComponent.fieldIDs.size() - 1)
							{
								inCollapsibleHeader = false;
								currentHeaderName = "";

								//if (collapsibleHeaderOpen)
								//{
								//	ImGui::TreePop();
								//}
								//else
								if (!collapsibleHeaderOpen)
								{
									UI::Spacing();
								}
							}
						}

						UI::EndPropertyGrid();
					}
				}
			}, EditorResources::ScriptIcon);

		DrawComponent<ParticleSystemComponent>("Particle System", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					UI::BeginPropertyGrid();

					UI::Property_DragFloat("Duration", aFirstComponent.duration);
					UI::Property_Checkbox("Looping", aFirstComponent.looping);
					UI::Property_Checkbox("Prewarm", aFirstComponent.prewarm);

					UI::Property_DragFloat("Start Lifetime", aFirstComponent.startLifetime);
					UI::Property_DragFloat("Start Speed", aFirstComponent.startSpeed);
					UI::Property_DragFloat("Start Size", aFirstComponent.startSize);
					UI::Property_ColorEdit4("Start Color", aFirstComponent.startColor);

					UI::EndPropertyGrid();

					if (UI::PropertyGridHeader("Emission", false))
					{
						UI::BeginPropertyGrid();

						UI::Property_DragFloat("Rate", aFirstComponent.emission.rate);

						UI::EndPropertyGrid();

						ImGui::TreePop();
					}

					if (UI::PropertyGridHeader("Shape", false))
					{
						UI::BeginPropertyGrid();

						const char* shapeStrings[] = { "Sphere", "Box", "Cone" };
						int currentShape = (int)aFirstComponent.shape.shape;
						if (UI::Property_Dropdown("Shape", shapeStrings, 3, currentShape))
						{
							aFirstComponent.shape.shape = (ParticleSystem::EmitterShape)currentShape;
						}

						if (aFirstComponent.shape.shape == ParticleSystem::EmitterShape::Cone)
						{
							UI::Property_DragFloat("Angle", aFirstComponent.shape.angle);
							UI::Property_DragFloat("Radius", aFirstComponent.shape.radius);
						}
						else if (aFirstComponent.shape.shape == ParticleSystem::EmitterShape::Sphere)
						{
							UI::Property_DragFloat("Radius", aFirstComponent.shape.radius);
						}

						CU::Vector3f position = aFirstComponent.shape.position * 0.01f;
						if (UI::Property_DragFloat3("Position", position))
						{
							aFirstComponent.shape.position = position * 100.0f;
						}

						UI::Property_DragFloat3("Rotation", aFirstComponent.shape.rotation);
						UI::Property_DragFloat3("Scale", aFirstComponent.shape.scale);

						UI::EndPropertyGrid();

						ImGui::TreePop();
					}

					if (UI::PropertyGridHeader("Velocity Over Lifetime", false))
					{
						UI::BeginPropertyGrid();

						UI::Property_DragFloat3("Linear", aFirstComponent.velocityOverLifetime.linear);

						UI::EndPropertyGrid();

						ImGui::TreePop();
					}

					if (UI::PropertyGridHeader("Color Over Lifetime", false))
					{
						UI::BeginPropertyGrid();

						UI::Property_Gradient("Gradient", aFirstComponent.colorOverLifetime.colorGradient);

						UI::EndPropertyGrid();

						ImGui::TreePop();
					}

					if (UI::PropertyGridHeader("Size Over Lifetime", false))
					{
						UI::BeginPropertyGrid();

						UI::Property_DragFloat("Start", aFirstComponent.sizeOverLifetime.start);
						UI::Property_DragFloat("End", aFirstComponent.sizeOverLifetime.end);

						UI::EndPropertyGrid();

						ImGui::TreePop();
					}
				}
			});

		//DONE - Multi Edit
		DrawComponent<MeshRendererComponent>("Mesh Renderer", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					UI::BeginPropertyGrid();

					AssetHandle assetHandle = aFirstComponent.mesh;
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<AssetHandle, MeshRendererComponent>([](const MeshRendererComponent& aOther) { return aOther.mesh; }));
					if (UI::Property_AssetReference<Mesh>("Mesh", assetHandle, "", { true }))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& mrc = entity.GetComponent<MeshRendererComponent>();
							mrc.mesh = assetHandle;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<bool, MeshRendererComponent>([](const MeshRendererComponent& aOther) { return aOther.castsShadows; }));
					if (UI::Property_Checkbox("Casts Shadows", aFirstComponent.castsShadows))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& mrc = entity.GetComponent<MeshRendererComponent>();
							mrc.castsShadows = aFirstComponent.castsShadows;
						}
					}
					ImGui::PopItemFlag();

					UI::EndPropertyGrid();

					//std::shared_ptr<Mesh> meshAsset = AssetManager::GetAssetAsync<Mesh>(assetHandle);
					DrawMaterialTable<MeshRendererComponent>(aEntities, aFirstComponent.materialTable);
				}
			}, EditorResources::MeshRendererIcon);

		//DONE - Multi Edit
		DrawComponent<SkinnedMeshRendererComponent>("Skinned Mesh Renderer", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					UI::BeginPropertyGrid();

					AssetHandle assetHandle = aFirstComponent.mesh;
					if (UI::Property_AssetReference<Mesh>("Mesh", assetHandle, "", { true }))
					{
						aFirstComponent.mesh = assetHandle;
					}

					const bool inconsistentRootBone = IsInconsistentPrimitive<UUID, SkinnedMeshRendererComponent>([](const SkinnedMeshRendererComponent& aOther) { return aOther.boneEntityIds.empty() ? (UUID)0 : aOther.boneEntityIds[0]; });
					UUID rootBoneEntityID = (isMultiSelect && inconsistentRootBone) ? (UUID)0 : aFirstComponent.boneEntityIds.empty() ? (UUID)0 : aFirstComponent.boneEntityIds[0];
					Entity boneEntity = myContext->TryGetEntityWithUUID(rootBoneEntityID);

					if (inconsistentRootBone)
					{
						UI::Property_Text("Root Bone", "---");
					}
					else if (!boneEntity)
					{
						UI::Property_ColoredText("Root Bone", "None", IM_COL32(255, 50, 30, 255));
					}
					else
					{
						UI::Property_Text("Root Bone", boneEntity.GetName());
					}

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<bool, SkinnedMeshRendererComponent>([](const SkinnedMeshRendererComponent& aOther) { return aOther.castsShadows; }));
					if (UI::Property_Checkbox("Casts Shadows", aFirstComponent.castsShadows))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& smrc = entity.GetComponent<SkinnedMeshRendererComponent>();
							smrc.castsShadows = aFirstComponent.castsShadows;
						}
					}
					ImGui::PopItemFlag();

					UI::EndPropertyGrid();
				}
			}, EditorResources::MeshRendererIcon);

		//DONE - Multi Edit
		DrawComponent<SpriteRendererComponent>("Sprite Renderer", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					UI::BeginPropertyGrid();

					AssetHandle assetHandle = aFirstComponent.texture;
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<AssetHandle, SpriteRendererComponent>([](const SpriteRendererComponent& aOther) { return aOther.texture; }));
					if (UI::Property_AssetReference<Texture2D>("Sprite", assetHandle))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& src = entity.GetComponent<SpriteRendererComponent>();
							src.texture = assetHandle;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<CU::Vector4f, SpriteRendererComponent>([](const SpriteRendererComponent& aOther) { return aOther.tint.GetVector4(); }));
					UI::Property_ColorEdit4("Tint", aFirstComponent.tint);
					ImGui::PopItemFlag();

					UI::BeginCheckboxGroup("Flip");
					
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<bool, SpriteRendererComponent>([](const SpriteRendererComponent& aOther) { return aOther.flipX; }));
					UI::Property_GroupCheckbox("X", aFirstComponent.flipX);
					ImGui::PopItemFlag();
					
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<bool, SpriteRendererComponent>([](const SpriteRendererComponent& aOther) { return aOther.flipY; }));
					UI::Property_GroupCheckbox("Y", aFirstComponent.flipY);
					ImGui::PopItemFlag();

					UI::EndCheckboxGroup();

					UI::EndPropertyGrid();
				}
			}, EditorResources::SpriteRendererIcon);

		//DrawComponent<VideoPlayerComponent>("Video Player", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
		//	{
		//		{
		//			UI::BeginPropertyGrid();
		//
		//			AssetHandle assetHandle = aFirstComponent.video;
		//			//if (UI::Property_AssetReference<Texture2D>("Sprite", assetHandle))
					//{
					//	aFirstComponent.texture = assetHandle;
					//}
		//			UI::Property_Text("Video", "None");
		//
		//			UI::Property_Checkbox("Play On Awake", aFirstComponent.playOnAwake);
		//			UI::Property_Checkbox("Loop", aFirstComponent.loop);
		//
		//			UI::Property_DragFloat("Playback Speed", aFirstComponent.playbackSpeed, 0.02f);
		//
		//			UI::EndPropertyGrid();
		//		}
		//	}, EditorResources::CameraIcon);

		DrawComponent<TextRendererComponent>("Text Renderer", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					UI::BeginPropertyGrid();

					UI::Property_InputTextMultiline("Text", aFirstComponent.text, CU::Vector2f::Zero, ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CtrlEnterForNewLine);
					
					AssetHandle assetHandle = aFirstComponent.font;
					if (UI::Property_AssetReference<Font>("Font", assetHandle))
					{
						aFirstComponent.font = assetHandle;
					}
					
					UI::Property_ColorEdit3("Color", aFirstComponent.color);

					//const char* alignmentStrings[] = { "UpperLeft", "UpperCenter", "UpperRight", "MiddleLeft", "MiddleCenter", "MiddleRight", "LowerLeft", "LowerCenter", "LowerRight" };
					//int currentAlignment = (int)aFirstComponent.alignment;
					//if (UI::Property_Dropdown("Alignment", alignmentStrings, 9, currentAlignment))
					//{
					//	aFirstComponent.alignment = (Alignment)currentAlignment;
					//}
					UI::Property_Checkbox("Centered", aFirstComponent.centered);

					UI::Property_DragFloat("Letter Spacing", aFirstComponent.letterSpacing, 0.02f);
					UI::Property_DragFloat("Line Spacing", aFirstComponent.lineSpacing, 0.02f);
					UI::Property_DragFloat("Line Width", aFirstComponent.maxWidth, 0.05f);

					UI::EndPropertyGrid();
				}
			}, EditorResources::TextRendererIcon);

		DrawComponent<SkyLightComponent>("Sky Light", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					UI::BeginPropertyGrid();

					AssetHandle assetHandle = aFirstComponent.environment;
					if (UI::Property_AssetReference<Environment>("Environment", assetHandle, "", { true }))
					{
						aFirstComponent.environment = assetHandle;
					}

					UI::Property_DragFloat("Intensity", aFirstComponent.intensity, 0.01f, 0.0f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp);

					UI::EndPropertyGrid();
				}
			}, EditorResources::SkyLightIcon);

		DrawComponent<DirectionalLightComponent>("Directional Light", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					UI::BeginPropertyGrid();

					UI::Property_ColorEdit3("Color", aFirstComponent.color);
					UI::Property_DragFloat("Intensity", aFirstComponent.intensity, 0.01f, 0.0f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp);
					UI::Property_Checkbox("Casts Shadows", aFirstComponent.castsShadows);

					UI::EndPropertyGrid();
				}
			}, EditorResources::DirectionalLightIcon);
		
		//DONE - Multi Edit
		DrawComponent<SpotlightComponent>("Spotlight", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					UI::BeginPropertyGrid();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<CU::Vector3f, SpotlightComponent>([](const SpotlightComponent& aOther) { return aOther.color.GetVector3(); }));
					if (UI::Property_ColorEdit3("Color", aFirstComponent.color))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& sl = entity.GetComponent<SpotlightComponent>();
							sl.color = aFirstComponent.color;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, SpotlightComponent>([](const SpotlightComponent& aOther) { return aOther.intensity; }));
					if (UI::Property_DragFloat("Intensity", aFirstComponent.intensity, 0.01f, 0.0f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& sl = entity.GetComponent<SpotlightComponent>();
							sl.intensity = aFirstComponent.intensity;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, SpotlightComponent>([](const SpotlightComponent& aOther) { return aOther.range; }));
					float range = aFirstComponent.range * 0.01f;
					if (UI::Property_DragFloat("Range", range))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& sl = entity.GetComponent<SpotlightComponent>();
							sl.range = range * 100.0f;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, SpotlightComponent>([](const SpotlightComponent& aOther) { return aOther.outerSpotAngle; }));
					float outerAngle = aFirstComponent.outerSpotAngle * CU::Math::ToDeg;
					if (UI::Property_DragFloat("Outer Angle", outerAngle))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& sl = entity.GetComponent<SpotlightComponent>();
							sl.outerSpotAngle = outerAngle * CU::Math::ToRad;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, SpotlightComponent>([](const SpotlightComponent& aOther) { return aOther.innerSpotAngle; }));
					float innerAngle = aFirstComponent.innerSpotAngle * CU::Math::ToDeg;
					if (UI::Property_DragFloat("Inner Angle", innerAngle))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& sl = entity.GetComponent<SpotlightComponent>();
							sl.innerSpotAngle = innerAngle * CU::Math::ToRad;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<AssetHandle, SpotlightComponent>([](const SpotlightComponent& aOther) { return aOther.cookieTexture; }));
					if (UI::Property_AssetReference<Texture2D>("Cookie", aFirstComponent.cookieTexture))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& sl = entity.GetComponent<SpotlightComponent>();
							sl.cookieTexture = aFirstComponent.cookieTexture;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<bool, SpotlightComponent>([](const SpotlightComponent& aOther) { return aOther.castsShadows; }));
					if (UI::Property_Checkbox("Casts Shadows", aFirstComponent.castsShadows))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& sl = entity.GetComponent<SpotlightComponent>();
							sl.castsShadows = aFirstComponent.castsShadows;
						}
					}
					ImGui::PopItemFlag();

					UI::EndPropertyGrid();
				}
			}, EditorResources::SpotlightIcon);
		
		//DONE - Multi Edit
		DrawComponent<PointLightComponent>("Point Light", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					UI::BeginPropertyGrid();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<CU::Vector3f, PointLightComponent>([](const PointLightComponent& aOther) { return aOther.color.GetVector3(); }));
					if (UI::Property_ColorEdit3("Color", aFirstComponent.color))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& pl = entity.GetComponent<PointLightComponent>();
							pl.color = aFirstComponent.color;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, PointLightComponent>([](const PointLightComponent& aOther) { return aOther.intensity; }));
					if (UI::Property_DragFloat("Intensity", aFirstComponent.intensity, 0.01f, 0.0f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& pl = entity.GetComponent<PointLightComponent>();
							pl.intensity = aFirstComponent.intensity;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, PointLightComponent>([](const PointLightComponent& aOther) { return aOther.range; }));
					float range = aFirstComponent.range * 0.01f;
					if (UI::Property_DragFloat("Range", range))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& pl = entity.GetComponent<PointLightComponent>();
							pl.range = range * 100.0f;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<bool, PointLightComponent>([](const PointLightComponent& aOther) { return aOther.castsShadows; }));
					if (UI::Property_Checkbox("Casts Shadows", aFirstComponent.castsShadows))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& pl = entity.GetComponent<PointLightComponent>();
							pl.castsShadows = aFirstComponent.castsShadows;
						}
					}
					ImGui::PopItemFlag();

					UI::EndPropertyGrid();
				}
			}, EditorResources::PointLightIcon);
		
		//DONE - Multi Edit (Constraints mixed values not implemented)
		DrawComponent<RigidbodyComponent>("Rigidbody", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					UI::BeginPropertyGrid();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, RigidbodyComponent>([](const RigidbodyComponent& aOther) { return aOther.mass; }));
					if (UI::Property_DragFloat("Mass", aFirstComponent.mass, 0.1f, 0.01f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& rb = entity.GetComponent<RigidbodyComponent>();
							rb.mass = aFirstComponent.mass;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, RigidbodyComponent>([](const RigidbodyComponent& aOther) { return aOther.linearDrag; }));
					if (UI::Property_DragFloat("Linear Drag", aFirstComponent.linearDrag, 0.1f, 0.0f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& rb = entity.GetComponent<RigidbodyComponent>();
							rb.linearDrag = aFirstComponent.linearDrag;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<float, RigidbodyComponent>([](const RigidbodyComponent& aOther) { return aOther.angularDrag; }));
					if (UI::Property_DragFloat("Angular Drag", aFirstComponent.angularDrag, 0.1f, 0.0f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& rb = entity.GetComponent<RigidbodyComponent>();
							rb.angularDrag = aFirstComponent.angularDrag;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<bool, RigidbodyComponent>([](const RigidbodyComponent& aOther) { return aOther.useGravity; }));
					if (UI::Property_Checkbox("Use Gravity", aFirstComponent.useGravity))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& rb = entity.GetComponent<RigidbodyComponent>();
							rb.useGravity = aFirstComponent.useGravity;
						}
					}
					ImGui::PopItemFlag();

					//ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<bool, RigidbodyComponent>([](const RigidbodyComponent& aOther) { return aOther.isKinematic; }));
					//if (UI::Property_Checkbox("Is Kinematic ", aFirstComponent.isKinematic))
					//{
					//	for (auto& entityID : aEntities)
					//	{
					//		Entity entity = myContext->GetEntityWithUUID(entityID);
					//		auto& rb = entity.GetComponent<RigidbodyComponent>();
					//		rb.isKinematic = aFirstComponent.isKinematic;
					//	}
					//}
					//ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<CU::Vector3f, RigidbodyComponent>([](const RigidbodyComponent& aOther) { return aOther.initialLinearVelocity; }));
					if (UI::Property_DragFloat3("Initial Linear Velocity", aFirstComponent.initialLinearVelocity))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& rb = entity.GetComponent<RigidbodyComponent>();
							rb.initialLinearVelocity = aFirstComponent.initialLinearVelocity;
						}
					}
					ImGui::PopItemFlag();

					CU::Vector3f initialAngularVelocity = aFirstComponent.initialAngularVelocity * CU::Math::ToDeg;
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<CU::Vector3f, RigidbodyComponent>([](const RigidbodyComponent& aOther) { return aOther.initialAngularVelocity; }));
					if (UI::Property_DragFloat3("Initial Angular Velocity", initialAngularVelocity))
					{
						initialAngularVelocity *= CU::Math::ToRad;

						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& rb = entity.GetComponent<RigidbodyComponent>();
							rb.initialAngularVelocity = initialAngularVelocity;
						}
					}
					ImGui::PopItemFlag();

					UI::EndPropertyGrid();

					//TODO: Make constraints multi-editable
					if (UI::SubHeader("Constraints", false))
					{
						UI::BeginPropertyGrid();

						const Physics::Axis constraints = aFirstComponent.constraints;

						bool translationX = (constraints & Physics::Axis::TranslationX) != Physics::Axis::None;
						bool translationY = (constraints & Physics::Axis::TranslationY) != Physics::Axis::None;
						bool translationZ = (constraints & Physics::Axis::TranslationZ) != Physics::Axis::None;

						//Freeze Position
						{
							UI::BeginCheckboxGroup("Freeze Position");

							if (UI::Property_GroupCheckbox("X", translationX))
							{
								for (auto& entityID : aEntities)
								{
									Entity entity = myContext->GetEntityWithUUID(entityID);
									auto& rb = entity.GetComponent<RigidbodyComponent>();

									translationX ?
										rb.constraints |= Physics::Axis::TranslationX :
										rb.constraints &= ~Physics::Axis::TranslationX;
								}
							}

							if (UI::Property_GroupCheckbox("Y", translationY))
							{
								for (auto& entityID : aEntities)
								{
									Entity entity = myContext->GetEntityWithUUID(entityID);
									auto& rb = entity.GetComponent<RigidbodyComponent>();

									translationY ?
										rb.constraints |= Physics::Axis::TranslationY :
										rb.constraints &= ~Physics::Axis::TranslationY;
								}
							}

							if (UI::Property_GroupCheckbox("Z", translationZ))
							{
								for (auto& entityID : aEntities)
								{
									Entity entity = myContext->GetEntityWithUUID(entityID);
									auto& rb = entity.GetComponent<RigidbodyComponent>();

									translationZ ?
										rb.constraints |= Physics::Axis::TranslationZ :
										rb.constraints &= ~Physics::Axis::TranslationZ;
								}
							}

							UI::EndCheckboxGroup();
						}

						bool rotationX = (constraints & Physics::Axis::RotationX) != Physics::Axis::None;
						bool rotationY = (constraints & Physics::Axis::RotationY) != Physics::Axis::None;
						bool rotationZ = (constraints & Physics::Axis::RotationZ) != Physics::Axis::None;

						//Freeze Rotation
						{
							UI::BeginCheckboxGroup("Freeze Rotation");

							if (UI::Property_GroupCheckbox("X", rotationX))
							{
								for (auto& entityID : aEntities)
								{
									Entity entity = myContext->GetEntityWithUUID(entityID);
									auto& rb = entity.GetComponent<RigidbodyComponent>();

									rotationX ?
										rb.constraints |= Physics::Axis::RotationX :
										rb.constraints &= ~Physics::Axis::RotationX;
								}
							}

							if (UI::Property_GroupCheckbox("Y", rotationY))
							{
								for (auto& entityID : aEntities)
								{
									Entity entity = myContext->GetEntityWithUUID(entityID);
									auto& rb = entity.GetComponent<RigidbodyComponent>();

									rotationY ?
										rb.constraints |= Physics::Axis::RotationY :
										rb.constraints &= ~Physics::Axis::RotationY;
								}
							}

							if (UI::Property_GroupCheckbox("Z", rotationZ))
							{
								for (auto& entityID : aEntities)
								{
									Entity entity = myContext->GetEntityWithUUID(entityID);
									auto& rb = entity.GetComponent<RigidbodyComponent>();

									rotationZ ?
										rb.constraints |= Physics::Axis::RotationZ :
										rb.constraints &= ~Physics::Axis::RotationZ;
								}
							}

							UI::EndCheckboxGroup();
						}

						UI::EndPropertyGrid();
						//ImGui::TreePop();
					}

					if (UI::SubHeader("Info", false))
					{
						if (myContext->IsPlaying())
						{
							auto physicsBody = myContext->GetPhysicsScene()->GetPhysicsBody(firstEntity);
							if (physicsBody)
							{
								UI::BeginPropertyGrid();

								ImGui::BeginDisabled();
								CU::Vector3f velocity = physicsBody->GetVelocity();
								CU::Vector3f angularVelocity = physicsBody->GetAngularVelocity();
								UI::Property_DragFloat3("Velocity", velocity);
								UI::Property_DragFloat3("Angular Velocity", angularVelocity);
								ImGui::EndDisabled();
								
								UI::EndPropertyGrid();
							}
						}
						else
						{
							UI::BeginPropertyGrid();

							ImGui::BeginDisabled();
							CU::Vector3f velocity;
							CU::Vector3f angularVelocity;
							UI::Property_DragFloat3("Velocity", velocity);
							UI::Property_DragFloat3("Angular Velocity", angularVelocity);
							ImGui::EndDisabled();

							UI::EndPropertyGrid();
						}
					}
				}
			}, EditorResources::RigidbodyIcon);

		//DONE - Multi Edit
		DrawComponent<BoxColliderComponent>("Box Collider", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					UI::BeginPropertyGrid();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<CU::Vector3f, BoxColliderComponent>([](const BoxColliderComponent& aOther) { return aOther.halfSize; }));
					CU::Vector3f halfSize = aFirstComponent.halfSize * 0.01f;
					if (UI::Property_DragFloat3("Half Size", halfSize, 0.05f, 0.01f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& bc = entity.GetComponent<BoxColliderComponent>();
							bc.halfSize = halfSize * 100.0f;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<CU::Vector3f, BoxColliderComponent>([](const BoxColliderComponent& aOther) { return aOther.offset; }));
					CU::Vector3f offset = aFirstComponent.offset * 0.01f;
					if (UI::Property_DragFloat3("Offset", offset))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& bc = entity.GetComponent<BoxColliderComponent>();
							bc.offset = offset * 100.0f;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<bool, BoxColliderComponent>([](const BoxColliderComponent& aOther) { return aOther.isTrigger; }));
					if (UI::Property_Checkbox("Is Triger ", aFirstComponent.isTrigger))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& bc = entity.GetComponent<BoxColliderComponent>();
							bc.isTrigger = aFirstComponent.isTrigger;
						}
					}
					ImGui::PopItemFlag();

					UI::EndPropertyGrid();
				}
			}, EditorResources::BoxColliderIcon);

		DrawComponent<SphereColliderComponent>("Sphere Collider", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					UI::BeginPropertyGrid();

					float radius = aFirstComponent.radius * 0.01f;
					if (UI::Property_DragFloat("Radius", radius, 0.05f, 0.01f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
					{
						aFirstComponent.radius = radius * 100.0f;
					}

					CU::Vector3f offset = aFirstComponent.offset * 0.01f;
					if (UI::Property_DragFloat3("Offset", offset))
					{
						aFirstComponent.offset = offset * 100.0f;
					}

					UI::Property_Checkbox("Is Triger ", aFirstComponent.isTrigger);

					UI::EndPropertyGrid();
				}
			}, EditorResources::SphereColliderIcon);

		DrawComponent<CapsuleColliderComponent>("Capsule Collider", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					UI::BeginPropertyGrid();

					float radius = aFirstComponent.radius * 0.01f;
					if (UI::Property_DragFloat("Radius", radius, 0.05f, 0.01f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
					{
						aFirstComponent.radius = radius * 100.0f;
					}

					float height = aFirstComponent.height * 0.01f;
					if (UI::Property_DragFloat("Height", height, 0.1f, 0.0f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
					{
						aFirstComponent.height = height * 100.0f;
					}

					CU::Vector3f offset = aFirstComponent.offset * 0.01f;
					if (UI::Property_DragFloat3("Offset", offset))
					{
						aFirstComponent.offset = offset * 100.0f;
					}

					UI::Property_Checkbox("Is Triger ", aFirstComponent.isTrigger);

					UI::EndPropertyGrid();
				}
			}, EditorResources::CapsuleColliderIcon);

		DrawComponent<CharacterControllerComponent>("Character Controller", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					UI::BeginPropertyGrid();

					float height = aFirstComponent.height * 0.01f;
					if (UI::Property_DragFloat("Height", height, 0.1f, (aFirstComponent.radius * 0.01f) * 2.0f + 0.01f, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
					{
						aFirstComponent.height = height * 100.0f;
					}

					float radius = aFirstComponent.radius * 0.01f;
					if (UI::Property_DragFloat("Radius", radius, 0.05f, 0.01f, (aFirstComponent.height * 0.01f) / 2.0f - 0.01f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
					{
						aFirstComponent.radius = radius * 100.0f;
					}

					CU::Vector3f offset = aFirstComponent.offset * 0.01f;
					if (UI::Property_DragFloat3("Offset", offset))
					{
						aFirstComponent.offset = offset * 100.0f;
					}


					float stepOffset = aFirstComponent.stepOffset * 0.01f;
					if (UI::Property_DragFloat("Step Offset", stepOffset, 0.05f, 0.0f, aFirstComponent.height * 0.01f))
					{
						aFirstComponent.stepOffset = stepOffset * 100.0f;
					}

					UI::Property_DragFloat("Slope Limit", aFirstComponent.slopeLimit, 0.1f, 0.0f, 90.0f);

					UI::EndPropertyGrid();
				}
			}, EditorResources::CharacterControllerIcon);

		//DONE - Multi Edit
		DrawComponent<TerrainComponent>("Terrain", [&](auto& aFirstComponent, const std::vector<UUID>& aEntities, const bool aIsMultiEdit)
			{
				{
					UI::BeginPropertyGrid();

					AssetHandle assetHandle = aFirstComponent.heightmap;
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, aIsMultiEdit && IsInconsistentPrimitive<AssetHandle, TerrainComponent>([](const TerrainComponent& aOther) { return aOther.heightmap; }));
					if (UI::Property_AssetReference<Texture2D>("Heightmap", assetHandle, "", { true }))
					{
						for (auto& entityID : aEntities)
						{
							Entity entity = myContext->GetEntityWithUUID(entityID);
							auto& tc = entity.GetComponent<TerrainComponent>();
							tc.heightmap = assetHandle;
						}
					}
					ImGui::PopItemFlag();

					UI::EndPropertyGrid();
				}
			});

		//Wiki: DrawComponent

		ImGui::Separator();
		UI::Spacing();

		const int buttonWidth = 120;
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (buttonWidth * 0.5f));

		static std::string aComponentSearchString;
		if (ImGui::Button("Add Component", { buttonWidth, 30 }))
		{
			ImGui::OpenPopup("AddComponent");
		}

		static bool grabFocus = true;

		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.1f, 0.1f, 0.1f, 0.95f));
		if (ImGui::BeginPopup("AddComponent", ImGuiWindowFlags_NoDocking))
		{
			if (ImGui::GetCurrentWindow()->Appearing)
			{
				grabFocus = true;
				aComponentSearchString.clear();
			}

			ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (ImGui::CalcTextSize("Add Component").x * 0.5f));
			UI::Fonts::PushFont("Bold");
			ImGui::Text("Add Component");
			UI::Fonts::PopFont();

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::SetNextItemWidth(200);
			ImGui::InputTextWithHint("##ComponentSearch", "Search...", &aComponentSearchString);

			if (grabFocus)
			{
				if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)
					&& !ImGui::IsAnyItemActive()
					&& !ImGui::IsMouseClicked(0))
				{
					ImGui::SetKeyboardFocusHere(-1);
				}

				if (ImGui::IsItemFocused())
				{
					grabFocus = false;
				}
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			//Wiki: DisplayAddComponent
			if (aComponentSearchString.empty())
			{
				//Sort by category

				if (ImGui::BeginMenu("Physics"))
				{
					DisplayAddComponentEntry<BoxColliderComponent>("Box Collider");
					DisplayAddComponentEntry<CapsuleColliderComponent>("Capsule Collider");
					DisplayAddComponentEntry<CharacterControllerComponent>("Character Controller");
					DisplayAddComponentEntry<RigidbodyComponent>("Rigidbody");
					DisplayAddComponentEntry<SphereColliderComponent>("Sphere Collider");

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Rendering"))
				{
					DisplayAddComponentEntry<CameraComponent>("Camera");
					DisplayAddComponentEntry<DirectionalLightComponent>("Directional Light");
					DisplayAddComponentEntry<MeshRendererComponent>("Mesh Renderer");
					DisplayAddComponentEntry<PointLightComponent>("Point Light");
					DisplayAddComponentEntry<SkinnedMeshRendererComponent>("Skinned Mesh Renderer");
					DisplayAddComponentEntry<SkyLightComponent>("Sky Light");
					DisplayAddComponentEntry<SpotlightComponent>("Spotlight");
					DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
					DisplayAddComponentEntry<TextRendererComponent>("Text Renderer");
					DisplayAddComponentEntry<TerrainComponent>("Terrain");
					//DisplayAddComponentEntry<VideoPlayerComponent>("Video Player");

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Scripts"))
				{
					//DisplayAddComponentEntry<ScriptComponent>("Script");
					DisplayAddScriptComponentEntry();

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Effects"))
				{
					DisplayAddComponentEntry<VolumeComponent>("Volume");
					DisplayAddComponentEntry<ParticleSystemComponent>("Particle System");

					ImGui::EndMenu();
				}
			}
			else
			{
				//Alphabetical order (A-B-C-D-E-F-G-H-I-J-K-L-M-N-O-P-Q-R-S-T-U-V-W-X-Y-Z)

				DisplayAddComponentEntry<BoxColliderComponent>("Box Collider", aComponentSearchString);
				DisplayAddComponentEntry<CameraComponent>("Camera", aComponentSearchString);
				DisplayAddComponentEntry<CapsuleColliderComponent>("Capsule Collider", aComponentSearchString);
				DisplayAddComponentEntry<CharacterControllerComponent>("Character Controller", aComponentSearchString);
				DisplayAddComponentEntry<DirectionalLightComponent>("Directional Light", aComponentSearchString);
				DisplayAddComponentEntry<MeshRendererComponent>("Mesh Renderer", aComponentSearchString);
				DisplayAddComponentEntry<ParticleSystemComponent>("Particle System", aComponentSearchString);
				DisplayAddComponentEntry<PointLightComponent>("Point Light", aComponentSearchString);
				DisplayAddComponentEntry<RigidbodyComponent>("Rigidbody", aComponentSearchString);
				DisplayAddComponentEntry<SkinnedMeshRendererComponent>("Skinned Mesh Renderer", aComponentSearchString);
				//DisplayAddComponentEntry<ScriptComponent>("Script", aFirstComponentSearchString);
				DisplayAddComponentEntry<SkyLightComponent>("Sky Light", aComponentSearchString);
				DisplayAddComponentEntry<SphereColliderComponent>("Sphere Collider", aComponentSearchString);
				DisplayAddComponentEntry<SpotlightComponent>("Spotlight", aComponentSearchString);
				DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer", aComponentSearchString);
				DisplayAddComponentEntry<TerrainComponent>("Terrain", aComponentSearchString);
				DisplayAddComponentEntry<TextRendererComponent>("Text Renderer", aComponentSearchString);
				//DisplayAddComponentEntry<VideoPlayerComponent>("Video Player", aFirstComponentSearchString);
				DisplayAddComponentEntry<VolumeComponent>("Volume", aComponentSearchString);

				ImGui::Separator();

				DisplayAddScriptComponentEntry(aComponentSearchString);
			}

			ImGui::EndPopup();
		}

		ImGui::PopStyleColor();
	}

	template<typename T>
	void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& aEntryName, const std::string& aSearchFilter)
	{
		if (!UI::IsMatchingSearch(aEntryName, aSearchFilter)) return;

		bool canAddComponent = false;

		for (const auto& entityID : SelectionManager::GetSelections(SelectionContext::Entity))
		{
			Entity entity = myContext->GetEntityWithUUID(entityID);
			if (!entity.HasComponent<T>())
			{
				canAddComponent = true;
				break;
			}
		}

		if (ImGui::MenuItem(aEntryName.c_str(), 0, false, canAddComponent))
		{
			for (const auto& entityID : SelectionManager::GetSelections(SelectionContext::Entity))
			{
				Entity entity = myContext->GetEntityWithUUID(entityID);
				if (!entity.HasComponent<T>())
				{
					entity.AddComponent<T>();
				}
			}
			ImGui::CloseCurrentPopup();
		}
	}

	static void UnpackPrefab(Entity aPrefabEntity)
	{
		if (!aPrefabEntity.HasComponent<PrefabComponent>()) return;

		aPrefabEntity.RemoveComponent<PrefabComponent>();

		for (auto child : aPrefabEntity.GetChildren())
		{
			UnpackPrefab(child);
		}
	}

	void SceneHierarchyPanel::DisplayAddScriptComponentEntry(const std::string& aSearchFilter)
	{
		bool canAddComponent = false;
		for (const auto& entityID : SelectionManager::GetSelections(SelectionContext::Entity))
		{
			Entity entity = myContext->GetEntityWithUUID(entityID);
			if (!entity.HasComponent<ScriptComponent>())
			{
				canAddComponent = true;
				break;
			}
		}

		auto scriptAssets = AssetManager::GetAllAssetsWithType<ScriptAsset>();

		for (AssetHandle scriptClassHandle : scriptAssets)
		{
			const auto scriptName = Project::GetEditorAssetManager()->GetMetadata(scriptClassHandle).filePath.string();

			if (!UI::IsMatchingSearch(scriptName, aSearchFilter)) continue;
			if (!ScriptEngine::IsModuleValid(scriptClassHandle)) continue;
			

			if (ImGui::MenuItem(scriptName.c_str(), 0, false, canAddComponent))
			{
				for (const auto& entityID : SelectionManager::GetSelections(SelectionContext::Entity))
				{
					Entity entity = myContext->GetEntityWithUUID(entityID);
					if (!entity.HasComponent<ScriptComponent>())
					{
						ScriptComponent& sc = entity.AddComponent<ScriptComponent>();
						sc.scriptClassHandle = scriptClassHandle;
						ScriptEngine::InitializeScriptEntity(entity);
					}
				}
				ImGui::CloseCurrentPopup();
			}
		}
	}

	void SceneHierarchyPanel::HierarchyPopup()
	{
		if (!ImGui::BeginPopup("Hierarchy Popup", ImGuiWindowFlags_NoMove))
		{
			staticPopupHovered = false;
			return;
		}

		staticPopupHovered = ImGui::IsWindowHovered();

		bool entitySelected = SelectionManager::GetSelectionCount(SelectionContext::Entity) == 1;
		Entity selection = entitySelected ? myContext->GetEntityWithUUID(SelectionManager::GetSelections(SelectionContext::Entity)[0]) : Entity();

		bool entityChilded = entitySelected ? selection.GetParent() : false;
		bool entityIsPrefab = entitySelected ? selection.HasComponent<PrefabComponent>() : false;

		if (ImGui::MenuItem("Rename", 0, false, entitySelected))
		{
		}

		if (ImGui::MenuItem("Duplicate", 0, false, entitySelected))
		{
			Entity duplicate = myContext->DuplicateEntity(selection);
			SelectionManager::DeselectAll(SelectionContext::Entity);
			SelectionManager::Select(SelectionContext::Entity, duplicate.GetUUID());
		}

		if (ImGui::MenuItem("Delete", 0, false, entitySelected))
		{
			myContext->DestroyEntity(selection);
		}

		if (ImGui::MenuItem("Unparent", 0, false, entityChilded))
		{
			myContext->UnparentEntity(selection);
		}

		ImGui::Separator();

		if (ImGui::BeginMenu("Prefab", entityIsPrefab))
		{
			if (ImGui::MenuItem("Unpack Prefab"))
			{
				UnpackPrefab(selection);
			}

			if (ImGui::MenuItem("Update Prefab"))
			{
				AssetHandle prefabAssetHandle = selection.GetComponent<PrefabComponent>().prefabID;
				std::shared_ptr<Prefab> prefab = AssetManager::GetAsset<Prefab>(prefabAssetHandle);
				if (prefab)
				{
					prefab->Create(selection);
					AssetImporter::Serialize(prefab);
				}
				else
				{
					LOG_ERROR("Prefab has invalid asset handle: {}", prefabAssetHandle);
				}
			}

			ImGui::EndMenu();
		}

		ImGui::Separator();

		for (const EntityPopupPlugin& customFunc : myEntityPopupPlugins)
		{
			if (ImGui::MenuItem(customFunc.name.c_str(), 0, false, entitySelected))
			{
				if (customFunc.function)
				{
					customFunc.function(selection);
				}
			}
		}

		ImGui::Separator();

		Entity createdEntity;

		if (ImGui::MenuItem("Create Empty"))
		{
			createdEntity = myContext->CreateEntity("New Entity");
		}

		if (ImGui::BeginMenu("3D Object"))
		{
			if (ImGui::MenuItem("Cube"))
			{
				createdEntity = myContext->CreateEntity("Cube");
				auto& mrc = createdEntity.AddComponent<MeshRendererComponent>();
				mrc.mesh = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Cube - Built-In");
				createdEntity.AddComponent<BoxColliderComponent>();
			}

			if (ImGui::MenuItem("Sphere"))
			{
				createdEntity = myContext->CreateEntity("Sphere");
				auto& mrc = createdEntity.AddComponent<MeshRendererComponent>();
				mrc.mesh = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Sphere - Built-In");
				createdEntity.AddComponent<SphereColliderComponent>();
			}

			if (ImGui::MenuItem("Plane"))
			{
				createdEntity = myContext->CreateEntity("Plane");
				auto& mrc = createdEntity.AddComponent<MeshRendererComponent>();
				mrc.mesh = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Plane - Built-In");
			}

			if (ImGui::MenuItem("Quad"))
			{
				createdEntity = myContext->CreateEntity("Quad");
				auto& mrc = createdEntity.AddComponent<MeshRendererComponent>();
				mrc.mesh = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Quad - Built-In");
			}

			if (ImGui::MenuItem("Terrain"))
			{
				createdEntity = myContext->CreateEntity("Terrain");
				createdEntity.AddComponent<TerrainComponent>();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Light"))
		{
			if (ImGui::MenuItem("Sky Light"))
			{
				createdEntity = myContext->CreateEntity("Sky Light");
				createdEntity.AddComponent<SkyLightComponent>();
			}

			if (ImGui::MenuItem("Directional Light"))
			{
				createdEntity = myContext->CreateEntity("Directional Light");
				createdEntity.AddComponent<DirectionalLightComponent>();
			}

			if (ImGui::MenuItem("Spotlight"))
			{
				createdEntity = myContext->CreateEntity("Spotlight");
				createdEntity.AddComponent<SpotlightComponent>();
			}

			if (ImGui::MenuItem("Point Light"))
			{
				createdEntity = myContext->CreateEntity("Point Light");
				createdEntity.AddComponent<PointLightComponent>();
			}

			ImGui::EndMenu();
		}

		if (ImGui::MenuItem("Camera"))
		{
			createdEntity = myContext->CreateEntity("Camera");
			CameraComponent& cc = createdEntity.AddComponent<CameraComponent>();
			if (!myContext->GetPrimaryCameraEntity())
			{
				cc.primary = true;
			}
		}

		if (ImGui::MenuItem("Particle System"))
		{
			createdEntity = myContext->CreateEntity("Particle System");
			createdEntity.AddComponent<ParticleSystemComponent>();
		}

		if (ImGui::MenuItem("Text"))
		{
			createdEntity = myContext->CreateEntity("Text");
			createdEntity.AddComponent<TextRendererComponent>();
		}

		if (createdEntity)
		{
			if (myEntityCreationCallback)
			{
				myEntityCreationCallback(createdEntity);
			}

			if (entitySelected)
			{
				myContext->ParentEntity(createdEntity, selection);
				createdEntity.Transform().SetTranslation(CU::Vector3f::Zero);
			}

			SelectionManager::DeselectAll(SelectionContext::Entity);
			SelectionManager::Select(SelectionContext::Entity, createdEntity.GetUUID());
		}

		ImGui::EndMenu();
	}

	void SceneHierarchyPanel::HandleAssetDrop(Entity aParent)
	{
		auto data = ImGui::AcceptDragDropPayload("asset_payload", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
		if (data)
		{
			unsigned count = data->DataSize / sizeof(AssetHandle);

			for (unsigned i = 0; i < count; i++)
			{
				AssetHandle assetHandle = *(((AssetHandle*)data->Data) + i);
				const AssetMetadata& assetData = Project::GetEditorAssetManager()->GetMetadata(assetHandle);

				std::shared_ptr<Asset> asset = AssetManager::GetAsset<Asset>(assetHandle);

				if (!asset)
				{
					continue;
				}

				Entity newEntity;

				if (asset->GetAssetType() == AssetType::Mesh)
				{
					std::shared_ptr<Mesh> mesh = std::static_pointer_cast<Mesh>(asset);
					newEntity = myContext->InstantiateMesh(mesh);

					if (aParent)
					{
						newEntity.SetParent(aParent);
					}
				}
				else if (asset->GetAssetType() == AssetType::Texture)
				{
					newEntity = myContext->CreateEntity(assetData.filePath.stem().string());
					newEntity.AddComponent<SpriteRendererComponent>(asset->GetHandle());

					if (aParent)
					{
						newEntity.SetParent(aParent);
					}
				}
				else if (asset->GetAssetType() == AssetType::Prefab)
				{
					std::shared_ptr<Prefab> prefab = std::static_pointer_cast<Prefab>(asset);

					if (aParent)
					{
						newEntity = myContext->InstantiateChild(prefab, aParent);
					}
					else
					{
						newEntity = myContext->Instantiate(prefab);
					}
				}

				if (newEntity)
				{
					SelectionManager::DeselectAll(SelectionContext::Entity);
					SelectionManager::Select(SelectionContext::Entity, newEntity.GetUUID());
				}
			}
		}
	}
}
