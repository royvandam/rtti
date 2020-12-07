#include <gtest/gtest.h>

#include <rtti.hh>
#include <typeinfo>

using namespace RTTI;

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