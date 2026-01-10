/*
 * Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Asset/AssetSerializer.h>
#include <Asset/BlastChunksAsset.h>

namespace Blast
{
    void BlastChunksAsset::SetChunks(const AZStd::vector<BlastChunkData>& chunks)
    {
        m_chunks = chunks;
    }

    const AZStd::vector<BlastChunkData>& BlastChunksAsset::GetChunks() const
    {
        return m_chunks;
    }

    void BlastChunksAsset::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<BlastChunksAsset>()
                ->Version(1)
                ->Attribute(AZ::Edit::Attributes::EnableForAssetEditor, true)
                ->Field("chunks", &BlastChunksAsset::m_chunks);
            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<BlastChunksAsset>("Blast Chunks Asset", "Blast Chunks Asset")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BlastChunksAsset::m_chunks, "Chunks", "Chunks");
            }
        }
    }

} // namespace Blast
