#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Memory/Memory.h>
#include <AzCore/RTTI/RTTI.h>
#include <SceneAPI/SceneCore/Components/GenerationComponent.h>
#include <SceneAPI/SceneCore/Containers/SceneGraph.h>
#include <SceneAPI/SceneCore/Events/ProcessingResult.h>
#include <SceneAPI/SceneCore/Events/GenerateEventContext.h>
#include <SceneAPI/SceneData/GraphData/MeshData.h>

namespace Blast
{
	namespace Pipeline
	{
		class BlastFractureComponent
			: public AZ::SceneAPI::SceneCore::GenerationComponent
		{
		public:
			AZ_COMPONENT(BlastFractureComponent, "{09A57F5A-3B66-4542-98A0-E0E94451D471}", AZ::SceneAPI::SceneCore::GenerationComponent);

			static void Reflect(AZ::ReflectContext* context);

			BlastFractureComponent();

			AZ::SceneAPI::Events::ProcessingResult FractureMeshes(AZ::SceneAPI::Events::GenerateEventContext& context) const;
		};
	}
}