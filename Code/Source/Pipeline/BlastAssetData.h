#pragma once

#include <SceneAPI/SceneCore/DataTypes/IGraphObject.h>

struct NvBlastAsset;

namespace Blast
{
	namespace Pipeline
	{
		class BlastAssetData
			: public AZ::SceneAPI::DataTypes::IGraphObject
		{
		public:
			AZ_RTTI(BlastAssetData, "{127463AF-6F7A-4329-92B5-1D6CF25CA3B8}", AZ::SceneAPI::DataTypes::IGraphObject);

			BlastAssetData() = default;
			virtual ~BlastAssetData();

			const NvBlastAsset* GetAssetData() const;
			void SetAssetData(const NvBlastAsset* data);
			void AppendCollisionHull(const AZStd::vector<AZStd::vector<AZ::Vector3>> collisionHull);
			void CloneAttributesFrom([[maybe_unused]] const IGraphObject* sourceObject) override {}

			const size_t GetCollisionHullCount() const;
			const AZStd::vector<AZStd::vector<AZ::Vector3>>& GetCollisionHull(size_t index) const;

		private:
			NvBlastAsset* m_assetData = nullptr;
			AZStd::vector<AZStd::vector<AZStd::vector<AZ::Vector3>>> m_collisionHulls;
        };
	}
}
