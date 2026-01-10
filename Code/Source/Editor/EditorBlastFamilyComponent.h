/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <Asset/BlastAsset.h>
#include <Asset/BlastChunksAsset.h>
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <Atom/RPI.Public/Model/Model.h>
#include <AtomLyIntegration/CommonFeatures/Material/MaterialComponentBus.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TransformBus.h>
#include <AzFramework/Physics/Material/PhysicsMaterialAsset.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <Blast/BlastActorConfiguration.h>
#include <Material/BlastMaterialAsset.h>

namespace Blast
{
    class EditorBlastFamilyComponent
        : public AzToolsFramework::Components::EditorComponentBase
        , private AZ::Data::AssetBus::MultiHandler
        , private AZ::TransformNotificationBus::Handler
        , private AZ::Render::MaterialComponentNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(
            EditorBlastFamilyComponent, "{ECB1689A-2B65-44D1-9227-9E62962A7FF7}",
            AzToolsFramework::Components::EditorComponentBase);

        EditorBlastFamilyComponent() = default;
        virtual ~EditorBlastFamilyComponent() = default;

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        void BuildGameEntity(AZ::Entity* gameEntity) override;

    private:
        void OnBlastChunksAssetChanged();
        void OnChunkDataChanged();
        AZ::Crc32 GetMeshAssetsVisibility() const;
        void OnMeshAssetsVisibilityChanged();

        void HandleModelChange(AZ::Data::Instance<AZ::RPI::Model> model);
        void RegisterModel();
        void UnregisterModel();

        const AZ::Data::Asset<BlastChunksAsset>& GetBlastChunksAsset() const;
        const AZStd::vector<BlastChunkData>& GetChunks() const;

        void OnMaterialsUpdated(const AZ::Render::MaterialAssignmentMap& materials) override;
        void OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world) override;

        AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor = nullptr;
        AZ::Render::MeshFeatureProcessorInterface::MeshHandle m_meshHandle;
        using ModelChangedEvent = AZ::Event<const AZ::Data::Instance<AZ::RPI::Model>&>;
        ModelChangedEvent::Handler m_changeEventHandler {
            [&](AZ::Data::Instance<AZ::RPI::Model> model) {
                HandleModelChange(model);
            } 
        };

        // AssetBus
        void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;

        AZ::Data::AssetId GetDefaultBlastAssetId() const;
        AZ::Data::AssetId GetDefaultPhysicsAssetId() const;

        // Configurations
        bool m_showChunkData = false;
        AZ::Data::Asset<BlastChunksAsset> m_blastChunksAsset;
        AZStd::vector<BlastChunkData> m_chunks;

        AZ::Data::Asset<BlastAsset> m_blastAsset;
        AZ::Data::Asset<MaterialAsset> m_blastMaterialAsset;
        AZ::Data::Asset<Physics::MaterialAsset> m_physicsMaterialAsset;

        BlastActorConfiguration m_actorConfiguration;
    };
} // namespace Blast
