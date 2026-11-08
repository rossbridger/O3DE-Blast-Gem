/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Math/Transform.h>
#include <AzFramework/Physics/PhysicsSystem.h>
#include <AzFramework/Physics/Shape.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/Utils.h>
#include <AzFramework/Physics/Material/PhysicsMaterialManager.h>
#include <AzFramework/Physics/Configuration/StaticRigidBodyConfiguration.h>
#include <Actor/BlastActor.h>
#include <Family/BlastFamily.h>
#include <NvBlastTkActor.h>
#include <NvBlastTkAsset.h>
#include <NvBlastTypes.h>

namespace Blast
{
    BlastActor::BlastActor(const BlastActorDesc& desc)
        : m_family(*desc.m_family)
        , m_tkActor(*desc.m_tkActor)
        , m_chunkIndices(desc.m_chunkIndices)
        , m_isStatic(desc.m_isStatic)
    {
        // Store pointer to ourselves in the blast toolkit actor's userData
        m_tkActor.userData = this;
        m_entity = AZStd::make_shared<AZ::Entity>();
        Spawn(desc);
    }

    BlastActor::~BlastActor()
    {
        BlastActorRequestBus::Handler::BusDisconnect();
        m_tkActor.userData = nullptr;
    }

    void BlastActor::Spawn(const BlastActorDesc& desc)
    {
        AzPhysics::SimulatedBodyConfiguration* bodyConfiguration = desc.m_bodyConfiguration.get();
        bodyConfiguration->m_entityId = m_entity->GetId();
        // Get physics material from id
        AZStd::shared_ptr<Physics::Material> physicsMaterial =
            AZ::Interface<Physics::MaterialManager>::Get()->GetMaterial(desc.m_physicsMaterialId);
        if (!physicsMaterial)
        {
            physicsMaterial = AZ::Interface<Physics::MaterialManager>::Get()->GetDefaultMaterial();
        }

        // Add shapes for each of the visible chunks
        auto shapes = AddShapes(m_chunkIndices, m_family.GetTkAsset(), physicsMaterial->GetMaterialAsset(), desc.m_scale);

        auto transform = AZ::Transform::CreateFromQuaternionAndTranslation(
            bodyConfiguration->m_orientation, bodyConfiguration->m_position);
        transform.MultiplyByUniformScale(desc.m_scale);

        bodyConfiguration->m_customUserData = this;
        if (m_isStatic)
        {
            auto m_staticBodyConfiguration = azrtti_cast<AzPhysics::StaticRigidBodyConfiguration*>(bodyConfiguration);
            m_staticBodyConfiguration->m_colliderAndShapeData = AZStd::move(shapes);
        }
        else
        {
            auto m_dynamicBodyConfiguration = azrtti_cast<AzPhysics::RigidBodyConfiguration*>(bodyConfiguration);
            m_dynamicBodyConfiguration->m_colliderAndShapeData = AZStd::move(shapes);
        }

        auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();
        auto sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
        AzPhysics::SimulatedBodyHandle simulatedBodyHandle = sceneInterface->AddSimulatedBody(sceneHandle, bodyConfiguration);
        m_simulatedBody = sceneInterface->GetSimulatedBodyFromHandle(sceneHandle, simulatedBodyHandle);

        BlastActorRequestBus::Handler::BusConnect(m_entity->GetId());
    }

    AZStd::vector<AZStd::shared_ptr<Physics::Shape>> BlastActor::AddShapes(
        const AZStd::vector<uint32_t>& chunkIndices, const Nv::Blast::TkAsset& asset,
        const AZ::Data::Asset<Physics::MaterialAsset>& physicsMaterialAsset, float scale)
    {
        const NvBlastChunk* chunks = asset.getChunks();
        const uint32_t chunkCount = asset.getChunkCount();

        AZStd::vector<AZStd::shared_ptr<Physics::Shape>> shapes;

        AZ_Assert(chunks, "Received asset with a null chunk array.");
        if (!chunks)
        {
            return shapes;
        }

        for (uint32_t chunkId : chunkIndices)
        {
            AZ_Assert(chunkId < chunkCount, "Out of bounds access to the chunks.");
            if (chunkId >= chunkCount)
            {
                continue;
            }

            AZ::Transform transform = AZ::Transform::CreateIdentity(); // TODO: Get transform from chunk
            auto colliderConfiguration = CalculateColliderConfiguration(transform, physicsMaterialAsset);

            Physics::PhysicsAssetShapeConfiguration shapeConfiguration;
            shapeConfiguration.m_asset = m_family.GetChunkDataContainer()->GetCollisionMeshAsset(chunkId);
            shapeConfiguration.m_scale = AZ::Vector3(scale);

            AZStd::shared_ptr<Physics::Shape> shape = AZ::Interface<Physics::SystemRequests>::Get()->CreateShape(
                colliderConfiguration, shapeConfiguration);

            AZ_Assert(shape, "Failed to create Shape for BlastActor");

            shapes.push_back(shape);
        }
        return shapes;
    }

    Physics::ColliderConfiguration BlastActor::CalculateColliderConfiguration(
        const AZ::Transform& transform, const AZ::Data::Asset<Physics::MaterialAsset>& physicsMaterialAsset)
    {
        auto& actorConfiguration = m_family.GetActorConfiguration();
        Physics::ColliderConfiguration colliderConfiguration;
        colliderConfiguration.m_position = transform.GetTranslation();
        colliderConfiguration.m_rotation = transform.GetRotation();
        colliderConfiguration.m_isExclusive = true;
        colliderConfiguration.m_materialSlots.SetMaterialAsset(0, physicsMaterialAsset);
        colliderConfiguration.m_collisionGroupId = actorConfiguration.m_collisionGroupId;
        colliderConfiguration.m_collisionLayer = actorConfiguration.m_collisionLayer;
        colliderConfiguration.m_isInSceneQueries = actorConfiguration.m_isInSceneQueries;
        colliderConfiguration.m_isSimulated = actorConfiguration.m_isSimulated;
        colliderConfiguration.m_tag = actorConfiguration.m_tag;

        return colliderConfiguration;
    }

    AZ::Transform BlastActor::GetTransform() const
    {
        return GetSimulatedBody()->GetTransform();
    }

    const BlastFamily& BlastActor::GetFamily() const
    {
        return m_family;
    }

    const Nv::Blast::TkActor& BlastActor::GetTkActor() const
    {
        return m_tkActor;
    }

    AzPhysics::SimulatedBody* BlastActor::GetSimulatedBody()
    {
        return m_simulatedBody;
    }

    const AzPhysics::SimulatedBody* BlastActor::GetSimulatedBody() const
    {
        return m_simulatedBody;
    }

    const AZStd::vector<uint32_t>& BlastActor::GetChunkIndices() const
    {
        return m_chunkIndices;
    }

    bool BlastActor::IsStatic() const
    {
        return m_isStatic;
    }

    const AZ::Entity* BlastActor::GetEntity() const
    {
        return m_entity.get();
    }

    void BlastActor::Damage(const NvBlastDamageProgram& program, NvBlastExtProgramParams* programParams)
    {
        m_tkActor.damage(program, programParams);
    }
} // namespace Blast
