/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <Actor/BlastActorFactory.h>
#include <Asset/BlastAsset.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/std/containers/unordered_set.h>
#include <Actor/BlastActor.h>
#include <Blast/BlastActorConfiguration.h>
#include <Blast/BlastDebug.h>
#include <Material/BlastMaterial.h>
#include <Common/BlastInterfaces.h>
#include <Family/ActorTracker.h>
#include <Family/BlastFamily.h>
#include <NvBlastTkEvent.h>

struct NvBlastExtMaterial;

namespace Nv
{
    namespace Blast
    {
        struct TkEvent;
        class TkFamily;
        class TkGroup;
        struct TkSplitEvent;
    } // namespace Blast
} // namespace Nv

namespace Blast
{
    class BlastChunkDataContainer;
    //! Set of options used to create a Blast Family
    struct BlastFamilyDesc
    {
        BlastAsset& m_asset; //!< Blast asset to create from.
        BlastListener* m_listener = nullptr; //!< Blast listener to notify about actor creations/destructions, this would generally be BlastFamilyComponent instance.
        Nv::Blast::TkGroup* m_group = nullptr; //!< if not nullptr created TkActor (and TkFamily) will be placed in this group
        Physics::MaterialId m_physicsMaterialId;
        const Material* m_blastMaterial = nullptr;
        AZStd::shared_ptr<BlastActorFactory> m_actorFactory;
        const BlastActorConfiguration& m_actorConfiguration;
        const BlastChunkDataContainer* m_chunkContainer = nullptr;
    };

    //! Listens to events from the Blast toolkit family and manages BlastActors for a destructible object.
    class BlastFamily
        : public Nv::Blast::TkEventListener
    {
    public:
        //! Class allocator to use aznew
        AZ_CLASS_ALLOCATOR(BlastFamily, AZ::SystemAllocator);

        //! Constructor for the BlastFamily
        //! @param desc Creation descriptor
        static AZStd::unique_ptr<BlastFamily> Create(const BlastFamilyDesc& desc);

        //! Spawns this BlastFamily, creating the initial actors.
        //! @param transform Initial transform of the destructible object.
        BlastFamily(const BlastFamilyDesc& desc);

        //! Despawns this BlastFamily, destroying all created actors.
        virtual ~BlastFamily();

        //! Spawns this BlastFamily, creating the initial actors.
        //! @param transform Initial transform of the destructible object.
        bool Spawn(const AZ::Transform& transform);
        //! Despawns this BlastFamily, destroying all created actors.
        void Despawn();

        void HandleEvents(const Nv::Blast::TkEvent* events, uint32_t eventCount);
        void DestroyActor(BlastActor* blastActor);

        ActorTracker& GetActorTracker();
        const Nv::Blast::TkFamily* GetTkFamily() const;
        Nv::Blast::TkFamily* GetTkFamily();
        const Nv::Blast::TkAsset& GetTkAsset() const;
        const BlastActorConfiguration& GetActorConfiguration() const;

        const BlastChunkDataContainer* GetChunkDataContainer() const;

        void FillDebugRender(DebugRenderBuffer& debugRenderBuffer, DebugRenderMode mode, float renderScale);

        //! TkEventListener interface.
        void receive(const Nv::Blast::TkEvent* events, uint32_t eventCount);

    private:
        void CreateActors(const AZStd::vector<BlastActorDesc>& tkActors);
        void DestroyActors(const AZStd::unordered_set<BlastActor*>& actors);

        void DispatchActorCreated(const BlastActor& actor);
        void DispatchActorDestroyed(const BlastActor& actor);

        AZStd::vector<BlastActorDesc> CalculateActorsDescFromFamily(const AZ::Transform& transform);

        void HandleSplitEvent(
            const Nv::Blast::TkSplitEvent* splitEvent, AZStd::vector<BlastActorDesc>& newActorsDesc,
            AZStd::unordered_set<BlastActor*>& actorsToDelete);

        // Calculates actor description for an actor that has a parent.
        BlastActorDesc CalculateActorDesc(
            AzPhysics::SimulatedBody* parentBody, bool parentStatic, AZ::Transform parentTransform,
            Nv::Blast::TkActor* tkActor);

        // Calculates actor description for an actor that does not have a parent.
        BlastActorDesc CalculateActorDesc(const AZ::Transform& transform, Nv::Blast::TkActor* tkActor);

        void FillDebugRenderHealthGraph(
            DebugRenderBuffer& debugRenderBuffer, DebugRenderMode                                                                                                                                                                                                                                                                                                                                                                                mode, const Nv::Blast::TkActor& actor);

        const BlastAsset& m_asset;
        ActorTracker m_actorTracker;
        AZStd::unique_ptr<Nv::Blast::TkFamily> m_tkFamily;
        AZStd::shared_ptr<BlastActorFactory> m_actorFactory;
        BlastListener* m_listener;

        Physics::MaterialId m_physicsMaterialId;
        const Material* m_blastMaterial = nullptr;
        const BlastChunkDataContainer* m_chunkContainer = nullptr;
        const BlastActorConfiguration& m_actorConfiguration;
        AZ::Transform m_initialTransform;
        bool m_isSpawned;
    };
} // namespace Blast
