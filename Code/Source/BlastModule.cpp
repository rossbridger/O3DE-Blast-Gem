
#include <BlastModuleInterface.h>
#include "Components/BlastSystemComponent.h"

namespace Blast
{
    class BlastModule
        : public BlastModuleInterface
    {
    public:
        AZ_RTTI(BlastModule, "{897CCA50-FBAF-4F5A-A859-1951091E0555}", BlastModuleInterface);
        AZ_CLASS_ALLOCATOR(BlastModule, AZ::SystemAllocator);
    };
}// namespace Blast

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), Blast::BlastModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_Blast, Blast::BlastModule)
#endif
