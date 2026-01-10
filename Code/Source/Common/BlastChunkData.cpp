/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Asset/AssetSerializer.h>
#include <Common/BlastChunkData.h>


namespace Blast
{
    void BlastChunkData::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<BlastChunkData>()
                ->Version(1)
                ->Field("MeshAsset", &BlastChunkData::m_meshAsset)
                ->Field("CollisionMeshAsset", &BlastChunkData::m_collisionMeshAsset)
                ->Field("Mass", &BlastChunkData::m_mass)
                ->Field("IsStatic", &BlastChunkData::m_isStatic)
                ;
            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<BlastChunkData>("Blast Chunk Data", "Blast Chunk Data")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BlastChunkData::m_meshAsset, "Model Assets", "Model Assets")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BlastChunkData::m_collisionMeshAsset, "Collision Mesh Assets", "Collision Mesh Assets")
                    ->Attribute(AZ_CRC_CE("SupportedAssetTypes"), AZStd::vector<AZ::Data::AssetType> { AZ::Data::AssetType{"{7A2871B9-5EAB-4DE0-A901-B0D2C6920DDB}"} })
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BlastChunkData::m_mass, "Mass", "Mass")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BlastChunkData::m_isStatic, "Is Static", "Is Static")
                    ;
            }
        }
    }
    BlastChunkDataContainer::BlastChunkDataContainer(
        const AZStd::vector<BlastChunkData>& chunks)
        : m_chunks(chunks)
    {
    }

    const AZ::Data::Asset<AZ::RPI::ModelAsset>& BlastChunkDataContainer::GetMeshAsset(size_t index) const
    {
        if (index < m_chunks.size())
        {
            return m_chunks[index].m_meshAsset;
        }

        AZ_Warning(
            "Blast", false, "Tried to get mesh asset at index %zu outside of Blast chunk array.", index);
        static AZ::Data::Asset<AZ::RPI::ModelAsset> invalidAsset;
        return invalidAsset;
    }

    const AZ::Data::Asset<AZ::Data::AssetData>& BlastChunkDataContainer::GetCollisionMeshAsset(size_t index) const
    {
        if (index < m_chunks.size())
        {
            return m_chunks[index].m_collisionMeshAsset;
        }

        AZ_Warning(
            "Blast", false, "Tried to get collision mesh asset at index %zu outside of Blast chunk array.", index);
        static AZ::Data::Asset<AZ::Data::AssetData> invalidAsset;
        return invalidAsset;
    }

    const float BlastChunkDataContainer::GetMass(size_t index) const
    {
        if (index < m_chunks.size())
        {
            return m_chunks[index].m_mass;
        }

        AZ_Warning(
            "Blast", false, "Tried to get mass at index %zu outside of Blast chunk array.", index);
        return 0.0f;
    }

    const bool BlastChunkDataContainer::GetIsStatic(size_t index) const
    {
        if (index < m_chunks.size())
        {
            return m_chunks[index].m_isStatic;
        }

        AZ_Warning(
            "Blast", false, "Tried to get static flag at index %zu outside of Blast chunk array.", index);

        return false;
    }

} // namespace Blast
