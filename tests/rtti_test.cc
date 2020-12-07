#include <gtest/gtest.h>

#include <rtti.hh>

// TODO(roda): Restructure, cleanup and improve unit-tests

namespace {
    struct ParentA : RTTI::Base<ParentA> {};
    struct ChildB : RTTI::Extends<ChildB, ParentA> {};
    struct ChildA : RTTI::Extends<ChildA, ParentA> {};

    struct ExtendedBase : RTTI::Base<ExtendedBase> {};
    struct ExtendedChildA : RTTI::Extends<ExtendedChildA, ChildA, ExtendedBase> {};

    class RTTITest : public ::testing::Test {};

    TEST_F(RTTITest, TypeIdentification) {
        ParentA parent;
        EXPECT_EQ(parent.typeId(), RTTI::TypeInfo<ParentA>::Id());
        EXPECT_TRUE(parent.isById(RTTI::TypeInfo<ParentA>::Id()));
        EXPECT_TRUE(parent.is<ParentA>());
        EXPECT_FALSE(parent.isById(RTTI::TypeInfo<int>::Id()));
        EXPECT_FALSE(parent.is<int>());

        ChildA child_a;
        EXPECT_EQ(child_a.typeId(), RTTI::TypeInfo<ChildA>::Id());
        EXPECT_TRUE(child_a.isById(RTTI::TypeInfo<ParentA>::Id()));
        EXPECT_TRUE(child_a.isById(RTTI::TypeInfo<ChildA>::Id()));
        EXPECT_FALSE(child_a.isById(RTTI::TypeInfo<ChildB>::Id()));

        ChildB child_b;
        EXPECT_EQ(child_b.typeId(), RTTI::TypeInfo<ChildB>::Id());
        EXPECT_TRUE(child_b.isById(RTTI::TypeInfo<ParentA>::Id()));
        EXPECT_TRUE(child_b.isById(RTTI::TypeInfo<ChildB>::Id()));
        EXPECT_FALSE(child_b.isById(RTTI::TypeInfo<ChildA>::Id()));

        ExtendedChildA extended_child_a;
        EXPECT_EQ(extended_child_a.typeId(), RTTI::TypeInfo<ExtendedChildA>::Id());
        EXPECT_TRUE(extended_child_a.isById(RTTI::TypeInfo<ParentA>::Id()));
        EXPECT_TRUE(extended_child_a.isById(RTTI::TypeInfo<ChildA>::Id()));
        EXPECT_TRUE(extended_child_a.isById(RTTI::TypeInfo<ExtendedBase>::Id()));
        EXPECT_TRUE(extended_child_a.isById(RTTI::TypeInfo<ExtendedChildA>::Id()));
        EXPECT_FALSE(extended_child_a.isById(RTTI::TypeInfo<ChildB>::Id()));

        RTTI::Type* type = &extended_child_a;
        EXPECT_EQ(type->typeId(), RTTI::TypeInfo<ExtendedChildA>::Id());
        EXPECT_TRUE(type->isById(RTTI::TypeInfo<ParentA>::Id()));
        EXPECT_TRUE(type->isById(RTTI::TypeInfo<ChildA>::Id()));
        EXPECT_TRUE(type->isById(RTTI::TypeInfo<ExtendedBase>::Id()));
        EXPECT_TRUE(type->isById(RTTI::TypeInfo<ExtendedChildA>::Id()));
        EXPECT_FALSE(type->isById(RTTI::TypeInfo<ChildB>::Id()));
    }

    TEST_F(RTTITest, DynamicCasting) {
        ExtendedChildA child_a;
        EXPECT_EQ(child_a.cast<ParentA>(), dynamic_cast<ParentA*>(&child_a));
        EXPECT_EQ(child_a.cast<ParentA>(), static_cast<ParentA*>(&child_a));
        EXPECT_EQ(child_a.cast<ChildA>(), &child_a);
        EXPECT_EQ(child_a.cast<ChildB>(), nullptr);
        EXPECT_EQ(child_a.cast<ExtendedBase>(), dynamic_cast<ExtendedBase*>(&child_a));

        ParentA* parent = &child_a;
        EXPECT_TRUE(parent->isById(RTTI::TypeInfo<ChildA>::Id()));
        EXPECT_TRUE(parent->isById(RTTI::TypeInfo<ExtendedChildA>::Id()));
        EXPECT_EQ(parent->cast<ChildA>(), dynamic_cast<ChildA*>(parent));
        EXPECT_EQ(parent->cast<ChildA>(), &child_a);
        EXPECT_EQ(parent->cast<ParentA>(), parent);
        EXPECT_EQ(parent->cast<ChildB>(), nullptr);

        ExtendedBase* base = &child_a;
        ExtendedBase const* const_base = child_a.cast<ExtendedBase>();
        EXPECT_TRUE(base->isById(RTTI::TypeInfo<ParentA>::Id()));
        EXPECT_TRUE(base->isById(RTTI::TypeInfo<ChildA>::Id()));
        EXPECT_TRUE(base->isById(RTTI::TypeInfo<ExtendedChildA>::Id()));
        EXPECT_EQ(base->cast<ChildA>(), dynamic_cast<ChildA*>(parent));
        EXPECT_EQ(base->cast<ChildA>(), &child_a);
        EXPECT_EQ(base->cast<ParentA>(), parent);
        EXPECT_EQ(base->cast<ChildB>(), nullptr);

        RTTI::Type* type = &child_a;
        EXPECT_TRUE(type->isById(RTTI::TypeInfo<ParentA>::Id()));
        EXPECT_TRUE(type->isById(RTTI::TypeInfo<ChildA>::Id()));
        EXPECT_TRUE(type->isById(RTTI::TypeInfo<ExtendedChildA>::Id()));
        EXPECT_EQ(type->cast<ChildA>(), dynamic_cast<ChildA*>(parent));
        EXPECT_EQ(type->cast<ChildA>(), &child_a);
        EXPECT_EQ(type->cast<ParentA>(), parent);
        EXPECT_EQ(type->cast<ChildB>(), nullptr);
    }

}  // namespace