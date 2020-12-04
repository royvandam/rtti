#include <gtest/gtest.h>

#include <rtti.hh>

namespace {
    struct ParentA : RTTI::Base<ParentA> {};
    struct ChildB : RTTI::Extends<ChildB, ParentA> {};
    struct ChildA : RTTI::Extends<ChildA, ParentA> {};

    struct ExtendedBase : RTTI::Base<ExtendedBase> {};
    struct ExtendedChildA : RTTI::Extends<ExtendedChildA, ChildA, ExtendedBase> {
        using ParentA::cast;
    };

    class RTTITest : public ::testing::Test {};

    TEST_F(RTTITest, TypeIdentification) {
        ParentA parent;
        EXPECT_EQ(parent.typeId(), RTTI::TypeInfo<ParentA>::Id());
        EXPECT_TRUE(parent.isA(RTTI::TypeInfo<ParentA>::Id()));
        EXPECT_FALSE(parent.isA(RTTI::TypeInfo<int>::Id()));

        ChildA child_a;
        EXPECT_EQ(child_a.typeId(), RTTI::TypeInfo<ChildA>::Id());
        EXPECT_TRUE(child_a.isA(RTTI::TypeInfo<ParentA>::Id()));
        EXPECT_TRUE(child_a.isA(RTTI::TypeInfo<ChildA>::Id()));
        EXPECT_FALSE(child_a.isA(RTTI::TypeInfo<ChildB>::Id()));

        ChildB child_b;
        EXPECT_EQ(child_b.typeId(), RTTI::TypeInfo<ChildB>::Id());
        EXPECT_TRUE(child_b.isA(RTTI::TypeInfo<ParentA>::Id()));
        EXPECT_TRUE(child_b.isA(RTTI::TypeInfo<ChildB>::Id()));
        EXPECT_FALSE(child_b.isA(RTTI::TypeInfo<ChildA>::Id()));

        ExtendedChildA extended_child_a;
        EXPECT_EQ(extended_child_a.typeId(), RTTI::TypeInfo<ExtendedChildA>::Id());
        EXPECT_TRUE(extended_child_a.isA(RTTI::TypeInfo<ParentA>::Id()));
        EXPECT_TRUE(extended_child_a.isA(RTTI::TypeInfo<ChildA>::Id()));
        EXPECT_TRUE(extended_child_a.isA(RTTI::TypeInfo<ExtendedBase>::Id()));
        EXPECT_TRUE(extended_child_a.isA(RTTI::TypeInfo<ExtendedChildA>::Id()));
        EXPECT_FALSE(extended_child_a.isA(RTTI::TypeInfo<ChildB>::Id()));
    }

    TEST_F(RTTITest, DynamicCasting) {
        ExtendedChildA child_a;
        EXPECT_EQ(child_a.cast<ParentA*>(), dynamic_cast<ParentA*>(&child_a));
        EXPECT_EQ(child_a.cast<ParentA*>(), static_cast<ParentA*>(&child_a));
        EXPECT_EQ(child_a.cast<ChildA*>(), &child_a);
        EXPECT_EQ(child_a.cast<ChildB*>(), nullptr);
        EXPECT_EQ(child_a.cast<ExtendedBase*>(), dynamic_cast<ExtendedBase*>(&child_a));

        ParentA* parent = child_a.cast<ParentA*>();
        EXPECT_TRUE(parent->isA(RTTI::TypeInfo<ChildA>::Id()));
        EXPECT_TRUE(parent->isA(RTTI::TypeInfo<ExtendedChildA>::Id()));
        EXPECT_EQ(parent->cast<ChildA*>(), dynamic_cast<ChildA*>(parent));
        EXPECT_EQ(parent->cast<ChildA*>(), &child_a);
        EXPECT_EQ(parent->cast<ParentA*>(), parent);
        EXPECT_EQ(parent->cast<ChildB*>(), nullptr);

        ExtendedBase* base = child_a.cast<ExtendedBase*>();
        EXPECT_TRUE(base->isA(RTTI::TypeInfo<ParentA>::Id()));
        EXPECT_TRUE(base->isA(RTTI::TypeInfo<ChildA>::Id()));
        EXPECT_TRUE(base->isA(RTTI::TypeInfo<ExtendedChildA>::Id()));
        EXPECT_EQ(base->cast<ChildA*>(), dynamic_cast<ChildA*>(parent));
        EXPECT_EQ(base->cast<ChildA*>(), &child_a);
        EXPECT_EQ(base->cast<ParentA*>(), parent);
        EXPECT_EQ(base->cast<ChildB*>(), nullptr);
    }
}  // namespace