/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Interface/Interface.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzToolsFramework/API/ViewPaneOptions.h>
#include <LyViewPaneNames.h>

#include <FracturingTool/FracturingToolWindow.h>

#include <Blast/BlastSystemBus.h>

namespace Blast
{
    namespace Editor
    {
        FracturingToolWindow::FracturingToolWindow(QWidget* parent)
            : QWidget(parent)
            //, m_ui(new Ui::FracturingToolWindowClass())
        {
            //m_ui->setupUi(this);
            resize(800, 600);
            setWindowTitle("Blast Fracturing Tool (Preview)");
            // const BlastGlobalConfiguration& configuration =
            //     AZ::Interface<Blast::BlastSystemRequests>::Get()->GetGlobalConfiguration();

            // m_ui->m_BlastConfigurationWidget->SetConfiguration(configuration);
            // connect(
            //     m_ui->m_BlastConfigurationWidget, &Blast::Editor::ConfigurationWidget::onConfigurationChanged, this,
            //     &EditorWindow::SaveConfiguration);
        }

        void FracturingToolWindow::RegisterViewClass()
        {
            AzToolsFramework::ViewPaneOptions options;
            options.preferedDockingArea = Qt::LeftDockWidgetArea;
            options.saveKeyName = "BlastFracturingTool";
            options.isPreview = false;
            AzToolsFramework::RegisterViewPane<FracturingToolWindow>(
                "Blast Fracturing Tool (Preview)", LyViewPane::CategoryTools, options);
        }
/* 
        void FracturingToolWindow::SaveConfiguration(const BlastGlobalConfiguration& configuration)
        {
            AZ::Interface<Blast::BlastSystemRequests>::Get()->SetGlobalConfiguration(configuration);
        } */
    } // namespace Editor
} // namespace Blast
