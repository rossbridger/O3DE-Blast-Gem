/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzFramework/Physics/PhysicsSystem.h>
#include <Blast/BlastSystemBus.h>
#include <Blast/BlastDebug.h>
#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentBus.h>
#include <AtomLyIntegration/CommonFeatures/Mesh/MeshComponentBus.h>
#include <Components/BlastFamilyComponent.h>
#include <Atom/RPI.Public/Scene.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <Editor/EditorBlastFamilyComponent.h>

namespace Blast
{
    void EditorBlastFamilyComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<EditorBlastFamilyComponent, EditorComponentBase>()
                ->Version(5)
                ->Field("Show Chunk Data", &EditorBlastFamilyComponent::m_showChunkData)
                ->Field("Chunk Data", &EditorBlastFamilyComponent::m_chunks)
                ->Field("Blast Chunks", &EditorBlastFamilyComponent::m_blastChunksAsset)
                ->Field("BlastAsset", &EditorBlastFamilyComponent::m_blastAsset)
                ->Field("BlastMaterialAsset", &EditorBlastFamilyComponent::m_blastMaterialAsset)
                ->Field("PhysicsMaterialAsset", &EditorBlastFamilyComponent::m_physicsMaterialAsset)
                ->Field("ActorConfiguration", &EditorBlastFamilyComponent::m_actorConfiguration);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<EditorBlastFamilyComponent>(
                      "Blast Family", "Used to add a Blast family for destruction that will spawn Blast actors")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Destruction")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Box.png")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Icons/Components/Viewport/Box.png")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::CheckBox, &EditorBlastFamilyComponent::m_showChunkData,
                        "Show mesh assets", "Allows manual editing of mesh assets")
                    ->Attribute(
                        AZ::Edit::Attributes::ChangeNotify,
                        &EditorBlastFamilyComponent::OnMeshAssetsVisibilityChanged)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &EditorBlastFamilyComponent::m_chunks, "Chunks",
                        "Data needed for each Blast chunk")
                    ->Attribute(
                        AZ::Edit::Attributes::Visibility, &EditorBlastFamilyComponent::GetMeshAssetsVisibility)
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, false)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorBlastFamilyComponent::OnChunkDataChanged)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &EditorBlastFamilyComponent::m_blastChunksAsset, "Blast Chunks",
                        "Manifest override to fill out meshes and material")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorBlastFamilyComponent::OnBlastChunksAssetChanged)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &EditorBlastFamilyComponent::m_blastAsset, "Blast asset",
                        "Assigned blast asset")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &EditorBlastFamilyComponent::m_blastMaterialAsset, "Blast Material",
                        "Assigned blast material asset")
                        ->Attribute(AZ::Edit::Attributes::DefaultAsset, &EditorBlastFamilyComponent::GetDefaultBlastAssetId)
                        ->Attribute(AZ_CRC_CE("EditButton"), "")
                        ->Attribute(AZ_CRC_CE("EditDescription"), "Open in Asset Editor")
                        ->Attribute(AZ_CRC_CE("DisableEditButtonWhenNoAssetSelected"), true)

                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &EditorBlastFamilyComponent::m_physicsMaterialAsset, "Physics Material",
                        "Assigned physics material asset")
                        ->Attribute(AZ::Edit::Attributes::DefaultAsset, &EditorBlastFamilyComponent::GetDefaultPhysicsAssetId)
                        ->Attribute(AZ_CRC_CE("EditButton"), "")
                        ->Attribute(AZ_CRC_CE("EditDescription"), "Open in Asset Editor")
                        ->Attribute(AZ_CRC_CE("DisableEditButtonWhenNoAssetSelected"), true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &EditorBlastFamilyComponent::m_actorConfiguration,
                        "Actor configuration", "Configurations for actors in this family");
            }
        }
    }

    void EditorBlastFamilyComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("BlastFamilyService"));
        provided.push_back(AZ_CRC_CE("MaterialConsumerService"));
    }

    void EditorBlastFamilyComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("TransformService"));
    }

    void EditorBlastFamilyComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("EditorVisibilityService"));
    }

    void EditorBlastFamilyComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("BlastFamilyService"));
        incompatible.push_back(AZ_CRC_CE("NonUniformScaleService"));
    }

    void EditorBlastFamilyComponent::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        if (asset == m_blastAsset)
        {
            m_blastAsset = asset;
        }
        else if (asset == m_blastMaterialAsset)
        {
            m_blastMaterialAsset = asset;
        }
    }

    void EditorBlastFamilyComponent::OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        OnAssetReady(asset);
    }

    void EditorBlastFamilyComponent::Activate()
    {
        AZ_PROFILE_FUNCTION(System);

        if (m_blastAsset.GetId().IsValid())
        {
            AZ::Data::AssetBus::MultiHandler::BusConnect(m_blastAsset.GetId());
            m_blastAsset.QueueLoad();
        }

        if (m_blastMaterialAsset.GetId().IsValid())
        {
            AZ::Data::AssetBus::MultiHandler::BusConnect(m_blastMaterialAsset.GetId());
            m_blastMaterialAsset.QueueLoad();
        }

        OnChunkDataChanged();

        m_meshFeatureProcessor =
            AZ::RPI::Scene::GetFeatureProcessorForEntity<AZ::Render::MeshFeatureProcessorInterface>(GetEntityId());
        RegisterModel();

        AZ::TransformNotificationBus::Handler::BusConnect(GetEntityId());
        AZ::Render::MaterialComponentNotificationBus::Handler::BusConnect(GetEntityId());
        EditorComponentBase::Activate();
    }

    void EditorBlastFamilyComponent::Deactivate()
    {
        AZ_PROFILE_FUNCTION(System);
        EditorComponentBase::Deactivate();
        AZ::Data::AssetBus::MultiHandler::BusDisconnect();
        AZ::Render::MaterialComponentNotificationBus::Handler::BusDisconnect(GetEntityId());
        AZ::TransformNotificationBus::Handler::BusDisconnect(GetEntityId());
        UnregisterModel();
    }

    void EditorBlastFamilyComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        gameEntity->CreateComponent<BlastFamilyComponent>(
            m_blastAsset,
            m_blastMaterialAsset,
            m_physicsMaterialAsset,
            m_actorConfiguration,
            m_chunks);
    }

    AZ::Data::AssetId EditorBlastFamilyComponent::GetDefaultBlastAssetId() const
    {
        // Used for Edit Context.
        // When the blast material asset property doesn't have an asset assigned it
        // will show "(default)" to indicate that the default material will be used.
        return {};
    }

    AZ::Data::AssetId EditorBlastFamilyComponent::GetDefaultPhysicsAssetId() const
    {
        // Used for Edit Context.
        // When the physics material asset property doesn't have an asset assigned it
        // will show "(default)" to indicate that the default material will be used.
        return {};
    }

    void EditorBlastFamilyComponent::OnBlastChunksAssetChanged()
    {
        if (!m_blastChunksAsset.GetId().IsValid())
        {
            return;
        }

        using namespace AZ::Data;
        const AssetId blastAssetId = m_blastChunksAsset.GetId();
        m_blastChunksAsset = AssetManager::Instance().GetAsset<BlastChunksAsset>(blastAssetId, AssetLoadBehavior::QueueLoad);
        m_blastChunksAsset.BlockUntilLoadComplete();

        if (!m_blastChunksAsset.Get() || m_blastChunksAsset.Get()->GetChunks().empty())
        {
            AZ_Warning("blast", false, "Blast Chunk Asset does not contain any models.")
                return;
        }

        // load up the new mesh list
        m_chunks = m_blastChunksAsset.Get()->GetChunks();

        UnregisterModel();
        RegisterModel();

        using namespace AzToolsFramework;
        ToolsApplicationEvents::Bus::Broadcast(&ToolsApplicationEvents::InvalidatePropertyDisplay, Refresh_EntireTree);
    }

    void EditorBlastFamilyComponent::OnChunkDataChanged()
    {
        for (auto& chunk : m_chunks)
        {
            chunk.m_meshAsset.SetAutoLoadBehavior(AZ::Data::AssetLoadBehavior::QueueLoad);
            chunk.m_collisionMeshAsset.SetAutoLoadBehavior(AZ::Data::AssetLoadBehavior::QueueLoad);
        }
        UnregisterModel();
        RegisterModel();
    }

    AZ::Crc32 EditorBlastFamilyComponent::GetMeshAssetsVisibility() const
    {
        return m_showChunkData ? AZ::Edit::PropertyVisibility::Show : AZ::Edit::PropertyVisibility::Hide;
    }

    void EditorBlastFamilyComponent::OnMeshAssetsVisibilityChanged()
    {
        AzToolsFramework::ToolsApplicationEvents::Bus::Broadcast(
            &AzToolsFramework::ToolsApplicationEvents::InvalidatePropertyDisplay, AzToolsFramework::Refresh_EntireTree);
    }

    void EditorBlastFamilyComponent::HandleModelChange(AZ::Data::Instance<AZ::RPI::Model> model)
    {
        if (model)
        {
            AZ::Render::MeshComponentNotificationBus::Event(
                GetEntityId(), &AZ::Render::MeshComponentNotificationBus::Events::OnModelReady, model->GetModelAsset(),
                model);
            AZ::Render::MaterialConsumerNotificationBus::Event(
                GetEntityId(), &AZ::Render::MaterialConsumerNotificationBus::Events::OnMaterialAssignmentSlotsChanged);
        }
    }

    void EditorBlastFamilyComponent::RegisterModel()
    {
        if (m_meshFeatureProcessor && !m_chunks.empty() && m_chunks[0].m_meshAsset.GetId().IsValid())
        {
            AZ::Render::MaterialAssignmentMap materials;
            AZ::Render::MaterialComponentRequestBus::EventResult(
                materials, GetEntityId(), &AZ::Render::MaterialComponentRequests::GetMaterialMap);

            m_meshFeatureProcessor->ReleaseMesh(m_meshHandle);
            AZ::Render::MeshHandleDescriptor meshHandleDescriptor {
                m_chunks[0].m_meshAsset,
                AZ::Render::ConvertToCustomMaterialMap(materials)
            };
            meshHandleDescriptor.m_modelChangedEventHandler = m_changeEventHandler;
            m_meshHandle = m_meshFeatureProcessor->AcquireMesh(meshHandleDescriptor);

            HandleModelChange(m_meshFeatureProcessor->GetModel(m_meshHandle));

            AZ::Transform transform = AZ::Transform::Identity();
            AZ::TransformBus::EventResult(transform, GetEntityId(), &AZ::TransformInterface::GetWorldTM);
            m_meshFeatureProcessor->SetTransform(m_meshHandle, transform);
        }
    }

    void EditorBlastFamilyComponent::UnregisterModel()
    {
        if (m_meshFeatureProcessor)
        {
            m_meshFeatureProcessor->ReleaseMesh(m_meshHandle);
        }
    }

    const AZ::Data::Asset<BlastChunksAsset>& EditorBlastFamilyComponent::GetBlastChunksAsset() const
    {
        return m_blastChunksAsset;
    }

    const AZStd::vector<BlastChunkData>& EditorBlastFamilyComponent::GetChunks() const
    {
        return m_chunks;
    }

    void EditorBlastFamilyComponent::OnMaterialsUpdated([
        [maybe_unused]] const AZ::Render::MaterialAssignmentMap& materials)
    {
        UnregisterModel();
        RegisterModel();
    }

    void EditorBlastFamilyComponent::OnTransformChanged(
        [[maybe_unused]] const AZ::Transform& local, const AZ::Transform& world)
    {
        if (m_meshFeatureProcessor)
        {
            m_meshFeatureProcessor->SetTransform(m_meshHandle, world);
        }
    }
} // namespace Blast
