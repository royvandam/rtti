#ifndef RTTI_TEST_TYPEINFO_LINKAGE_HH
#define RTTI_TEST_TYPEINFO_LINKAGE_HH

#include <memory>
#include <rtti.hh>

namespace TypeInfoTest {
    struct Base : virtual RTTI::Enable {
        RTTI_DECLARE_TYPEINFO(Base);
    };

    struct OtherBase : virtual RTTI::Enable {
        RTTI_DECLARE_TYPEINFO(OtherBase);

    public:
        int value = 42;
    };

    struct Child
        : Base
        , OtherBase {
        RTTI_DECLARE_TYPEINFO(Child, Base, OtherBase);
    };

    RTTI::TypeId IntIdFromAnotherTranslationUnit() noexcept;
    RTTI::TypeId ChildIdFromAnotherTranslationUnit() noexcept;
    RTTI::TypeId QualifiedChildIdFromAnotherTranslationUnit() noexcept;
    std::unique_ptr<Base> MakeChildInAnotherTranslationUnit();
    bool IsChildInAnotherTranslationUnit(Base const& object) noexcept;
}  // namespace TypeInfoTest

#endif  // RTTI_TEST_TYPEINFO_LINKAGE_HH
