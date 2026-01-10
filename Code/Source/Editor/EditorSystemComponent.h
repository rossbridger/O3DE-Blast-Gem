/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzToolsFramework/Entity/EditorEntityContextBus.h>
#include <Components/BlastSystemComponent.h>

namespace AzFramework
{
    template <typename AssetType>
    class GenericAssetHandler;
}

namespace Blast
{
    ///
    /// System component for Blast editor
    ///
    class EditorSystemComponent
        : public BlastSystemComponent
        , private AzToolsFramework::EditorEvents::Bus::Handler
    {
    public:
        AZ_COMPONENT(EditorSystemComponent, "{D29CF1A3-5E2C-4385-8541-F7CE78D5FFF8}");
        static void Reflect(AZ::ReflectContext* context);

        EditorSystemComponent() = default;

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
        {
			BlastSystemComponent::GetProvidedServices(provided);
            provided.push_back(AZ_CRC_CE("BlastEditorService"));
        }

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
        {
			BlastSystemComponent::GetRequiredServices(required);
        }

		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
		{
			BlastSystemComponent::GetIncompatibleServices(incompatible);
		}

        AZStd::unique_ptr<AzFramework::GenericAssetHandler<BlastChunksAsset>> m_editorBlastChunksAssetHandler;

        // AZ::Component
        void Activate() override;
        void Deactivate() override;

        // AzToolsFramework::EditorEvents
        void NotifyRegisterViews() override;

        // TickBus
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
    };
} // namespace Blast
