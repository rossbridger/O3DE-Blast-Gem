#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Debug/Trace.h>
#include <AzCore/Debug/Profiler.h>
#include <AzCore/Math/Random.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <SceneAPI/SceneCore/Components/GenerationComponent.h>
#include <SceneAPI/SceneCore/Containers/Scene.h>
#include <SceneAPI/SceneCore/Containers/SceneGraph.h>
#include <SceneAPI/SceneCore/Containers/Views/SceneGraphChildIterator.h>
#include <SceneAPI/SceneCore/Containers/Utilities/Filters.h>
#include <SceneAPI/SceneCore/Containers/Utilities/SceneGraphUtilities.h>
#include <SceneAPI/SceneCore/Containers/Views/ConvertIterator.h>
#include <SceneAPI/SceneCore/Containers/Views/FilterIterator.h>
#include <SceneAPI/SceneCore/Containers/Views/View.h>
#include <SceneAPI/SceneCore/DataTypes/GraphData/IMeshData.h>
#include <SceneAPI/SceneCore/DataTypes/GraphData/IMeshVertexUVData.h>
#include <SceneAPI/SceneCore/DataTypes/ManifestBase/ISceneNodeSelectionList.h>
#include <SceneAPI/SceneCore/Events/GenerateEventContext.h>
#include <SceneAPI/SceneCore/Events/ProcessingResult.h>
#include <SceneAPI/SceneCore/Utilities/Reporting.h>
#include <SceneAPI/SceneData/GraphData/BlendShapeData.h>
#include <SceneAPI/SceneData/GraphData/CustomPropertyData.h>
#include <SceneAPI/SceneData/GraphData/MeshData.h>
#include <SceneAPI/SceneData/GraphData/MeshVertexUVData.h>
#include <SceneAPI/SceneData/Groups/MeshGroup.h>
#include <Source/Pipeline/BlastFractureComponent.h>
#include <Source/Pipeline/MeshGroup.h>
#include <NvBlastExtAuthoring.h>
#include <NvBlastExtAuthoringMesh.h>
#include <NvBlastExtAuthoringMeshCleaner.h>
#include <NvBlastExtAuthoringFractureTool.h>
#include <NvBlastExtAuthoringConvexMeshBuilder.h>
#include <NvBlastExtAuthoringBondGenerator.h>
#include <Common/Utils.h>
#include "BlastAssetData.h"

using AZ::SceneAPI::Containers::SceneGraph;
using AZ::SceneAPI::DataTypes::IBlendShapeData;
using AZ::SceneAPI::DataTypes::IMeshData;
using AZ::SceneAPI::DataTypes::IMeshVertexUVData;
using AZ::SceneAPI::DataTypes::IMeshVertexColorData;
using AZ::SceneAPI::DataTypes::ICustomPropertyData;
using AZ::SceneAPI::Events::ProcessingResult;
using AZ::SceneAPI::Events::GenerateSimplificationEventContext;
using AZ::SceneAPI::SceneCore::GenerationComponent;
using AZ::SceneData::GraphData::BlendShapeData;
using AZ::SceneData::GraphData::MeshData;
using AZ::SceneData::GraphData::MeshVertexUVData;
using NodeIndex = AZ::SceneAPI::Containers::SceneGraph::NodeIndex;
namespace Containers = AZ::SceneAPI::Containers;
namespace Views = Containers::Views;

inline constexpr const char* FracturedMeshSuffix = "_chunk";
inline constexpr const char* FracturedMeshPropertyMapKey = "o3de_fractured_mesh_node";

// Recurse through the SceneAPI's iterator types, extracting the real underlying iterator.
struct ConvertToHierarchyIterator
{
	template<typename T, typename U>
	static auto Unwrap(const Containers::Views::ConvertIterator<T, U>& it)
	{
		return Unwrap(it.GetBaseIterator());
	}

	template<typename T, typename U>
	static auto Unwrap(const Containers::Views::FilterIterator<T, U>& it)
	{
		return Unwrap(it.GetBaseIterator());
	}

	template<typename T>
	static auto Unwrap(const Containers::Views::SceneGraphChildIterator<T>& it)
	{
		return it.GetHierarchyIterator();
	}
};

AZ_DEFINE_BUDGET(Blast);

static ICustomPropertyData::PropertyMap& FindOrCreateCustomPropertyData(
	Containers::SceneGraph& graph, const Containers::SceneGraph::NodeIndex& nodeIndex)
{
	NodeIndex customPropertyIndex =
		AZ::SceneAPI::Utilities::GetImmediateChildOfType(graph, nodeIndex, azrtti_typeid<ICustomPropertyData>());

	if (!customPropertyIndex.IsValid())
	{
		// If no custom property data node exists, insert one
		AZStd::shared_ptr<AZ::SceneData::GraphData::CustomPropertyData> createdCustumPropertyData =
			AZStd::make_shared<AZ::SceneData::GraphData::CustomPropertyData>();
		customPropertyIndex = graph.AddChild(nodeIndex, "custom_properties", AZStd::move(createdCustumPropertyData));
	}

	ICustomPropertyData* customPropertyDataNode =
		azrtti_cast<ICustomPropertyData*>(graph.GetNodeContent(customPropertyIndex).get());

	return customPropertyDataNode->GetPropertyMap();
}

static bool HasFracturedMeshNode(ICustomPropertyData::PropertyMap& propertyMap)
{
	// Now look up the fractured index
	auto iter = propertyMap.find(FracturedMeshPropertyMapKey);
	if (iter != propertyMap.end())
	{
		const auto& [key, fracturedAnyIndex] = *iter;
		if (!fracturedAnyIndex.empty() && fracturedAnyIndex.is<NodeIndex>())
		{
			return true;
		}
	}

	return false;
}

class RandomGenerator : public Nv::Blast::RandomGeneratorBase
{
public:
	float getRandomValue() override
	{
		return m_random.GetRandomFloat();
	}
	void seed(int32_t seed) override
	{
		m_random.SetSeed(seed);
	}
private:
	AZ::SimpleLcgRandom m_random;
};

class CollisionBuilder : public Nv::Blast::ConvexMeshBuilder
{
	AZ_CLASS_ALLOCATOR(CollisionBuilder, AZ::SystemAllocator);
public:
	Nv::Blast::CollisionHull* buildCollisionGeometry([[maybe_unused]] uint32_t verticesCount, [[maybe_unused]] const NvcVec3* vData) override
	{
		AZ_Info("Blast", "Allocating collision hull");
		Nv::Blast::CollisionHull *collisionHull = aznew Nv::Blast::CollisionHull();
		collisionHull->pointsCount = verticesCount;
		collisionHull->points = aznew NvcVec3[verticesCount];
		memcpy(collisionHull->points, vData, verticesCount * sizeof(NvcVec3));

		return collisionHull;
	}

	void releaseCollisionHull(Nv::Blast::CollisionHull* collisionHull) const override
    {
		AZ_Info("Blast", "releasing collision hull");
		delete collisionHull->points;
		delete collisionHull;
    }

	void release() override
	{
		delete this;
	}
};

namespace Blast
{
	namespace Pipeline
	{

		Nv::Blast::Mesh* CreateBlastMesh(const IMeshData* meshData, const IMeshVertexUVData* meshVertexUvData)
		{
			if (meshData == nullptr)
			{
				AZ_Error("Blast", false, "Mesh data is null.");
				return nullptr;
			}
			const uint32_t vertexCount = meshData->GetVertexCount();

			AZStd::vector<NvcVec3> positions(vertexCount);
			AZStd::vector<NvcVec3> normals(vertexCount);
			AZStd::vector<NvcVec2> uvs(vertexCount);

			for (unsigned int i = 0; i < vertexCount; ++i)
			{
				positions[i] = Convert(meshData->GetPosition(i));
				normals[i] = Convert(meshData->GetNormal(i));
				if (meshVertexUvData)
				{
					uvs[i] = Convert(meshVertexUvData->GetUV(i));
				}
			}

			const uint32_t indicesCount = meshData->GetFaceCount() * 3;
			AZStd::vector<uint32_t> indices(indicesCount);
			AZStd::vector<int32_t> materialIds(meshData->GetFaceCount());
			for (unsigned int i = 0; i < meshData->GetFaceCount(); ++i)
			{
				indices[i * 3] = meshData->GetFaceInfo(i).vertexIndex[0];
				indices[i * 3 + 1] = meshData->GetFaceInfo(i).vertexIndex[1];
				indices[i * 3 + 2] = meshData->GetFaceInfo(i).vertexIndex[2];
				materialIds[i] = static_cast<int32_t>(meshData->GetFaceMaterialId(i));
			}

			Nv::Blast::Mesh* nvMesh = NvBlastExtAuthoringCreateMesh(
				positions.data(),
				normals.data(),
				uvs.data(),
				vertexCount,
				indices.data(),
				indicesCount);
			nvMesh->setMaterialId(materialIds.data());
			return nvMesh;
		}

		void BlastFractureComponent::Reflect(AZ::ReflectContext* context)
		{
			auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
			if (serializeContext)
			{
				serializeContext->Class<BlastFractureComponent, GenerationComponent>()->Version(1);
			}
		}

		BlastFractureComponent::BlastFractureComponent()
		{
			BindToCall(&BlastFractureComponent::FractureMeshes);
		}

		AZ::SceneAPI::Events::ProcessingResult BlastFractureComponent::FractureMeshes(AZ::SceneAPI::Events::GenerateEventContext& context) const
		{
			SceneGraph& graph = context.GetScene().GetGraph();

			AZStd::vector<AZStd::pair<const IMeshData*, NodeIndex>> meshes;
			const auto meshNodes = Containers::MakeDerivedFilterView<IMeshData>(graph.GetContentStorage());
			for (auto it = meshNodes.cbegin(); it != meshNodes.cend(); ++it)
			{
				// Get the mesh data and node index and store them in the vector as a pair, so we can iterate over them later.
				// The sequential calls to GetBaseIterator unwrap the layers of FilterIterators from the MakeDerivedFilterView
				meshes.emplace_back(&(*it), graph.ConvertToNodeIndex(it.GetBaseIterator().GetBaseIterator()));
			}

			const auto meshGroups = Containers::MakeDerivedFilterView<MeshGroup>(context.GetScene().GetManifest().GetValueStorage());

			AZStd::unordered_map<const MeshGroup*, AZStd::vector<AZStd::string_view>> selectedNodes;

			for (const MeshGroup& meshGroup : meshGroups)
			{
				meshGroup.GetSceneNodeSelectionList().EnumerateSelectedNodes(
					[&selectedNodes, &meshGroup](const AZStd::string& name)
					{
						selectedNodes[&meshGroup].emplace_back(name);
						return true;
					});
			}

			const auto childNodes = [&graph](NodeIndex nodeIndex) { return Views::MakeSceneGraphChildView(graph, nodeIndex, graph.GetContentStorage().cbegin(), true); };
			const auto nodeIndexes = [&graph](const auto& view)
				{
					AZStd::vector<NodeIndex> indexes;
					indexes.reserve(AZStd::distance(view.begin(), view.end()));
					for (auto it = view.begin(); it != view.end(); ++it)
					{
						indexes.emplace_back(graph.ConvertToNodeIndex(ConvertToHierarchyIterator::Unwrap(it)));
					}
					return indexes;
				};

			AZStd::vector<AZStd::shared_ptr<AZ::SceneAPI::SceneData::MeshGroup>> chuckMeshGroups;
			// Iterate over them. We had to build the array before as this method can insert new nodes, so using the iterator directly would fail.
			for (const auto& [mesh, nodeIndex] : meshes)
			{
				// A Mesh can have multiple child nodes that contain other data streams, like uvs and tangents

				const auto uvDatasView = Containers::MakeDerivedFilterView<IMeshVertexUVData>(childNodes(nodeIndex));

				const AZStd::vector<AZStd::reference_wrapper<const IMeshVertexUVData>> uvDatas(uvDatasView.begin(), uvDatasView.end());

				const AZStd::string_view nodePath(graph.GetNodeName(nodeIndex).GetPath(), graph.GetNodeName(nodeIndex).GetPathLength());

				for (const MeshGroup& meshGroup : meshGroups)
				{
					if (!selectedNodes.contains(&meshGroup))
					{
						AZ_Warning(
							AZ::SceneAPI::Utilities::LogWindow,
							false,
							"MeshGroup %s wasn't found in the list of selected nodes.",
							meshGroup.GetName().c_str());
						continue;
					}
					// Skip meshes that are not used by this mesh group
					if (AZStd::find(selectedNodes.at(&meshGroup).cbegin(), selectedNodes.at(&meshGroup).cend(), nodePath) == selectedNodes.at(&meshGroup).cend())
					{
						continue;
					}

					ICustomPropertyData::PropertyMap& unfracturedPropertyMap = FindOrCreateCustomPropertyData(graph, nodeIndex);
					if (HasFracturedMeshNode(unfracturedPropertyMap))
					{
						// There is already an optimized mesh node for this mesh, so skip it.
						// There must be another mesh group already referencing this mesh node.
						continue;
					}
					
					if (uvDatas.size() > 1)
					{
						AZ_Info("Blast", "Multiple UV sets found, using the first one.");
					}
					const IMeshVertexUVData* uvData = uvDatas.empty() ? nullptr : &(uvDatas[0].get());

					Nv::Blast::Mesh* nvMesh = CreateBlastMesh(mesh, uvData);

					if (nvMesh == nullptr)
					{
						AZ_Error("Blast", false, "Failed to create Blast mesh.");
						return ProcessingResult::Failure;
					}

					Nv::Blast::FractureTool* fractureTool = NvBlastExtAuthoringCreateFractureTool();

					const Blast::Pipeline::FractureMethod fractureMethod = meshGroup.GetFractureMethod();

					if (meshGroup.IsCleanMeshEnabled()) // clean up mesh
					{
						Nv::Blast::MeshCleaner* cleaner = NvBlastExtAuthoringCreateMeshCleaner();
						Nv::Blast::Mesh* newMesh = cleaner->cleanMesh(nvMesh);
						cleaner->release();
						nvMesh->release();
						nvMesh = newMesh;
					}

					fractureTool->setSourceMeshes(&nvMesh, 1); // TODO: support multiple source meshes

					bool fractureSuccess = true;
					RandomGenerator rng;
					rng.seed(meshGroup.GetSeed()); //TODO: make seed configurable
					Nv::Blast::VoronoiSitesGenerator* voronoiSitesGenerator = NvBlastExtAuthoringCreateVoronoiSitesGenerator(nvMesh, &rng);
					if (voronoiSitesGenerator == nullptr)
					{
						AZ_Error("Blast", false, "Failed to create Voronoi sites generator");
						fractureSuccess = false;
					}

					if (fractureSuccess)
					{
						switch (fractureMethod)
						{
						case Blast::Pipeline::FractureMethod::Island:
						{
							fractureTool->islandDetectionAndRemoving(0, true);
							break;
						}
						case Blast::Pipeline::FractureMethod::Voronoi:
						{
							uint32_t cellsCount = meshGroup.GetVoronoiParams().m_siteCount;
							voronoiSitesGenerator->uniformlyGenerateSitesInMesh(cellsCount);
							const NvcVec3* sites = nullptr;
							uint32_t sitesCount = voronoiSitesGenerator->getVoronoiSites(sites);
							if (fractureTool->voronoiFracturing(0, sitesCount, sites, meshGroup.IsReplaceChunkEnabled()) != 0)
							{
								AZ_Error("Blast", false, "Failed to fracture with Voronoi");
								fractureSuccess = false;
							}
							break;
						}
						case Blast::Pipeline::FractureMethod::ClusteredVoronoi:
						{
							uint32_t cellsCount = meshGroup.GetClusteredVoronoiParams().m_siteCount;
							uint32_t clusterCount = meshGroup.GetClusteredVoronoiParams().m_clusterCount;
							float clusterRad = meshGroup.GetClusteredVoronoiParams().m_clusterRadius;
							voronoiSitesGenerator->clusteredSitesGeneration(cellsCount, clusterCount, clusterRad);
							const NvcVec3* sites = nullptr;
							uint32_t sitesCount = voronoiSitesGenerator->getVoronoiSites(sites);
							if (fractureTool->voronoiFracturing(0, sitesCount, sites, meshGroup.IsReplaceChunkEnabled()) != 0)
							{
								AZ_Error("Blast", false, "Failed to fracture with Clustered Voronoi");
                                fractureSuccess = false;
							}
							break;
						}
						case Blast::Pipeline::FractureMethod::Slicing:
						{
							Nv::Blast::SlicingConfiguration slConfig;
							slConfig.x_slices = meshGroup.GetSlicingParams().m_xSlices;
							slConfig.y_slices = meshGroup.GetSlicingParams().m_ySlices;
							slConfig.z_slices = meshGroup.GetSlicingParams().m_zSlices;
							slConfig.angle_variations = meshGroup.GetSlicingParams().m_angleVariations;
							slConfig.offset_variations = meshGroup.GetSlicingParams().m_offsetVariations;
							if (fractureTool->slicing(0, slConfig, meshGroup.IsReplaceChunkEnabled(), &rng) != 0)
							{
								AZ_Error("Blast", false, "Failed to fracture with Slicing");
                                fractureSuccess = false;
							}
							break;
						}
						case Blast::Pipeline::FractureMethod::PlaneCut:
						{
							Nv::Blast::NoiseConfiguration noise;
							NvcVec3 normal = Convert(meshGroup.GetPlaneCutParams().m_normal);
							NvcVec3 point = Convert(meshGroup.GetPlaneCutParams().m_origin);
							noise.amplitude = meshGroup.GetPlaneCutParams().m_noiseAmplitude;
							noise.frequency = meshGroup.GetPlaneCutParams().m_noiseFrequency;
							noise.octaveNumber = meshGroup.GetPlaneCutParams().m_noiseOctaveNumber;
							noise.samplingInterval = Convert(meshGroup.GetPlaneCutParams().m_noiseSamplingInterval);
							if (fractureTool->cut(0, normal, point, noise, false, &rng) != 0)
							{
								AZ_Error("Blast", false, "Failed to fracture with Cutout (in half-space, plane cut)");
								fractureSuccess = false;
							}
							break;
						}
						case Blast::Pipeline::FractureMethod::Cutout:
						{
							AZ_Error("Blast", false, "Cutout not yet supported");
							fractureSuccess = false;
							break;
						}
						default:
							AZ_Error("Blast", false, "Unknown fracture method");
							fractureSuccess = false;
						}
					}

					voronoiSitesGenerator->release();
					nvMesh->release();

					if (!fractureSuccess)
					{
						fractureTool->release();
						return ProcessingResult::Failure;
					}

					// build collision mesh

					CollisionBuilder collisionBuilder;
					Nv::Blast::BlastBondGenerator* bondGenerator = NvBlastExtAuthoringCreateBondGenerator(&collisionBuilder);
					Nv::Blast::ConvexDecompositionParams collisionParameter;
					Nv::Blast::AuthoringResult* result = NvBlastExtAuthoringProcessFracture(*fractureTool, *bondGenerator, collisionBuilder, collisionParameter);

					// get mesh data from result
					AZ_Info("Blast", "Fracture result: %u chunks", result->chunkCount);
					AZStd::unique_ptr<BlastAssetData> assetData = AZStd::make_unique<BlastAssetData>();
					assetData->SetAssetData(result->asset);

					for (uint32_t chunkIndex = 0; chunkIndex < result->chunkCount; ++chunkIndex)
					{
						AZStd::unique_ptr<MeshData> chunkMesh = AZStd::make_unique<MeshData>();
						AZStd::unique_ptr<MeshVertexUVData> chunkUV = AZStd::make_unique<MeshVertexUVData>();
						//const auto& chunk = result->chunkDescs[chunkIndex];

						AZ_Info("Blast", "  chunk %u: %u faces", chunkIndex, result->geometryOffset[chunkIndex + 1] - result->geometryOffset[chunkIndex]);
						uint32_t index = 0;
						for (uint32_t faceIndex = result->geometryOffset[chunkIndex]; faceIndex < result->geometryOffset[chunkIndex + 1]; ++faceIndex)
						{
							const auto& face = result->geometry[faceIndex];
							chunkMesh->AddPosition(Convert(face.a.p));
							chunkMesh->AddPosition(Convert(face.b.p));
							chunkMesh->AddPosition(Convert(face.c.p));
							chunkMesh->AddNormal(Convert(face.a.n));
							chunkMesh->AddNormal(Convert(face.b.n));
							chunkMesh->AddNormal(Convert(face.c.n));
							chunkMesh->AddFace(index, index+1, index+2, face.materialId);

							chunkUV->AppendUV(Convert(face.a.uv[0]));
							chunkUV->AppendUV(Convert(face.b.uv[0]));
							chunkUV->AppendUV(Convert(face.c.uv[0]));
							index += 3;
						}

						const AZStd::string chunkName = AZStd::string::format("%s_%s_chunk%u",
							graph.GetNodeName(nodeIndex).GetName(),
							meshGroup.GetName().c_str(),
							chunkIndex);
						const NodeIndex chunkMeshNodeIndex =
							graph.AddChild(nodeIndex, chunkName.c_str(), AZStd::move(chunkMesh));
                        // Insert a mesh group into manifest
						auto chuckMeshGroup = AZStd::make_shared<AZ::SceneAPI::SceneData::MeshGroup>();
						chuckMeshGroup->SetName(chunkName);
						chuckMeshGroup->GetSceneNodeSelectionList().AddSelectedNode(graph.GetNodeName(chunkMeshNodeIndex).GetPath());
						chuckMeshGroups.push_back(chuckMeshGroup);
						
						if (!chunkMeshNodeIndex.IsValid())
						{
							return ProcessingResult::Failure;
						}
						const NodeIndex chunkUVNodeIndex =
							graph.AddChild(chunkMeshNodeIndex, "UV0", AZStd::move(chunkUV));
						graph.MakeEndPoint(chunkUVNodeIndex);

						// for the convex vertices
						AZStd::vector<AZStd::vector<AZ::Vector3>> collisionHull;
						for (uint32_t hullIndex = result->collisionHullOffset[chunkIndex]; hullIndex < result->collisionHullOffset[chunkIndex + 1]; ++hullIndex)
						{
							AZStd::vector<AZ::Vector3> collisionHullVertices;
							Nv::Blast::CollisionHull* nvHull = result->collisionHull[hullIndex];
							for (uint32_t pointIndex = 0; pointIndex < nvHull->pointsCount; ++pointIndex)
							{
                                collisionHullVertices.push_back(Convert(nvHull->points[pointIndex]));
							}
                            collisionHull.push_back(collisionHullVertices);
						}
						assetData->AppendCollisionHull(collisionHull);
					}

					const AZStd::string assetName = AZStd::string::format("%s_%s_blast",
						graph.GetNodeName(nodeIndex).GetName(),
						meshGroup.GetName().c_str());
					const NodeIndex blastAssetNodeIndex =
						graph.AddChild(nodeIndex, assetName.c_str(), AZStd::move(assetData));

					unfracturedPropertyMap[FracturedMeshPropertyMapKey] =
						AZStd::make_any<NodeIndex>(blastAssetNodeIndex);

					bondGenerator->release();
					fractureTool->release();
					NvBlastExtAuthoringReleaseAuthoringResultCollision(collisionBuilder, result);
					NvBlastExtAuthoringReleaseAuthoringResult(collisionBuilder, result);
				}
			}

			for (auto& chuckMeshGroup : chuckMeshGroups)
			{
				context.GetScene().GetManifest().AddEntry(chuckMeshGroup);
			}

			return AZ::SceneAPI::Events::ProcessingResult::Success;
		}
	}
}