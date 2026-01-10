/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Memory/Memory.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzFramework/Physics/Common/PhysicsEvents.h>
#include <SceneAPI/SceneCore/Containers/RuleContainer.h>
#include <SceneAPI/SceneCore/DataTypes/Groups/ISceneNodeGroup.h>
#include <SceneAPI/SceneData/ManifestBase/SceneNodeSelectionList.h>

namespace AZ
{
    class ReflectContext;
    class SerializeContext;

    namespace SceneAPI::Containers
    {
        class SceneGraph;
    }
}

namespace Blast
{
    namespace Pipeline
    {
        enum class FractureMethod : AZ::u8
        {
            Island,
            Voronoi,
            ClusteredVoronoi,
            Slicing,
            PlaneCut,
            Cutout,
        };

        struct VoronoiParams
        {
            AZ_RTTI(VoronoiParams, "{1CDCA378-B577-46CF-A1E3-AC1C0D3E9027}");
            static void Reflect(AZ::ReflectContext* context);
            AZ::u32 m_siteCount = 5;
        };
        struct ClusteredVoronoiParams
        {
            AZ_RTTI(ClusteredVoronoiParams, "{76FE1648-88B9-49D3-9591-B6830559EAD1}");
            static void Reflect(AZ::ReflectContext* context);
            AZ::u32 m_siteCount = 5;
            AZ::u32 m_clusterCount = 5;
            float m_clusterRadius = 1.0f;
        };
        struct SlicingParams
        {
            AZ_RTTI(SlicingParams, "{57EA5926-CF73-4C9A-B8B0-B6E640E0027B}");
            static void Reflect(AZ::ReflectContext* context);
            AZ::u32 m_xSlices = 1;
            AZ::u32 m_ySlices = 1;
            AZ::u32 m_zSlices = 1;
            float m_angleVariations = 0.0f;
            float m_offsetVariations = 0.0f;
        };
        struct PlaneCutParams
        {
            AZ_RTTI(PlaneCutParams, "{D7D0C05D-B63E-4A9A-886B-78A3370E394A}");
            static void Reflect(AZ::ReflectContext* context);
            AZ::Vector3 m_origin = AZ::Vector3(0.0f, 0.0f, 0.0f);
            AZ::Vector3 m_normal = AZ::Vector3(1.0f, 0.0f, 0.0f);
            float m_noiseAmplitude = 0.0f;
            float m_noiseFrequency = 1.0f;
            AZ::u32 m_noiseOctaveNumber = 1;
            AZ::Vector3 m_noiseSamplingInterval = AZ::Vector3(1.0f, 1.0f, 1.0f);
        };
#if 0
        struct FractureCutoutParams
        {
            static void Reflect(AZ::ReflectContext* context);
        };
#endif
        class MeshGroup
            : public AZ::SceneAPI::DataTypes::ISceneNodeGroup
        {
        public:
            AZ_RTTI(MeshGroup, "{BFE34022-27CF-4294-9C85-50B90276A227}", AZ::SceneAPI::DataTypes::ISceneNodeGroup);
			AZ_CLASS_ALLOCATOR_DECL

			MeshGroup();
            ~MeshGroup() override;

            static void Reflect(AZ::ReflectContext* context);

            const AZStd::string& GetName() const override;
            void SetName(const AZStd::string& name);
            void SetName(AZStd::string&& name);
            const AZ::Uuid& GetId() const override;
            void OverrideId(const AZ::Uuid& id);
            void SetSceneGraph(const AZ::SceneAPI::Containers::SceneGraph* graph);

            AZ::SceneAPI::Containers::RuleContainer& GetRuleContainer() override;
            const AZ::SceneAPI::Containers::RuleContainer& GetRuleContainerConst() const override;

            AZ::SceneAPI::DataTypes::ISceneNodeSelectionList& GetSceneNodeSelectionList() override;
            const AZ::SceneAPI::DataTypes::ISceneNodeSelectionList& GetSceneNodeSelectionList() const override;

            const FractureMethod GetFractureMethod() const { return m_fractureMethod; }

            const bool IsCleanMeshEnabled() const { return m_cleanMesh; }

            const bool IsReplaceChunkEnabled() const { return m_replaceChunk; }

            const AZ::s32 GetSeed() const { return m_seed; }

            const VoronoiParams& GetVoronoiParams() const { return m_voronoiParams; }

            const ClusteredVoronoiParams& GetClusteredVoronoiParams() const { return m_clusteredVoronoiParams; }

            const SlicingParams& GetSlicingParams() const { return m_slicingParams; }

            const PlaneCutParams& GetPlaneCutParams() const { return m_planeCutParams; }

            bool GetVoronoiParamsVisibility() const { return m_fractureMethod == FractureMethod::Voronoi; }
            bool GetClusteredVoronoiParamsVisibility() const { return m_fractureMethod == FractureMethod::ClusteredVoronoi; }
            bool GetSlicingParamsVisibility() const { return m_fractureMethod == FractureMethod::Slicing; }
            bool GetPlaneCutParamsVisibility() const { return m_fractureMethod == FractureMethod::PlaneCut; }

        protected:

            AZ::u32 OnNodeSelectionChanged();
            AZ::Uuid m_id{};
            AZStd::string m_name{};
            AZ::SceneAPI::SceneData::SceneNodeSelectionList m_nodeSelectionList{};
            AZ::SceneAPI::Containers::RuleContainer m_rules{};

            const AZ::SceneAPI::Containers::SceneGraph* m_graph = nullptr;

        private:
            bool m_cleanMesh = false;
            bool m_replaceChunk = false;
            AZ::s32 m_seed = 0;
            FractureMethod m_fractureMethod;
            VoronoiParams m_voronoiParams;
            ClusteredVoronoiParams m_clusteredVoronoiParams;
            SlicingParams m_slicingParams;
            PlaneCutParams m_planeCutParams;
        };
    }
}
