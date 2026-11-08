/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Asset/BlastChunksAsset.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <Editor/EditorSystemComponent.h>
#include <Editor/EditorWindow.h>
#include <FracturingTool/FracturingToolWindow.h>
#include <DebugDraw/DebugDrawBus.h>
#include <AzFramework/Asset/GenericAssetHandler.h>

namespace Blast
{
    void EditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        BlastChunksAsset::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<EditorSystemComponent, BlastSystemComponent>()->Version(1);
        }
    }

    void EditorSystemComponent::Activate()
    {
		BlastSystemComponent::Activate();
        m_editorBlastChunksAssetHandler = AZStd::make_unique<AzFramework::GenericAssetHandler<BlastChunksAsset>>(
            "Blast Chucks", "Others", "blastchunks");
        m_editorBlastChunksAssetHandler->Register();

        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void EditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        m_editorBlastChunksAssetHandler.reset();
		BlastSystemComponent::Deactivate();
    }

    // This will be called when the IEditor instance is ready
    void EditorSystemComponent::NotifyRegisterViews()
    {
        Blast::Editor::EditorWindow::RegisterViewClass();
        Blast::Editor::FracturingToolWindow::RegisterViewClass();
    }
    void Blast::EditorSystemComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
    {
        BlastSystemComponent::OnTick(deltaTime, time);
        if (m_debugRenderMode)
        {
            AZ_PROFILE_SCOPE(Physics, "BlastSystemComponent::OnTick::DebugRender");
            DebugRenderBuffer buffer;
            BlastFamilyComponentRequestBus::Broadcast(
                &BlastFamilyComponentRequests::FillDebugRenderBuffer, buffer, m_debugRenderMode);

            // This is a system component, and thus is not associated with a specific scene, so use the bootstrap scene
            // for the debug drawing
            const auto mainScene = AZ::RPI::RPISystemInterface::Get()->GetSceneByName(AZ::Name("Main"));
            if (mainScene)
            {
                for (DebugLine& line : buffer.m_lines)
                {
                    DebugDraw::DebugDrawRequestBus::Broadcast(&DebugDraw::DebugDrawRequests::DrawLineLocationToLocation, line.m_p0, line.m_p1, line.m_color, 0.0f);
                }
            }
        }
    }
} // namespace Blast
