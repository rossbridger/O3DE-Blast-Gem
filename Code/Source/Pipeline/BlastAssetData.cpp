#include <NvBlast.h>
#include <NvBlastGlobals.h>
#include <NvBlastTypes.h>
#include "BlastAssetData.h"
#include <cstring>

namespace Blast
{
	namespace Pipeline
	{
		BlastAssetData::~BlastAssetData()
		{
            if (m_assetData)
            {
                azfree(m_assetData);
            }
		}

		const NvBlastAsset* BlastAssetData::GetAssetData() const
		{
			return m_assetData;
		}

        void BlastAssetData::SetAssetData(const NvBlastAsset* assetData)
        {
			uint32_t assetSize = NvBlastAssetGetSize(assetData, Nv::Blast::logLL);
			m_assetData = static_cast<NvBlastAsset*>(azmalloc(assetSize, 16));
			memcpy(m_assetData, assetData, assetSize);
        }
		void BlastAssetData::AppendCollisionHull(AZStd::vector<AZStd::vector<AZ::Vector3>> collisionHull)
		{
			m_collisionHulls.push_back(collisionHull);
		}

		const size_t BlastAssetData::GetCollisionHullCount() const
		{
			return m_collisionHulls.size();
		}

        const AZStd::vector<AZStd::vector<AZ::Vector3>>& BlastAssetData::GetCollisionHull(size_t index) const
        {
            return m_collisionHulls[index];
        }
	}
}