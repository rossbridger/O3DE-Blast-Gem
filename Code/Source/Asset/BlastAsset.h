/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/std/parallel/mutex.h>

#include <NvBlastTkAsset.h>

class NvBlastExtDamageAccelerator;

namespace Blast
{
    //! An AZ::Asset wrapper around Blast::ExtPxAsset and NvBlastExtDamageAccelerator.
    class BlastAsset : public AZ::Data::AssetData
    {
    public:
        AZ_RTTI(BlastAsset, "{5BBFE65A-B2F7-4752-A12A-8B44A07D88EB}", AZ::Data::AssetData);
        AZ_CLASS_ALLOCATOR(BlastAsset, AZ::SystemAllocator, 0);

        BlastAsset(Nv::Blast::TkAsset* tkAsset = nullptr);

        static void Reflect(AZ::ReflectContext* context);

        bool LoadFromBuffer(void* buffer, size_t bytesSize);

        const Nv::Blast::TkAsset* GetTkAsset() const
        {
            return m_tkAsset.get();
        }

        NvBlastExtDamageAccelerator* GetAccelerator() const
        {
            return m_damageAccelerator.get();
        }

        float GetBondHealthMax() const
        {
            return m_bondHealthMax;
        }

    private:
        static AZStd::mutex s_assetCreationMutex;

        AZStd::unique_ptr<Nv::Blast::TkAsset> m_tkAsset;
        AZStd::unique_ptr<NvBlastExtDamageAccelerator> m_damageAccelerator;

        float m_bondHealthMax;
    };
} // namespace Blast
