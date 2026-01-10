/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzToolsFramework/UI/PropertyEditor/PropertyEditorAPI.h>
#include <SceneAPI/SceneCore/DataTypes/Rules/IRule.h>
#include <SceneAPI/SceneCore/DataTypes/GraphData/IBoneData.h>
#include <SceneAPI/SceneCore/DataTypes/GraphData/IMeshData.h>
#include <SceneAPI/SceneCore/Containers/SceneGraph.h>

#include <Source/Pipeline/MeshGroup.h>

namespace Blast
{
    namespace Pipeline
    {
        AZ_CLASS_ALLOCATOR_IMPL(MeshGroup, AZ::SystemAllocator);

        void VoronoiParams::Reflect(AZ::ReflectContext* context)
        {
            if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serializeContext->Class<VoronoiParams>()
                    ->Version(1)
                    ->Field("siteCount", &VoronoiParams::m_siteCount)
                    ;
                if (AZ::EditContext* editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<VoronoiParams>("Fracture Voronoi Params", "Fracture Voronoi Params")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &VoronoiParams::m_siteCount, "Number of Voronoi Cells",
                            "Number of Voronoi Cells")
                        ;
                }
            }
        }

        void ClusteredVoronoiParams::Reflect(AZ::ReflectContext* context)
        {
            if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serializeContext->Class<ClusteredVoronoiParams>()
                    ->Version(1)
                    ->Field("siteCount", &ClusteredVoronoiParams::m_siteCount)
                    ->Field("clusterCount", &ClusteredVoronoiParams::m_clusterCount)
                    ->Field("clusterRadius", &ClusteredVoronoiParams::m_clusterRadius)
                    ;
                if (AZ::EditContext* editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<ClusteredVoronoiParams>("Fracture Voronoi Params", "Fracture Voronoi Params")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &ClusteredVoronoiParams::m_siteCount, "Number of Voronoi Cells",
                            "Number of Voronoi Cells")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &ClusteredVoronoiParams::m_clusterCount, "Number of Clusters",
                            "Number of Clusters")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &ClusteredVoronoiParams::m_clusterRadius, "Cluster Radius",
                            "Cluster Radius")
                        ;
                }
            }
        }

        void SlicingParams::Reflect(AZ::ReflectContext* context)
        {
            if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serializeContext->Class<SlicingParams>()
                    ->Version(1)
                    ->Field("xSlices", &SlicingParams::m_xSlices)
                    ->Field("ySlices", &SlicingParams::m_ySlices)
                    ->Field("zSlices", &SlicingParams::m_zSlices)
                    ->Field("angleVariations", &SlicingParams::m_angleVariations)
                    ->Field("offsetVariations", &SlicingParams::m_offsetVariations)
                    ;
                if (AZ::EditContext* editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<SlicingParams>("Fracture Slicing Params", "Fracture Slicing Params")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &SlicingParams::m_xSlices, "Number of Slices on X Axis",
                            "Number of Slices on X Axis")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &SlicingParams::m_ySlices, "Number of Slices on Y Axis",
                            "Number of Slices on Y Axis")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &SlicingParams::m_zSlices, "Number of Slices on Z Axis",
                            "Number of Slices on Z Axis")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &SlicingParams::m_angleVariations, "Angle Variations",
                            "Angle Variations")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &SlicingParams::m_offsetVariations, "Offset Variations",
                            "Offset Variations")
                        ;
                }
            }
        }

        void PlaneCutParams::Reflect(AZ::ReflectContext* context)
        {
            if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serializeContext->Class<PlaneCutParams>()
                    ->Version(1)
                    ->Field("origin", &PlaneCutParams::m_origin)
                    ->Field("normal", &PlaneCutParams::m_normal)
                    ->Field("noiseAmplitude", &PlaneCutParams::m_noiseAmplitude)
                    ->Field("noiseFrequency", &PlaneCutParams::m_noiseFrequency)
                    ->Field("noiseOctaveNumber", &PlaneCutParams::m_noiseOctaveNumber)
                    ->Field("noiseSamplingInterval", &PlaneCutParams::m_noiseSamplingInterval)
                    ;
                if (AZ::EditContext* editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<PlaneCutParams>("Fracture Plane Cut Params", "Fracture Plane Cut Params")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &PlaneCutParams::m_origin, "Plane Origin",
                            "Plane Origin")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &PlaneCutParams::m_normal, "Plane Normal",
                            "Plane Normal")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &PlaneCutParams::m_noiseAmplitude, "Noise Amplitude",
                            "Noise Amplitude")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &PlaneCutParams::m_noiseFrequency, "Noise Frequency",
                            "Noise Frequency")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &PlaneCutParams::m_noiseOctaveNumber, "Noise Octave Number",
                            "Noise Octave Number")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &PlaneCutParams::m_noiseSamplingInterval, "Noise Sampling Interval",
                            "Noise Sampling Interval")
                        ;
                }
            }
        }

        MeshGroup::MeshGroup()
            : m_id(AZ::Uuid::CreateRandom())
        {
        }

        MeshGroup::~MeshGroup()
        {
        }

        void MeshGroup::Reflect(AZ::ReflectContext* context)
        {
            if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                VoronoiParams::Reflect(context);
                ClusteredVoronoiParams::Reflect(context);
                SlicingParams::Reflect(context);
                PlaneCutParams::Reflect(context);
                serializeContext->Class<MeshGroup, AZ::SceneAPI::DataTypes::ISceneNodeGroup>()->Version(1)
                    ->Field("id", &MeshGroup::m_id)
                    ->Field("name", &MeshGroup::m_name)
                    ->Field("NodeSelectionList", &MeshGroup::m_nodeSelectionList)
                    ->Field("rules", &MeshGroup::m_rules)
                    ->Field("cleanMesh", &MeshGroup::m_cleanMesh)
                    ->Field("replaceChunk", &MeshGroup::m_replaceChunk)
                    ->Field("seed", &MeshGroup::m_seed)
                    ->Field("fractureMethod", &MeshGroup::m_fractureMethod)
                    ->Field("voronoiParams", &MeshGroup::m_voronoiParams)
                    ->Field("clusteredVoronoiParams", &MeshGroup::m_clusteredVoronoiParams)
                    ->Field("slicingParams", &MeshGroup::m_slicingParams)
                    ->Field("planeCutParams", &MeshGroup::m_planeCutParams)
                    ;

                if (AZ::EditContext* editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<MeshGroup>("Blast Mesh group", "Configure blast mesh data exporting.")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->Attribute(AZ::Edit::Attributes::NameLabelOverride, "")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/MeshCollider.svg")
                        ->Attribute(AZ::Edit::Attributes::CategoryStyle, "display divider")
                        ->Attribute(AZ::Edit::Attributes::HelpPageURL, "https://www.o3de.org/docs/user-guide/assets/scene-settings/physx-tab/")

                        ->DataElement(AZ_CRC_CE("ManifestName"), &MeshGroup::m_name, "Name Blast Mesh",
                            "<span>Name for the group. This name will also be used as a part of the name for the "
                            "generated file.</span>")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &MeshGroup::m_nodeSelectionList, "Select meshes",
                            "<span>Select the meshes to be included in the mesh group.</span>")
                        ->Attribute("FilterName", "meshes")
                        ->Attribute("FilterType", AZ::SceneAPI::DataTypes::IMeshData::TYPEINFO_Uuid())
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &MeshGroup::OnNodeSelectionChanged)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &MeshGroup::m_rules, "",
                            "Add or remove rules to fine-tune the export process.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ_CRC_CE("PropertyVisibility_ShowChildrenOnly"))
                        ->DataElement(AZ::Edit::UIHandlers::Default, &MeshGroup::m_cleanMesh, "Clean Mesh",
                            "Clean mesh before fracture")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &MeshGroup::m_replaceChunk, "Replace Chunk",
                            "Newly generated chunks replace source chunk")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &MeshGroup::m_seed, "Seed",
                            "<span>Seed for the random number generator.</span>")
                        ->DataElement(AZ::Edit::UIHandlers::ComboBox, &MeshGroup::m_fractureMethod, "Fracture Method",
                            "<span>Fracture method for the mesh group.</span>")
                        ->EnumAttribute(FractureMethod::Island, "Island")
                        ->EnumAttribute(FractureMethod::Voronoi, "Voronoi")
                        ->EnumAttribute(FractureMethod::ClusteredVoronoi, "Clustered Voronoi")
                        ->EnumAttribute(FractureMethod::PlaneCut, "Plane Cut")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::EntireTree)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &MeshGroup::m_voronoiParams, "Voronoi Parameters",
                            "Parameters for Voronoi Fracture")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &MeshGroup::GetVoronoiParamsVisibility)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &MeshGroup::m_clusteredVoronoiParams, "Clustered Voronoi Parameters",
                            "Parameters for Clustered Voronoi Fracture")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &MeshGroup::GetClusteredVoronoiParamsVisibility)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &MeshGroup::m_slicingParams, "Slicing Parameters",
                            "Parameters for Slicing Fracture")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &MeshGroup::GetSlicingParamsVisibility)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &MeshGroup::m_planeCutParams, "Plane Cut Parameters",
                            "Parameters for Plane Cut Fracture")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &MeshGroup::GetPlaneCutParamsVisibility)
                        ;
                }
            }
        }

        const AZStd::string& MeshGroup::GetName() const
        {
            return m_name;
        }

        void MeshGroup::SetName(const AZStd::string& name)
        {
            m_name = name;
        }

        void MeshGroup::SetName(AZStd::string&& name)
        {
            m_name = AZStd::move(name);
        }

        const AZ::Uuid& MeshGroup::GetId() const
        {
            return m_id;
        }

        void MeshGroup::OverrideId(const AZ::Uuid& id)
        {
            m_id = id;
        }

        void MeshGroup::SetSceneGraph(const AZ::SceneAPI::Containers::SceneGraph* graph)
        {
            m_graph = graph;
        }

        AZ::SceneAPI::Containers::RuleContainer& MeshGroup::GetRuleContainer()
        {
            return m_rules;
        }

        const AZ::SceneAPI::Containers::RuleContainer& MeshGroup::GetRuleContainerConst() const
        {
            return m_rules;
        }

        AZ::SceneAPI::DataTypes::ISceneNodeSelectionList& MeshGroup::GetSceneNodeSelectionList()
        {
            return m_nodeSelectionList;
        }

        const AZ::SceneAPI::DataTypes::ISceneNodeSelectionList& MeshGroup::GetSceneNodeSelectionList() const
        {
            return m_nodeSelectionList;
        }

        AZ::u32 MeshGroup::OnNodeSelectionChanged()
        {
            return AZ::Edit::PropertyRefreshLevels::EntireTree;
        }
    }
}
