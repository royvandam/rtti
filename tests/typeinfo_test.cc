#include <gtest/gtest.h>

#include <rtti.hh>
#include <typeinfo>

using namespace RTTI;

TEST(TypeInfo, ImplementsNativeBehaviour) {
    // Same types are equal
    EXPECT_EQ(typeid(int), typeid(int));
    EXPECT_EQ(TypeInfo<int>::Id(), TypeInfo<int>::Id());

    // Different types are not equal
    EXPECT_NE(typeid(int), typeid(bool));
    EXPECT_NE(TypeInfo<int>::Id(), TypeInfo<bool>::Id());

    // Pointer types are not equal
    EXPECT_NE(typeid(int), typeid(int*));
    EXPECT_NE(TypeInfo<int>::Id(), TypeInfo<int*>::Id());

    // Pointer and const pointer type are not equal
    EXPECT_NE(typeid(int*), typeid(int const*));
    EXPECT_NE(TypeInfo<int>::Id(), TypeInfo<int const*>::Id());

    // Typeid strips const
    EXPECT_EQ(typeid(int), typeid(int const));
    EXPECT_EQ(TypeInfo<int>::Id(), TypeInfo<int const>::Id());

    EXPECT_EQ(typeid(int*), typeid(int* const));
    EXPECT_EQ(TypeInfo<int*>::Id(), TypeInfo<int* const>::Id());

    // Typeid strips references
    EXPECT_EQ(typeid(int), typeid(int&));
    EXPECT_EQ(TypeInfo<int>::Id(), TypeInfo<int&>::Id());

    EXPECT_EQ(typeid(int), typeid(int&&));
    EXPECT_EQ(TypeInfo<int>::Id(), TypeInfo<int&&>::Id());
}