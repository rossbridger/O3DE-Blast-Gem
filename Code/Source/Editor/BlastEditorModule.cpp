
#include <BlastModuleInterface.h>
#include <Editor/EditorBlastFamilyComponent.h>
#include <Editor/EditorSystemComponent.h>
#include <Pipeline/MeshBehavior.h>
#include <Pipeline/BlastExporter.h>
#include <Pipeline/BlastFractureComponent.h>

namespace Blast
{
    class BlastEditorModule
        : public BlastModuleInterface
    {
    public:
        AZ_RTTI(BlastEditorModule, "{7B661833-1D1A-4415-8546-F323AD50BB2C}", BlastModuleInterface);
        AZ_CLASS_ALLOCATOR(BlastEditorModule, AZ::SystemAllocator);

        BlastEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                EditorSystemComponent::CreateDescriptor(),
                EditorBlastFamilyComponent::CreateDescriptor(),
                Pipeline::MeshBehavior::CreateDescriptor(),
                Pipeline::BlastExporter::CreateDescriptor(),
                Pipeline::BlastFractureComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<EditorSystemComponent>(),
            };
        }
    };
}// namespace Blast

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), Blast::BlastEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_Blast_Editor, Blast::BlastEditorModule)
#endif
