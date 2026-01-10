/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <Actor/BlastActorDesc.h>
#include <Blast/BlastActor.h>

namespace Blast
{
    class BlastFamily;

    //! Interface that creates new actors and calculates necessary parts of actor description

    class BlastActorFactory
    {
    public:
        BlastActor* CreateActor(const BlastActorDesc& desc);
        void DestroyActor(BlastActor* actor);

        AZStd::vector<uint32_t> CalculateVisibleChunks(
            const BlastFamily& blastFamily, const Nv::Blast::TkActor& tkActor) const;
        bool CalculateIsLeafChunk(
            const Nv::Blast::TkActor& tkActor, const AZStd::vector<uint32_t>& chunkIndices) const;
        bool CalculateIsStatic(
            const BlastFamily& blastFamily, const Nv::Blast::TkActor& tkActor,
            const AZStd::vector<uint32_t>& chunkIndices) const;

    private:
        bool SupportGraphHasStaticActor(const BlastFamily& blastFamily, const Nv::Blast::TkActor& tkActor) const;
        bool VisibleChunksHasStaticActor(
            const BlastFamily& blastFamily, const AZStd::vector<uint32_t>& chunkIndices) const;
    };
} // namespace Blast
