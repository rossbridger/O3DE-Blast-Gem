/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/std/containers/vector.h>
#include <AzCore/Asset/AssetCommon.h>
#include <Atom/RPI.Reflect/Model/ModelAsset.h>
#include <AzFramework/Physics/ShapeConfiguration.h>

namespace Blast
{
    struct BlastChunkData
    {
        AZ_TYPE_INFO(BlastChunkData, "{4406DF79-841A-45B4-8FCA-153144DD934E}");
        static void Reflect(AZ::ReflectContext* context);

        AZ::Data::Asset<AZ::RPI::ModelAsset> m_meshAsset;
        AZ::Data::Asset<AZ::Data::AssetData> m_collisionMeshAsset;
        float m_mass = 1.0f;
        bool m_isStatic = false;
    };

    //! Component that stores meshes for the blast family to use during game time.
    class BlastChunkDataContainer
    {
    public:
        BlastChunkDataContainer(const AZStd::vector<BlastChunkData>& chunks);

        const AZ::Data::Asset<AZ::RPI::ModelAsset>& GetMeshAsset(size_t index) const;
        const AZ::Data::Asset<AZ::Data::AssetData>& GetCollisionMeshAsset(size_t index) const;
        const float GetMass(size_t index) const;
        const bool GetIsStatic(size_t index) const;

    private:
        const AZStd::vector<BlastChunkData>& m_chunks;
    };
} // namespace Blast
