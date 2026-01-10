/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Math/Vector3.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>
#include <AzFramework/Physics/Material/PhysicsMaterialSlots.h>

#include <SceneAPI/SceneCore/Components/ExportingComponent.h>

namespace AZ
{
    namespace SceneAPI
    {
        namespace Events
        {
            class ExportEventContext;
        }

        namespace Containers
        {
            class Scene;
            class SceneGraph;
        }

        namespace DataTypes
        {
            class IMeshData;
        }
    }
}

namespace Blast
{
    namespace Pipeline
    {
        class MeshGroup;

        class BlastExporter
            : public AZ::SceneAPI::SceneCore::ExportingComponent
        {
        public:
            AZ_COMPONENT(BlastExporter, "{25F1FE79-0ADB-4E5C-BAB1-64FFACC27D7D}", AZ::SceneAPI::SceneCore::ExportingComponent);

            BlastExporter();
            ~BlastExporter() override = default;

            static void Reflect(AZ::ReflectContext* context);

            AZ::SceneAPI::Events::ProcessingResult ProcessContext(AZ::SceneAPI::Events::ExportEventContext& context) const;
        };
    } // namespace Pipeline
} // namespace PhysX
