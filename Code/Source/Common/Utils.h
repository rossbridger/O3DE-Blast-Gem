/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/Math/Quaternion.h>
#include <AzCore/Math/Vector2.h>
#include <AzCore/Math/Vector3.h>
#include <NvBlastGlobals.h>

namespace Blast
{
    inline AZ::Vector2 Convert(const NvcVec2 nvcVec2)
    {
        return AZ::Vector2(nvcVec2.x, nvcVec2.y);
    }

    inline NvcVec2 Convert(const AZ::Vector2 azVec2)
    {
        return NvcVec2{ azVec2.GetX(), azVec2.GetY() };
    }

    inline AZ::Vector3 Convert(const NvcVec3 nvcVec3)
    {
        return AZ::Vector3(nvcVec3.x, nvcVec3.y, nvcVec3.z);
    }

    inline NvcVec3 Convert(const AZ::Vector3 azVec3)
    {
        return NvcVec3{ azVec3.GetX(), azVec3.GetY(), azVec3.GetZ() };
    }

    inline NvcQuat Convert(const AZ::Quaternion azQuat)
    {
        return NvcQuat{ azQuat.GetX(), azQuat.GetY(), azQuat.GetZ(), azQuat.GetW() };
    }
} // namespace Blast
