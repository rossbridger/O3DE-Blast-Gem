
#include "BlastModuleInterface.h"
#include <AzCore/Memory/Memory.h>

#include <Components/BlastSystemComponent.h>
#include <Components/BlastFamilyComponent.h>
#include <Components/BlastSystemComponent.h>

namespace Blast
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(BlastModuleInterface,
        "BlastModuleInterface", "{3CC50049-FB9E-4A5A-99D5-45052F776080}");
    AZ_RTTI_NO_TYPE_INFO_IMPL(BlastModuleInterface, AZ::Module);
    AZ_CLASS_ALLOCATOR_IMPL(BlastModuleInterface, AZ::SystemAllocator);

    BlastModuleInterface::BlastModuleInterface()
    {
        // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
        // Add ALL components descriptors associated with this gem to m_descriptors.
        // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
        // This happens through the [MyComponent]::Reflect() function.
		m_descriptors.insert(m_descriptors.end(), {
            BlastFamilyComponent::CreateDescriptor(),
            BlastSystemComponent::CreateDescriptor(),
            });
    }

    AZ::ComponentTypeList BlastModuleInterface::GetRequiredSystemComponents() const
    {
        return AZ::ComponentTypeList {
            azrtti_typeid<BlastSystemComponent>(),
        };
    }
} // namespace Blast
