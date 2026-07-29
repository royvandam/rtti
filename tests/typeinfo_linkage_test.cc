#include <rtti.hh>

namespace TypeInfoTest {
    RTTI::TypeId IntIdFromAnotherTranslationUnit() noexcept {
        return RTTI::TypeInfo<int>::Id();
    }
}
