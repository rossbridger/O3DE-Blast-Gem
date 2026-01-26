/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/std/containers/unordered_map.h>
#include <Asset/BlastAsset.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Asset/AssetCommon.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBodyEvents.h>
#include <AzFramework/Physics/Material/PhysicsMaterialAsset.h>
#include <AzFramework/Physics/Components/SimulatedBodyComponentBus.h>
#include <Blast/BlastDebug.h>
#include <Blast/BlastFamilyComponentBus.h>
#include <Material/BlastMaterialAsset.h>
#include <Common/BlastInterfaces.h>
#include <Common/BlastChunkData.h>
#include <Family/ActorRenderManager.h>
#include <Family/BlastFamily.h>
#include <Family/DamageManager.h>
#include <NvBlastExtStressSolver.h>
#include <AzFramework/Physics/Common/PhysicsTypes.h>
#include <AzFramework/Physics/Common/PhysicsSceneQueries.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>

namespace AzPhysics
{
    struct CollisionEvent;
}

namespace Physics
{
    class Material;
}

namespace Blast
{
    //! Component that handles simulation of the Blast family.
    class BlastFamilyComponent
        : public AZ::Component
        , protected BlastFamilyDamageRequestBus::MultiHandler
        , protected BlastFamilyComponentRequestBus::Handler
        , protected BlastListener
        , protected AZ::Data::AssetBus::MultiHandler
    {
    public:
        AZ_COMPONENT(BlastFamilyComponent, "{88ECE087-C88A-4A83-A83C-477BA9C13221}", AZ::Component);

        BlastFamilyComponent(
            AZ::Data::Asset<BlastAsset> blastAsset,
            AZ::Data::Asset<MaterialAsset> blastMaterialAsset,
            AZ::Data::Asset<Physics::MaterialAsset> physicsMaterialAsset,
            const BlastActorConfiguration& actorConfiguration,
            const AZStd::vector<BlastChunkData>& chunks);

        BlastFamilyComponent() = default;
        virtual ~BlastFamilyComponent() = default;
        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component overrides ...
        void Init() override;
        void Activate() override;
        void Deactivate() override;

    protected:
        // Lifecycle
        void Spawn();
        void Despawn();

        // BlastFamilyDamageRequestBus overrides ...
        AZ::EntityId GetFamilyId() override;
        void RadialDamage(const AZ::Vector3& position, float minRadius, float maxRadius, float damage) override;
        void CapsuleDamage(
            const AZ::Vector3& position0, const AZ::Vector3& position1, float minRadius, float maxRadius,
            float damage) override;
        void ShearDamage(
            const AZ::Vector3& position, const AZ::Vector3& normal, float minRadius, float maxRadius,
            float damage) override;
        void TriangleDamage(
            const AZ::Vector3& position0, const AZ::Vector3& position1, const AZ::Vector3& position2,
            float damage) override;
        void ImpactSpreadDamage(const AZ::Vector3& position, float minRadius, float maxRadius, float damage) override;
        void StressDamage(const AZ::Vector3& position, const AZ::Vector3& force) override;
        void StressDamage(const AZ::EntityId actor, const AZ::Vector3& position, const AZ::Vector3& force) override;
        void DestroyActor() override;

        // BlastFamilyComponentRequestBus overrides ...
        AZStd::vector<AZ::EntityId> GetActors() const override;

        void FillDebugRenderBuffer(DebugRenderBuffer& debugRenderBuffer, DebugRenderMode debugRenderMode) override;
        void ApplyStressDamage() override;
        void SyncMeshes() override;

        // AZ::Data::AssetBus overrides ...
        void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetError(AZ::Data::Asset<AZ::Data::AssetData> asset) override;

    private:
        // BlastListener overrides ...
        // These methods trigger notifications on BlastFamilyComponentNotificationBus.
        void OnActorCreated(const BlastFamily& family, const BlastActor& actor) override;
        void OnActorDestroyed(const BlastFamily& family, const BlastActor& actor) override;

        // Dispatched when two shapes start colliding.
        void OnCollisionBegin(const AzPhysics::CollisionEvent& collisionEvent);

        void StressDamage(const BlastActor& blastActor, const AZ::Vector3& position, const AZ::Vector3& force);

        // Logic processors
        AZStd::unique_ptr<DamageManager> m_damageManager;
        AZStd::unique_ptr<ActorRenderManager> m_actorRenderManager;
        AZStd::unique_ptr<Nv::Blast::ExtStressSolver> m_solver;
        AZStd::unique_ptr<BlastFamily> m_family;

        // Blast material instance
        AZStd::unique_ptr<Material> m_blastMaterial;

        // Physics material instance
        AZStd::shared_ptr<Physics::Material> m_physicsMaterial;

        // Dependencies
        AZStd::unique_ptr<BlastChunkDataContainer> m_chunkContainer;

        // Configurations
        AZ::Data::Asset<BlastAsset> m_blastAsset;
        AZ::Data::Asset<MaterialAsset> m_blastMaterialAsset;
        AZ::Data::Asset<Physics::MaterialAsset> m_physicsMaterialAsset;

        AZStd::vector<BlastChunkData> m_chunks;
        const BlastActorConfiguration m_actorConfiguration{};

        bool m_isSpawned = false;
        DebugRenderMode m_debugRenderMode;

        using CollisionHandlersMap = AZStd::unordered_map<AZ::EntityId, AzPhysics::SimulatedBodyEvents::OnCollisionBegin::Handler>;
        using CollisionHandlersMapItr = CollisionHandlersMap::iterator;
        CollisionHandlersMap m_collisionHandlers;
    };
} // namespace Blast
