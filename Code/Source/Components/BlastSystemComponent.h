/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <Atom/RPI.Public/AuxGeom/AuxGeomFeatureProcessorInterface.h>

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <Blast/BlastDebug.h>
#include <Blast/BlastSystemBus.h>
#include <NvBlastExtDamageShaders.h>
#include <NvBlastExtSerialization.h>
#include <NvBlastGlobals.h>
#include <NvBlastTkFramework.h>
#include <NvBlastTkGroup.h>

namespace Blast
{
    class BlastSystemComponent
        : public AZ::Component
        , BlastSystemRequestBus::Handler
        , public AZ::Interface<Blast::BlastSystemRequests>::Registrar
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(BlastSystemComponent, "{9705144A-FF10-45CE-AA3D-3E1F43872429}");
        BlastSystemComponent() = default;
        BlastSystemComponent(const BlastSystemComponent&) = delete;
        BlastSystemComponent& operator=(const BlastSystemComponent&) = delete;

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        // AZTickBus interface implementation
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        // Blast configuration methods
        void LoadConfiguration();
        void SaveConfiguration();

        // Getters for physx/NvBlast structures
        Nv::Blast::TkFramework* GetTkFramework() const override;
        Nv::Blast::ExtSerialization* GetExtSerialization() const override;
        Nv::Blast::TkGroup* CreateTkGroup() override;

        void AddDamageDesc(AZStd::unique_ptr<NvBlastExtRadialDamageDesc> desc) override;
        void AddDamageDesc(AZStd::unique_ptr<NvBlastExtCapsuleRadialDamageDesc> desc) override;
        void AddDamageDesc(AZStd::unique_ptr<NvBlastExtShearDamageDesc> desc) override;
        void AddDamageDesc(AZStd::unique_ptr<NvBlastExtTriangleIntersectionDamageDesc> desc) override;
        void AddDamageDesc(AZStd::unique_ptr<NvBlastExtImpactSpreadDamageDesc> desc) override;
        void AddProgramParams(AZStd::unique_ptr<NvBlastExtProgramParams> program) override;

        const BlastGlobalConfiguration& GetGlobalConfiguration() const override;
        void SetGlobalConfiguration(const BlastGlobalConfiguration& materialLibrary) override;
        void SetDebugRenderMode(DebugRenderMode debugRenderMode) override;

    protected:
        void InitPhysics();
        void DeactivatePhysics();

        void ApplyGlobalConfiguration(const BlastGlobalConfiguration& materialLibrary);

        // Internal helper functions & classes
        class AZBlastAllocatorCallback : public nvidia::NvAllocatorCallback
        {
        public:
            // Blast requires 16-byte alignment
            static const size_t alignment = 16;

            void* allocate(
                size_t size,
                [[maybe_unused]] const char* typeName,
                [[maybe_unused]] const char* filename,
                [[maybe_unused]] int line) override
            {
                return azmalloc(size, alignment);
            }

            void deallocate(void* ptr) override
            {
                azfree(ptr);
            }
        };

        class AZBlastProfilerCallback : public nvidia::NvProfilerCallback
        {
        public:
            void* zoneStart(const char* eventName, bool detached, uint64_t contextId) override;
            void zoneEnd(void* profilerData, const char* eventName, bool detached, uint64_t contextId) override;
        };

        class AZBlastErrorCallback : public nvidia::NvErrorCallback
        {
        public:
            void reportError(nvidia::NvErrorCode::Enum code, const char* message, const char* file, int line) override;
        };

        AZBlastAllocatorCallback m_blastAllocatorCallback;
        AZBlastProfilerCallback m_blastProfilerCallback;
        AZBlastErrorCallback m_blastErrorCallback;

        AZStd::vector<AZStd::unique_ptr<Nv::Blast::TkGroup>> m_groups;

        // Container for asset types that need to be registered
        AZStd::vector<AZStd::unique_ptr<AZ::Data::AssetHandler>> m_assetHandlers;

        // Blast framework & physics singletons, in order of initialization
        Nv::Blast::TkFramework* m_tkFramework;
        Nv::Blast::ExtSerialization* m_extSerialization;

        // Library for blast materials and other global configurations
        BlastGlobalConfiguration m_configuration;

        // Storage for damage info that gets simulated.
        AZStd::vector<AZStd::unique_ptr<NvBlastExtRadialDamageDesc>> m_radialDamageDescs;
        AZStd::vector<AZStd::unique_ptr<NvBlastExtCapsuleRadialDamageDesc>> m_capsuleDamageDescs;
        AZStd::vector<AZStd::unique_ptr<NvBlastExtShearDamageDesc>> m_shearDamageDescs;
        AZStd::vector<AZStd::unique_ptr<NvBlastExtTriangleIntersectionDamageDesc>> m_triangleDamageDescs;
        AZStd::vector<AZStd::unique_ptr<NvBlastExtImpactSpreadDamageDesc>> m_impactDamageDescs;
        AZStd::vector<AZStd::unique_ptr<NvBlastExtProgramParams>> m_programParams;

        bool m_registered;
        DebugRenderMode m_debugRenderMode;
    };
} // namespace Blast
