/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Components/BlastSystemComponent.h>

#include <Asset/BlastAsset.h>
#include <Asset/BlastAssetHandler.h>
#include <Atom/RPI.Public/AuxGeom/AuxGeomDraw.h>
#include <Atom/RPI.Public/Scene.h>
#include <AzCore/Jobs/JobCompletion.h>
#include <AzCore/Jobs/JobFunction.h>
#include <AzCore/Memory/Memory.h>
#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <Blast/BlastDebug.h>
#include <Blast/BlastFamilyComponentBus.h>
#include <Material/BlastMaterialAsset.h>
#include <AzCore/Console/IConsole.h>
#include <NvBlastExtTkSerialization.h>
#include <NvBlastTkJoint.h>
#include <Common/BlastChunkData.h>

namespace Blast
{

    static const char* const DefaultConfigurationPath = "default.blastconfiguration";

    void BlastSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        BlastGlobalConfiguration::Reflect(context);

        MaterialConfiguration::Reflect(context);
        MaterialAsset::Reflect(context);
        BlastChunkData::Reflect(context);

        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<BlastSystemComponent, AZ::Component>()->Version(1);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<BlastSystemComponent>("Blast", "Adds support for the NVIDIA Blast destruction system")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
            }
        }
    }

    void BlastGlobalConfiguration::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<BlastGlobalConfiguration>()
                ->Version(2)
                ->Field("StressSolverIterations", &BlastGlobalConfiguration::m_stressSolverIterations);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<BlastGlobalConfiguration>(
                    "Blast global configuration", "Set of configuration that are applied globally within Blast gem.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &BlastGlobalConfiguration::m_stressSolverIterations,
                        "Stress solver iterations",
                        "Number of iterations stress solver on each family runs for each tick.")
                    ->Attribute(AZ::Edit::Attributes::Min, 0)
                    ->Attribute(AZ::Edit::Attributes::Max, 50000);
            }
        }
    }

    void BlastSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("BlastService"));
    }

    void BlastSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("BlastService"));
    }

    void BlastSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("PhysicsService"));
        required.push_back(AZ_CRC_CE("AssetCatalogService"));
        required.push_back(AZ_CRC_CE("AssetDatabaseService"));
    }

    void BlastSystemComponent::Init()
    {
        // Route NvBlast allocations through the AZ system allocator
        NvBlastGlobalSetAllocatorCallback(&m_blastAllocatorCallback);
        NvBlastGlobalSetErrorCallback(&m_blastErrorCallback);
        NvBlastGlobalSetProfilerCallback(&m_blastProfilerCallback);
        m_debugRenderMode = DebugRenderDisabled;
    }

    void BlastSystemComponent::Activate()
    {
        AZ_PROFILE_FUNCTION(Physics);
        auto blastAssetHandler = aznew BlastAssetHandler();
        blastAssetHandler->Register();
        m_assetHandlers.emplace_back(blastAssetHandler);

        auto materialAsset = aznew AzFramework::GenericAssetHandler<MaterialAsset>(
            "Blast Material", "Blast Material", "blastmaterial");
        materialAsset->Register();
        m_assetHandlers.emplace_back(materialAsset);

        // Add asset types and extensions to AssetCatalog. Uses "AssetCatalogService".
        auto assetCatalog = AZ::Data::AssetCatalogRequestBus::FindFirstHandler();
        if (assetCatalog)
        {
            assetCatalog->EnableCatalogForAsset(AZ::AzTypeInfo<BlastAsset>::Uuid());
            assetCatalog->AddExtension("blast");
        }

        m_registered = false;

        BlastSystemRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();

        InitPhysics();
        LoadConfiguration();
    }

    void BlastSystemComponent::Deactivate()
    {
        AZ_PROFILE_FUNCTION(Physics);
        AZ::TickBus::Handler::BusDisconnect();
        BlastSystemRequestBus::Handler::BusDisconnect();

        SaveConfiguration();
        DeactivatePhysics();

        m_assetHandlers.clear();
    };

    void BlastSystemComponent::InitPhysics()
    {
        // Create blast singletons
        m_tkFramework = NvBlastTkFrameworkCreate();
        m_extSerialization = NvBlastExtSerializationCreate();

        if (m_extSerialization != nullptr)
        {
            NvBlastExtTkSerializerLoadSet(*m_tkFramework, *m_extSerialization);
        }

        NvBlastGlobalSetProfilerCallback(&m_blastProfilerCallback);
    }

    void BlastSystemComponent::DeactivatePhysics()
    {
        m_extSerialization->release();
        m_tkFramework->release();
    }

    void BlastSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        AZ_PROFILE_FUNCTION(Physics);

        AZ::JobCompletion jobCompletion;

        BlastFamilyComponentRequestBus::EnumerateHandlers(
            [&jobCompletion](BlastFamilyComponentRequests* handler)
            {
                auto stressDamageJob = AZ::CreateJobFunction(
                    [handler]() -> void
                    {
                        handler->ApplyStressDamage();
                    },
                    true);
                stressDamageJob->SetDependent(&jobCompletion);
                stressDamageJob->Start();

                return true;
            });

        BlastFamilyComponentRequestBus::EnumerateHandlers(
            [](BlastFamilyComponentRequests* handler)
            {
                handler->SyncMeshes();

                return true;
            });

        jobCompletion.StartAndWaitForCompletion();

        // Remove empty groups
        for (size_t i = 0; i < m_groups.size();)
        {
            if (m_groups[i]->getActorCount() == 0)
            {
                AZStd::swap(m_groups[i], m_groups.back());
                m_groups.pop_back();
            }
            else
            {
                ++i;
            }
        }

        // Clean up damage descriptions and program params now that groups have run.
        {
            AZ_PROFILE_SCOPE(Physics, "BlastSystemComponent::OnTick::Cleanup");
            m_radialDamageDescs.clear();
            m_capsuleDamageDescs.clear();
            m_shearDamageDescs.clear();
            m_triangleDamageDescs.clear();
            m_impactDamageDescs.clear();
            m_programParams.clear();
        }
    }

    void BlastSystemComponent::LoadConfiguration()
    {
        BlastGlobalConfiguration globalConfiguration;
        [[maybe_unused]] bool loaded = AZ::Utils::LoadObjectFromFileInPlace<BlastGlobalConfiguration>(
            DefaultConfigurationPath, globalConfiguration);
        AZ_Warning("Blast", loaded, "Failed to load Blast configuration, initializing with default configs.");

        ApplyGlobalConfiguration(globalConfiguration);
        if (!loaded)
        {
            SaveConfiguration();
        }
    }

    void BlastSystemComponent::SaveConfiguration()
    {
        auto assetRoot = AZ::IO::FileIOBase::GetInstance()->GetAlias("@projectroot@");

        if (!assetRoot)
        {
            return;
        }

        AZStd::string fullPath;
        AzFramework::StringFunc::Path::Join(assetRoot, DefaultConfigurationPath, fullPath);

        [[maybe_unused]] bool saved = AZ::Utils::SaveObjectToFile<BlastGlobalConfiguration>(
            fullPath.c_str(), AZ::DataStream::ST_JSON, &m_configuration);
        AZ_Warning("BlastSystemComponent", saved, "Failed to save Blast configuration");
    }

    Nv::Blast::TkFramework* BlastSystemComponent::GetTkFramework() const
    {
        return m_tkFramework;
    }

    Nv::Blast::ExtSerialization* BlastSystemComponent::GetExtSerialization() const
    {
        return m_extSerialization;
    }

    Nv::Blast::TkGroup* BlastSystemComponent::CreateTkGroup()
    {
        m_groups.emplace_back();
        Nv::Blast::TkGroupDesc groupDesc;
        groupDesc.workerCount = AZStd::thread::hardware_concurrency();
        m_groups.back().reset(m_tkFramework->createGroup(groupDesc));

        return m_groups.back().get();
    }

    void BlastSystemComponent::AddDamageDesc(AZStd::unique_ptr<NvBlastExtRadialDamageDesc> desc)
    {
        m_radialDamageDescs.push_back(AZStd::move(desc));
    }

    void BlastSystemComponent::AddDamageDesc(AZStd::unique_ptr<NvBlastExtCapsuleRadialDamageDesc> desc)
    {
        m_capsuleDamageDescs.push_back(AZStd::move(desc));
    }

    void BlastSystemComponent::AddDamageDesc(AZStd::unique_ptr<NvBlastExtShearDamageDesc> desc)
    {
        m_shearDamageDescs.push_back(AZStd::move(desc));
    }

    void BlastSystemComponent::AddDamageDesc(AZStd::unique_ptr<NvBlastExtTriangleIntersectionDamageDesc> desc)
    {
        m_triangleDamageDescs.push_back(AZStd::move(desc));
    }

    void BlastSystemComponent::AddDamageDesc(AZStd::unique_ptr<NvBlastExtImpactSpreadDamageDesc> desc)
    {
        m_impactDamageDescs.push_back(AZStd::move(desc));
    }

    void BlastSystemComponent::AddProgramParams(AZStd::unique_ptr<NvBlastExtProgramParams> program)
    {
        m_programParams.emplace_back(AZStd::move(program));
    }

    const BlastGlobalConfiguration& BlastSystemComponent::GetGlobalConfiguration() const
    {
        return m_configuration;
    }

    void BlastSystemComponent::SetGlobalConfiguration(const BlastGlobalConfiguration& globalConfiguration)
    {
        ApplyGlobalConfiguration(globalConfiguration);
        SaveConfiguration();
    }

    void BlastSystemComponent::ApplyGlobalConfiguration(const BlastGlobalConfiguration& globalConfiguration)
    {
        m_configuration = globalConfiguration;
    }

    void* BlastSystemComponent::AZBlastProfilerCallback::zoneStart(const char* eventName, [[maybe_unused]] bool detached, [[maybe_unused]] uint64_t contextId)
    {
        AZ_PROFILE_BEGIN(Physics, eventName);
        return nullptr;
    }

    void BlastSystemComponent::AZBlastProfilerCallback::zoneEnd([[maybe_unused]] void* profilerData, [[maybe_unused]] const char* eventName, [[maybe_unused]] bool detached, [[maybe_unused]] uint64_t contextId)
    {
        AZ_PROFILE_END(Physics);
    }

    static void CmdToggleBlastDebugVisualization(const int& blast_debug)
    {
		auto userPreference = static_cast<DebugRenderMode>(blast_debug);
		BlastSystemRequestBus::Broadcast(&BlastSystemRequests::SetDebugRenderMode, userPreference);
    }

    AZ_CVAR(int, blast_debug, 0, CmdToggleBlastDebugVisualization, AZ::ConsoleFunctorFlags::Null, "Enable blast debug draw");

    void BlastSystemComponent::SetDebugRenderMode(DebugRenderMode debugRenderMode)
    {
        m_debugRenderMode = debugRenderMode;
    }
    void BlastSystemComponent::AZBlastErrorCallback::reportError(nvidia::NvErrorCode::Enum code, const char* message, const char* file, int line)
    {
        AZ_Error("Blast", false, "Blast error code %u: %s (in %s line %d)", static_cast<uint32_t>(code), message, file, line);
    }
} // namespace Blast
