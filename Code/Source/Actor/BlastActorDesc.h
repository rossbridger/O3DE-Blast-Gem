/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/std/containers/vector.h>
#include <AzFramework/Physics/Material/PhysicsMaterialId.h>
#include <AzFramework/Physics/Configuration/RigidBodyConfiguration.h>

namespace Nv::Blast
{
    class TkActor;
} // namespace Nv::Blast

namespace Blast
{
    class BlastFamily;

    //! Data required to create a BlastActor.
    struct BlastActorDesc
    {
        BlastFamily* m_family;
        Nv::Blast::TkActor* m_tkActor;
        Physics::MaterialId m_physicsMaterialId;
        AZStd::unique_ptr<AzPhysics::SimulatedBodyConfiguration> m_bodyConfiguration; //!< Either rigid dynamic or rigid static
        AZStd::vector<uint32_t> m_chunkIndices; //!< Chunks that are going to simulate this actor.
        bool m_isStatic = false; //!< Denotes whether actor should be simulated by a static or dynamic rigid body.
        bool m_isLeafChunk = false; //!< Denotes whether this actor represented by a single leaf chunk.
        float m_scale = 1.0f; //!< Uniform scale applied to the actor.
    };
} // namespace Blast
