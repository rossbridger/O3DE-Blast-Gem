/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Actor/BlastActorFactory.h>

#include <Actor/BlastActor.h>
#include <Common/BlastChunkData.h>
#include <Family/BlastFamily.h>
#include <NvBlastTkActor.h>
#include <NvBlastTkAsset.h>

namespace Blast
{
    BlastActor* BlastActorFactory::CreateActor(const BlastActorDesc& desc)
    {
        auto actor = aznew BlastActor(desc);
        return actor;
    }

    void BlastActorFactory::DestroyActor(BlastActor* actor)
    {
        delete actor;
    }

    AZStd::vector<uint32_t> BlastActorFactory::CalculateVisibleChunks(
        [[maybe_unused]] const BlastFamily& blastFamily, const Nv::Blast::TkActor& tkActor) const
    {
        const Nv::Blast::TkAsset* tkAsset = tkActor.getAsset();
        AZ_Assert(tkAsset, "Invalid TkAsset on TkActor.");
        if (!tkAsset)
        {
            return {};
        }

        const NvBlastChunk* chunks = tkAsset->getChunks();
        const uint32_t chunkCount = tkAsset->getChunkCount();

        AZ_Assert(chunks, "TkActor's asset has a null chunk array.");

        if (!chunks)
        {
            return {};
        }

        AZStd::vector<uint32_t> chunkIndicesScratch;
        chunkIndicesScratch.resize_no_construct(tkActor.getVisibleChunkCount());
        tkActor.getVisibleChunkIndices(chunkIndicesScratch.data(), static_cast<uint32_t>(chunkIndicesScratch.size()));

        AZStd::vector<uint32_t> chunkIndices;
        // Filter the visible chunks to ensure they have subchunks
        chunkIndices.reserve(chunkIndicesScratch.size());
        for (uint32_t chunkIndex : chunkIndicesScratch)
        {
            AZ_Assert(chunkIndex < chunkCount, "Out of bounds access to the TkAsset's Chunks.");
            if (chunkIndex < chunkCount)
            {
                const NvBlastChunk& chunk = chunks[chunkIndex];
                if (chunk.childIndexStop > chunk.firstChildIndex)
                {
                    chunkIndices.push_back(chunkIndex);
                }
            }
        }
        return chunkIndices;
    }

    bool BlastActorFactory::CalculateIsLeafChunk(
        const Nv::Blast::TkActor& tkActor, const AZStd::vector<uint32_t>& chunkIndices) const
    {
        const Nv::Blast::TkAsset* tkAsset = tkActor.getAsset();
        AZ_Assert(tkAsset, "Invalid TkAsset on TkActor.");
        if (!tkAsset)
        {
            return false;
        }

        const uint32_t nodeCount = tkActor.getGraphNodeCount();
        const uint32_t chunkCount = tkAsset->getChunkCount();
        const NvBlastChunk* chunks = tkAsset->getChunks();

        AZStd::vector<uint32_t> chunkIndicesScratch;
        chunkIndicesScratch.resize_no_construct(tkActor.getVisibleChunkCount());
        tkActor.getVisibleChunkIndices(chunkIndicesScratch.data(), static_cast<uint32_t>(chunkIndicesScratch.size()));

        bool isLeafChunk = false;
        // Single lower-support chunk actors might be leaf actors - if so, set our leaf chunk flag for use in filter
        // data
        if (nodeCount <= 1 && !chunkIndicesScratch.empty() && !chunkIndices.empty())
        {
            const uint32_t chunkIndex = chunkIndices[0];
            AZ_Assert(chunkIndex < chunkCount, "Out of bounds access to the ExtPxAsset's PxChunks.");
            if (chunkIndex < chunkCount)
            {
                const NvBlastChunk& chunk = chunks[chunkIndex];
                // Mark as a leaf chunk if it has no children
                isLeafChunk = chunk.firstChildIndex == chunk.childIndexStop;
            }
        }
        return isLeafChunk;
    }

    bool BlastActorFactory::CalculateIsStatic(
        const BlastFamily& blastFamily, const Nv::Blast::TkActor& tkActor,
        const AZStd::vector<uint32_t>& chunkIndices) const
    {
        return tkActor.isBoundToWorld() || SupportGraphHasStaticActor(blastFamily, tkActor) ||
            VisibleChunksHasStaticActor(blastFamily, chunkIndices);
    }

    bool BlastActorFactory::SupportGraphHasStaticActor(
        [[maybe_unused]] const BlastFamily& blastFamily, [[maybe_unused]] const Nv::Blast::TkActor& tkActor) const
    {
        const uint32_t nodeCount = tkActor.getGraphNodeCount();
        const auto chunkContainer = blastFamily.GetChunkDataContainer();
        const Nv::Blast::TkAsset* tkAsset = tkActor.getAsset();

        if (nodeCount == 0 || !chunkContainer || !tkAsset)
        {
            AZ_Assert(chunkContainer, "BlastFamily's asset has a null chunk array.");
            AZ_Assert(tkAsset, "Invalid TkAsset on TkActor.");
            return false;
        }
        bool staticFound = false;

        AZStd::vector<uint32_t> graphNodeIndices;
        graphNodeIndices.resize_no_construct(nodeCount);
        tkActor.getGraphNodeIndices(graphNodeIndices.data(), static_cast<uint32_t>(graphNodeIndices.size()));
        const NvBlastSupportGraph graph = tkAsset->getGraph();
        const uint32_t chunkCount = blastFamily.GetTkAsset().getChunkCount();

        for (uint32_t graphNodeIndex : graphNodeIndices)
        {
            if (graphNodeIndex >= graph.nodeCount)
            {
                AZ_Assert(false, "Out of bounds access to NvBlastSupportGraph.");
                continue;
            }

            const uint32_t chunkIndex = graph.chunkIndices[graphNodeIndex];

            if (chunkIndex >= chunkCount)
            {
                AZ_Assert(false, "Out of bounds access to BlastFamily asset's Chunks.");
                continue;
            }

            if (blastFamily.GetChunkDataContainer()->GetIsStatic(chunkIndex))
            {
                staticFound = true;
                break;
            }
        }

        return staticFound;
    }

    bool BlastActorFactory::VisibleChunksHasStaticActor(const BlastFamily& blastFamily, const AZStd::vector<uint32_t>& chunkIndices) const
    {
        const auto chunkContainer = blastFamily.GetChunkDataContainer();
        if (!chunkContainer)
        {
            AZ_Assert(false, "Family has null chunk data container.");
            return false;
        }

        bool staticFound = false;
        for (const uint32_t chunkIndex : chunkIndices)
        {
            if (chunkContainer->GetIsStatic(chunkIndex))
            {
                staticFound = true;
                break;
            }
        }
        return staticFound;
    }
} // namespace Blast
