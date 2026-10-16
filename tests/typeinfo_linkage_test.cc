#include "typeinfo_linkage.hh"

namespace TypeInfoTest {
    RTTI::TypeId IntIdFromAnotherTranslationUnit() noexcept {
        return RTTI::TypeInfo<int>::Id();
    }

    RTTI::TypeId ChildIdFromAnotherTranslationUnit() noexcept {
        return Child::TypeInfo::Id();
    }

    RTTI::TypeId QualifiedChildIdFromAnotherTranslationUnit() noexcept {
        return RTTI::TypeInfo<Child const&>::Id();
    }

    std::unique_ptr<Base> MakeChildInAnotherTranslationUnit() {
        return std::make_unique<Child>();
    }

    bool IsChildInAnotherTranslationUnit(Base const& object) noexcept {
        return object.is<Child>() && object.cast<Child>() != nullptr;
    }
}
