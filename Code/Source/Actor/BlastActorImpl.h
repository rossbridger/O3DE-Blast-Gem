/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <Blast/BlastActor.h>
#include <Actor/BlastActorDesc.h>
#include <AzFramework/Physics/Material/PhysicsMaterialAsset.h>

namespace Nv::Blast
{
    class TkAsset;
} // namespace Nv::Blast

namespace Blast
{
    //! Provides the glue between Blast actors and the PhysX actors they manipulate.
    class BlastActorImpl : public BlastActor
    {
    public:
        AZ_CLASS_ALLOCATOR(BlastActorImpl, AZ::SystemAllocator, 0);

        BlastActorImpl(const BlastActorDesc& desc);
        ~BlastActorImpl();

        void Damage(const NvBlastDamageProgram& program, NvBlastExtProgramParams* programParams) override;

        const BlastFamily& GetFamily() const override;
        const Nv::Blast::TkActor& GetTkActor() const override;
        AZ::Transform GetTransform() const override;

        const AZ::Entity* GetEntity() const override;
        const AZStd::vector<uint32_t>& GetChunkIndices() const override;
        bool IsStatic() const override;

        AzPhysics::SimulatedBody* GetSimulatedBody() override;
        const AzPhysics::SimulatedBody* GetSimulatedBody() const override;

    private:
        void Spawn(const BlastActorDesc& desc);

        virtual Physics::ColliderConfiguration CalculateColliderConfiguration(
            const AZ::Transform& transform, const AZ::Data::Asset<Physics::MaterialAsset>& physicsMaterialAsset);

        AZStd::vector<AZStd::shared_ptr<Physics::Shape>> AddShapes(
            const AZStd::vector<uint32_t>& chunkIndices, const Nv::Blast::TkAsset& asset,
            const AZ::Data::Asset<Physics::MaterialAsset>& physicsMaterialAsset, float scale);

        const BlastFamily& m_family;
        Nv::Blast::TkActor& m_tkActor;

        AZStd::shared_ptr<AZ::Entity> m_entity;
        AZStd::vector<uint32_t> m_chunkIndices;
        bool m_isLeafChunk;
        bool m_isStatic;

        AzPhysics::SimulatedBody* m_simulatedBody = nullptr;
    };
} // namespace Blast
