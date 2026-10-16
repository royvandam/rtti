#include <gtest/gtest.h>

#include <rtti.hh>
#include <type_traits>
#include <utility>

#include "typeinfo_linkage.hh"

using namespace RTTI;

namespace FirstNamespace {
    struct SameName {};
}  // namespace FirstNamespace

namespace SecondNamespace {
    struct SameName {};
}  // namespace SecondNamespace

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
    static_assert(TypeInfo<int>::Id() == 0x95e97e5eu);
    static_assert(TypeInfo<void>::Id() == 0x48b5725fu);
#endif
}

TEST(TypeInfo, IdsAgreeAcrossTranslationUnits) {
    EXPECT_EQ(TypeInfo<int>::Id(), TypeInfoTest::IntIdFromAnotherTranslationUnit());
    EXPECT_EQ(TypeInfoTest::Child::TypeInfo::Id(),
              TypeInfoTest::ChildIdFromAnotherTranslationUnit());
    EXPECT_EQ(TypeInfoTest::Child::TypeInfo::Id(),
              TypeInfoTest::QualifiedChildIdFromAnotherTranslationUnit());
}

TEST(TypeInfo, CastsObjectsCreatedInAnotherTranslationUnit) {
    auto object = TypeInfoTest::MakeChildInAnotherTranslationUnit();
    ASSERT_NE(object, nullptr);
    EXPECT_EQ(object->typeId(), TypeInfoTest::Child::TypeInfo::Id());
    EXPECT_TRUE(object->is<TypeInfoTest::Child>());
    auto* child = object->cast<TypeInfoTest::Child>();
    ASSERT_NE(child, nullptr);
    EXPECT_EQ(object->cast<TypeInfoTest::OtherBase>(),
              static_cast<TypeInfoTest::OtherBase*>(child));
    EXPECT_EQ(std::as_const(*object).cast<const TypeInfoTest::Child>(), child);
    ASSERT_NE(object->cast<TypeInfoTest::OtherBase>(), nullptr);
    EXPECT_EQ(object->cast<TypeInfoTest::OtherBase>()->value, 42);
}

TEST(TypeInfo, InspectsLocalObjectsInAnotherTranslationUnit) {
    TypeInfoTest::Child child;
    EXPECT_TRUE(TypeInfoTest::IsChildInAnotherTranslationUnit(child));
    TypeInfoTest::Base base;
    EXPECT_FALSE(TypeInfoTest::IsChildInAnotherTranslationUnit(base));
}

TEST(TypeInfo, NamesPreserveTypeQualifiers) {
    static_assert(TypeName<int>() != TypeName<const int>());
    static_assert(TypeName<int>() != TypeName<int&>());
    static_assert(TypeName<int>() != TypeName<int*>());
    static_assert(TypeName<int*>() != TypeName<const int*>());
    static_assert(TypeName<void>() != TypeName<void (*)()>());
}

TEST(TypeInfo, NamesAndIdsDistinguishNamespacesAndCompoundTypes) {
    static_assert(TypeName<FirstNamespace::SameName>() != TypeName<SecondNamespace::SameName>());
    EXPECT_NE(TypeInfo<FirstNamespace::SameName>::Id(), TypeInfo<SecondNamespace::SameName>::Id());
    EXPECT_NE(TypeInfo<int[2]>::Id(), TypeInfo<int[3]>::Id());
    EXPECT_NE(TypeInfo<void (*)()>::Id(), TypeInfo<void (*)(int)>::Id());
}

TEST(TypeInfo, SameTypesAreEqual) {
    EXPECT_EQ(TypeInfo<int>::Id(), TypeInfo<int>::Id());
}

TEST(TypeInfo, DifferentTypesAreNotEqual) {
    EXPECT_NE(TypeInfo<int>::Id(), TypeInfo<bool>::Id());
}

TEST(TypeInfo, TypesAndPointerToTypesAreNotEqual) {
    EXPECT_NE(TypeInfo<int>::Id(), TypeInfo<int*>::Id());
}

TEST(TypeInfo, PointerAndConstPointerTypesAreNotEqual) {
    EXPECT_NE(TypeInfo<int>::Id(), TypeInfo<int const*>::Id());
}

TEST(TypeInfo, NonConstAndConstTypesAreEqual) {
    EXPECT_EQ(TypeInfo<int>::Id(), TypeInfo<int const>::Id());

    EXPECT_EQ(TypeInfo<int*>::Id(), TypeInfo<int* const>::Id());
}

TEST(TypeInfo, TypesAndReferencesToTypesAreEqual) {
    EXPECT_EQ(TypeInfo<int>::Id(), TypeInfo<int&>::Id());

    EXPECT_EQ(TypeInfo<int>::Id(), TypeInfo<int&&>::Id());
    EXPECT_EQ(TypeInfo<int>::Id(), TypeInfo<int const&>::Id());
    EXPECT_EQ(TypeInfo<int>::Id(), TypeInfo<int const&&>::Id());
}
