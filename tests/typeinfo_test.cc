#include <gtest/gtest.h>

#include <rtti.hh>
#include <type_traits>
#include <typeinfo>

using namespace RTTI;

namespace TypeInfoTest {
    TypeId IntIdFromAnotherTranslationUnit() noexcept;
}

TEST(TypeInfo, NamesAreAvailableAtCompileTime) {
    static_assert(TypeName<void>() == "void");
    static_assert(TypeName<const void>() == "const void");
    static_assert(TypeName<int>() == "int");
    static_assert(TypeName<bool>() == "bool");
    static_assert(TypeInfo<void>::Name() == "void");
}

TEST(TypeInfo, ConfiguredIdRepresentation) {
#if RTTI_USE_POINTER_TYPE_IDS
    static_assert(std::is_same_v<TypeId, void const*>);
    static_assert(TypeInfo<int>::Id() != nullptr);
    static_assert(TypeInfo<int>::Id() != TypeInfo<bool>::Id());
#else
    static_assert(std::is_same_v<TypeId, std::uint32_t>);
    static_assert(TypeInfo<int>::Id() == Hash::FNV1a("int"));
    static_assert(TypeInfo<void>::Id() == Hash::FNV1a("void"));
#endif
}

TEST(TypeInfo, IdsAgreeAcrossTranslationUnits) {
    EXPECT_EQ(TypeInfo<int>::Id(), TypeInfoTest::IntIdFromAnotherTranslationUnit());
}

TEST(TypeInfo, NamesPreserveTypeQualifiers) {
    static_assert(TypeName<int>() != TypeName<const int>());
    static_assert(TypeName<int>() != TypeName<int&>());
    static_assert(TypeName<int>() != TypeName<int*>());
    static_assert(TypeName<int*>() != TypeName<const int*>());
    static_assert(TypeName<void>() != TypeName<void (*)()>());
}

TEST(TypeInfo, SameTypesAreEqual) {
    EXPECT_EQ(typeid(int), typeid(int));
    EXPECT_EQ(TypeInfo<int>::Id(), TypeInfo<int>::Id());
}

TEST(TypeInfo, DifferentTypesAreNotEqual) {
    EXPECT_NE(typeid(int), typeid(bool));
    EXPECT_NE(TypeInfo<int>::Id(), TypeInfo<bool>::Id());
}

TEST(TypeInfo, TypesAndPointerToTypesAreNotEqual) {
    EXPECT_NE(typeid(int), typeid(int*));
    EXPECT_NE(TypeInfo<int>::Id(), TypeInfo<int*>::Id());
}

TEST(TypeInfo, PointerAndConstPointerTypesAreNotEqual) {
    EXPECT_NE(typeid(int*), typeid(int const*));
    EXPECT_NE(TypeInfo<int>::Id(), TypeInfo<int const*>::Id());
}

TEST(TypeInfo, NonConstAndConstTypesAreEqual) {
    EXPECT_EQ(typeid(int), typeid(int const));
    EXPECT_EQ(TypeInfo<int>::Id(), TypeInfo<int const>::Id());

    EXPECT_EQ(typeid(int*), typeid(int* const));
    EXPECT_EQ(TypeInfo<int*>::Id(), TypeInfo<int* const>::Id());
}

TEST(TypeInfo, TypesAndReferencesToTypesAreEqual) {
    EXPECT_EQ(typeid(int), typeid(int&));
    EXPECT_EQ(TypeInfo<int>::Id(), TypeInfo<int&>::Id());

    EXPECT_EQ(typeid(int), typeid(int&&));
    EXPECT_EQ(TypeInfo<int>::Id(), TypeInfo<int&&>::Id());
}
