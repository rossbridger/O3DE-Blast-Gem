/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <Blast/BlastFamilyComponentBus.h>

namespace Blast
{
    //! Behavior handler which forwards BlastFamilyComponentNotificationBus events to script canvas.
    class BlastFamilyComponentNotificationBusHandler
        : public BlastFamilyComponentNotificationBus::Handler
        , public AZ::BehaviorEBusHandler
    {
    public:
        AZ_EBUS_BEHAVIOR_BINDER(BlastFamilyComponentNotificationBusHandler,
            "{17C9DB55-8003-4610-B08D-7E369EC4225A}",
            AZ::SystemAllocator,
            OnActorCreated,
            OnActorDestroyed);

        // CollisionNotificationBus
        void OnActorCreated(AZ::EntityId blastActor) override;
        void OnActorDestroyed(AZ::EntityId blastActor) override;
    };
} // namespace Blast
