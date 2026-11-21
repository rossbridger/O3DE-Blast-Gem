/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzFramework/StringFunc/StringFunc.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzToolsFramework/Debug/TraceContext.h>
#include <SceneAPI/SceneCore/Containers/Utilities/Filters.h>
#include <SceneAPI/SceneCore/Containers/Utilities/SceneGraphUtilities.h>
#include <SceneAPI/SceneCore/DataTypes/GraphData/IMeshData.h>
#include <SceneAPI/SceneCore/Events/ExportEventContext.h>
#include <SceneAPI/SceneCore/Events/ExportProductList.h>
#include <SceneAPI/SceneCore/Utilities/FileUtilities.h>
#include <SceneAPI/SceneCore/Utilities/Reporting.h>
#include <SceneAPI/SceneCore/Containers/Utilities/SceneUtilities.h>
#include <SceneAPI/SceneCore/Containers/Views/SceneGraphChildIterator.h>
#include <SceneAPI/SceneCore/DataTypes/GraphData/IMaterialData.h>
#include <SceneAPI/SceneData/Rules/CoordinateSystemRule.h>
#include <SceneAPI/SceneData/GraphData/CustomPropertyData.h>

#include <NvBlastExtAuthoringMesh.h>

#include <Source/Pipeline/BlastExporter.h>
#include <Source/Pipeline/MeshGroup.h>
#include <Source/Pipeline/BlastAssetData.h>

#include <AzCore/std/smart_ptr/make_shared.h>
#include <NvBlastExtTkSerialization.h>
#include <NvBlastTkAsset.h>
#include <NvBlastTkFramework.h>
#include <Asset/BlastAsset.h>
#include <Blast/BlastSystemBus.h>
#include <Asset/BlastAssetHandler.h>
#include <AzCore/Serialization/Utils.h>
#include <PhysX/MeshAsset.h>
#include <AzCore/std/any.h>


using AZ::SceneAPI::Containers::SceneGraph;
using AZ::SceneAPI::DataTypes::IMeshData;
using AZ::SceneAPI::DataTypes::ICustomPropertyData;
using AZ::SceneAPI::Events::ProcessingResult;
using NodeIndex = AZ::SceneAPI::Containers::SceneGraph::NodeIndex;
namespace Containers = AZ::SceneAPI::Containers;
namespace Views = Containers::Views;

namespace Blast
{
    namespace Pipeline
    {
        namespace SceneContainers = AZ::SceneAPI::Containers;
        namespace SceneEvents = AZ::SceneAPI::Events;
        namespace SceneUtil = AZ::SceneAPI::Utilities;

        static const char* const DefaultMaterialName = "default";

        static bool WriteCookedMeshToFile(const AZStd::string& filePath, const PhysX::Pipeline::MeshAssetData& assetData)
        {
            AZ::SerializeContext* serializeContext = nullptr;
            AZ::ComponentApplicationBus::BroadcastResult(serializeContext, &AZ::ComponentApplicationRequests::GetSerializeContext);
            return AZ::Utils::SaveObjectToFile(filePath, AZ::DataStream::ST_BINARY, &assetData, serializeContext);
        }

        BlastExporter::BlastExporter()
        {
            BindToCall(&BlastExporter::ProcessContext);
        }

        void BlastExporter::Reflect(AZ::ReflectContext* context)
        {
            AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
            if (serializeContext)
            {
                serializeContext->Class<BlastExporter, AZ::SceneAPI::SceneCore::ExportingComponent>()
                    ->Version(1); // Use Blast version to trigger assets recompilation
            }
        }

        SceneEvents::ProcessingResult BlastExporter::ProcessContext(SceneEvents::ExportEventContext& context) const
        {
            AZ_TraceContext("Exporter", "Blast");

            SceneEvents::ProcessingResultCombiner result;

            const AZ::SceneAPI::Containers::Scene& scene = context.GetScene();
            const AZ::SceneAPI::Containers::SceneGraph& graph = scene.GetGraph();

            const SceneContainers::SceneManifest& manifest = context.GetScene().GetManifest();

            SceneContainers::SceneManifest::ValueStorageConstData valueStorage = manifest.GetValueStorage();
            auto view = SceneContainers::MakeDerivedFilterView<MeshGroup>(valueStorage);

            for (const MeshGroup& blastMeshGroup : view)
            {
                const AZStd::string& groupName = blastMeshGroup.GetName();

                AZ_TraceContext("Group Name", groupName);

                const auto& sceneNodeSelectionList = blastMeshGroup.GetSceneNodeSelectionList();

                SceneEvents::ProcessingResult enumerationResult = SceneEvents::ProcessingResult::Success;

                sceneNodeSelectionList.EnumerateSelectedNodes(
                    [&](const AZStd::string& name)
                    {
                        AZ_Info(AZ::SceneAPI::Utilities::LogWindow, "Blast: Exporting from node %s", name.c_str());
                        AZ::SceneAPI::Containers::SceneGraph::NodeIndex nodeIndex = graph.Find(name);

                        NodeIndex customPropertyIndex =
                            AZ::SceneAPI::Utilities::GetImmediateChildOfType(graph, nodeIndex, azrtti_typeid<ICustomPropertyData>());

                        if (!customPropertyIndex.IsValid())
                        {
                            return true;
                        }

                        const ICustomPropertyData* customPropertyDataNode =
                            azrtti_cast<const ICustomPropertyData*>(graph.GetNodeContent(customPropertyIndex).get());

                        const ICustomPropertyData::PropertyMap& propertyMap = customPropertyDataNode->GetPropertyMap();

                        AZ::SceneAPI::Containers::SceneGraph::NodeIndex blastNodeIndex;

                        constexpr const char* FracturedMeshPropertyMapKey = "o3de_fractured_mesh_node";
                        // Now look up the fractured index
                        auto iter = propertyMap.find(FracturedMeshPropertyMapKey);
                        if (iter != propertyMap.end())
                        {
                            const auto& [key, fracturedAnyIndex] = *iter;
                            if (!fracturedAnyIndex.empty() && fracturedAnyIndex.is<NodeIndex>())
                            {
                                blastNodeIndex = AZStd::any_cast<NodeIndex>(fracturedAnyIndex);
                            }
                            else
                            {
                                return true;
                            }
                        }
                        else
                        {
                            return true;
                        }

                        auto blastNode = azrtti_cast<const BlastAssetData*>(*graph.ConvertToStorageIterator(blastNodeIndex));

                        if (!blastNode)
                        {
                            return true;
                        }

                        // write blast asset data
                        auto llAsset = blastNode->GetAssetData();
                        Nv::Blast::ExtSerialization* serializer;
                        Nv::Blast::TkFramework* framework;
                        BlastSystemRequestBus::BroadcastResult(serializer, &BlastSystemRequestBus::Events::GetExtSerialization);
                        BlastSystemRequestBus::BroadcastResult(framework, &BlastSystemRequestBus::Events::GetTkFramework);
                        const Nv::Blast::TkAsset* asset = framework->createAsset(llAsset);

                        void* buffer;

                        uint64_t size = NvBlastExtSerializationSerializeTkAssetIntoBuffer(buffer, *serializer, asset);

                        AZStd::vector<AZ::u8> assetBuffer(size);
                        memcpy(assetBuffer.data(), buffer, size);
                        NVBLAST_FREE(buffer);

                        AZStd::string assetName = blastMeshGroup.GetName();
                        AZStd::string filename = SceneUtil::FileUtilities::CreateOutputFileName(
                            assetName, context.GetOutputDirectory(), BlastAssetHandler::s_assetFileExtension, context.GetScene().GetSourceExtension());
                        // writes blast asset to the file
                        if (AZ::Utils::SaveStreamToFile(filename, assetBuffer))
                        {
                            AZStd::string productUuidString = blastMeshGroup.GetId().ToString<AZStd::string>();
                            AZ::Uuid productUuid = AZ::Uuid::CreateName(productUuidString);
                            context.GetProductList().AddProduct(
                                AZStd::move(filename), productUuid, AZ::AzTypeInfo<BlastAsset>::Uuid(), AZStd::nullopt, AZStd::nullopt);
                        }

                        // write mesh data
                        for (size_t i = 0; i < blastNode->GetCollisionHullCount(); ++i)
                        {
                            PhysX::Pipeline::MeshAssetData assetData;
                            const AZStd::vector<AZStd::vector<AZ::Vector3>>& collisionHull = blastNode->GetCollisionHull(i);

                            AZStd::string chunkFilename = SceneUtil::FileUtilities::CreateOutputFileName(
                                assetName + AZStd::string::format("_chunk%zu", i), context.GetOutputDirectory(),
                                ".pxmesh", context.GetScene().GetSourceExtension());

                            // needs to be cooked before export
                            for (size_t j = 0; j < collisionHull.size(); ++j)
                            {
                                PhysX::Pipeline::MeshAssetData::ShapeConfigurationPair shape;
                                const AZStd::vector<AZ::Vector3>& points = collisionHull[j];
                                AZStd::vector<AZ::u8> cookedMeshData;
                                Physics::SystemRequestBus::Broadcast(&Physics::SystemRequests::CookConvexMeshToMemory, points.data(), points.size(), cookedMeshData);
                                AZStd::shared_ptr<Physics::CookedMeshShapeConfiguration> shapeConfig =
                                    AZStd::make_shared<Physics::CookedMeshShapeConfiguration>();
                                shapeConfig->SetCookedMeshData(cookedMeshData.data(), cookedMeshData.size(), Physics::CookedMeshShapeConfiguration::MeshType::Convex);
                                shape.second = shapeConfig;
                                assetData.m_colliderShapes.push_back(shape);
                            }

                            if (WriteCookedMeshToFile(chunkFilename, assetData))
                            {
                                AZStd::string productUuidString = blastMeshGroup.GetId().ToString<AZStd::string>();
                                AZ::Uuid productUuid = AZ::Uuid::CreateName(productUuidString);

                                context.GetProductList().AddProduct(
                                    AZStd::move(chunkFilename), productUuid, AZ::AzTypeInfo<PhysX::Pipeline::MeshAsset>::Uuid(), AZStd::nullopt, AZStd::nullopt);
                            }
                        }

                        return true;
                    });

                if (enumerationResult == SceneEvents::ProcessingResult::Failure)
                {
                    return enumerationResult;
                }
            }
            return result.GetResult();
        }
    }
}
