/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Components/BlastFamilyComponentNotificationBusHandler.h>

namespace Blast
{
    void BlastFamilyComponentNotificationBusHandler::OnActorCreated(AZ::EntityId entityId)
    {
        Call(FN_OnActorCreated, entityId);
    }

    void BlastFamilyComponentNotificationBusHandler::OnActorDestroyed(AZ::EntityId entityId)
    {
        Call(FN_OnActorDestroyed, entityId);
    }
} // namespace Blast
