#pragma once

/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <SceneAPI/SceneCore/Components/BehaviorComponent.h>
#include <SceneAPI/SceneCore/Events/ManifestMetaInfoBus.h>
#include <SceneAPI/SceneCore/Events/AssetImportRequest.h>

namespace Blast
{
    namespace Pipeline
    {
        class MeshBehavior
            : public AZ::SceneAPI::SceneCore::BehaviorComponent
            , public AZ::SceneAPI::Events::ManifestMetaInfoBus::Handler
            , public AZ::SceneAPI::Events::AssetImportRequestBus::Handler
        {
        public:
            AZ_COMPONENT(MeshBehavior, "{4183F74F-3486-41F7-B927-491F5B89805B}", AZ::SceneAPI::SceneCore::BehaviorComponent);
            static void Reflect(AZ::ReflectContext* context);

            ~MeshBehavior() override = default;

            // BehaviorComponent overrides ...
            void Activate() override;
            void Deactivate() override;

            // ManifestMetaInfoBus overrides ...
            void GetCategoryAssignments(CategoryRegistrationList& categories, const AZ::SceneAPI::Containers::Scene& scene) override;
            void GetAvailableModifiers(
                AZ::SceneAPI::Events::ManifestMetaInfo::ModifiersList& modifiers,
                const AZ::SceneAPI::Containers::Scene& scene,
                const AZ::SceneAPI::DataTypes::IManifestObject& target) override;
            void InitializeObject(const AZ::SceneAPI::Containers::Scene& scene, AZ::SceneAPI::DataTypes::IManifestObject& target) override;

            // AssetImportRequestBus overrides ...
            AZ::SceneAPI::Events::ProcessingResult UpdateManifest(AZ::SceneAPI::Containers::Scene& scene, ManifestAction action,
                RequestingApplication requester) override;
            void GetPolicyName(AZStd::string& result) const override
            {
                result = "Blast::Pipeline::MeshBehavior";
            }

        private:
            AZ::SceneAPI::Events::ProcessingResult BuildDefault(AZ::SceneAPI::Containers::Scene& scene) const;
            AZ::SceneAPI::Events::ProcessingResult UpdateBlastMeshGroups(AZ::SceneAPI::Containers::Scene& scene) const;

            static constexpr int s_meshBehaviorPreferredTabOrder{ 5 };
        };
    } // Pipeline
} // PhysX
