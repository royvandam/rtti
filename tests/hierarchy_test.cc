#include <gtest/gtest.h>

#include <rtti.hh>

struct GrandParent : virtual RTTI::Enable {
    RTTI_DECLARE_TYPEINFO(GrandParent);
};

struct ParentA : virtual GrandParent {
    RTTI_DECLARE_TYPEINFO(ParentA, GrandParent);
};

struct ParentB : virtual GrandParent {
    RTTI_DECLARE_TYPEINFO(ParentB, GrandParent);
};

struct ChildA
    : ParentA
    , ParentB {
    RTTI_DECLARE_TYPEINFO(ChildA, ParentA, ParentB);
};

struct ChildB
    : ParentA
    , ParentB {
    RTTI_DECLARE_TYPEINFO(ChildB, ParentA, ParentB);
};

template<typename T>
struct ChildT
    : ParentA
    , ParentB {
    RTTI_DECLARE_TYPEINFO(ChildT<T>, ParentA, ParentB);
};

namespace {
    TEST(HierarchyTest, TypeIdentification) {
        ChildA childA;
        EXPECT_EQ(childA.typeId(), RTTI::TypeInfo<ChildA>::Id());
        EXPECT_TRUE(childA.is<ChildA>());
        EXPECT_FALSE(childA.is<ChildB>());
        EXPECT_TRUE(childA.is<ParentA>());
        EXPECT_TRUE(childA.is<ParentB>());
        EXPECT_TRUE(childA.is<GrandParent>());

        ChildB childB;
        EXPECT_EQ(childB.typeId(), RTTI::TypeInfo<ChildB>::Id());
        EXPECT_FALSE(childB.is<ChildA>());
        EXPECT_TRUE(childB.is<ChildB>());
        EXPECT_TRUE(childB.is<ParentA>());
        EXPECT_TRUE(childB.is<ParentB>());
        EXPECT_TRUE(childB.is<GrandParent>());

        ChildT<int> childTi;
        EXPECT_EQ(childTi.typeId(), RTTI::TypeInfo<ChildT<int>>::Id());
        EXPECT_FALSE(childTi.is<ChildT<float>>());
        EXPECT_TRUE(childTi.is<ChildT<int>>());
        EXPECT_TRUE(childTi.is<ParentA>());
        EXPECT_TRUE(childTi.is<ParentB>());
        EXPECT_TRUE(childTi.is<GrandParent>());
        
        ChildT<float> childTf;
        EXPECT_EQ(childTf.typeId(), RTTI::TypeInfo<ChildT<float>>::Id());
        EXPECT_FALSE(childTf.is<ChildT<int>>());
        EXPECT_TRUE(childTf.is<ChildT<float>>());
        EXPECT_TRUE(childTf.is<ParentA>());
        EXPECT_TRUE(childTf.is<ParentB>());
        EXPECT_TRUE(childTf.is<GrandParent>());

        EXPECT_NE(childTi.typeId(), childTf.typeId());

        ParentA parentA;
        EXPECT_EQ(parentA.typeId(), RTTI::TypeInfo<ParentA>::Id());
        EXPECT_FALSE(parentA.is<ChildA>());
        EXPECT_FALSE(parentA.is<ChildB>());
        EXPECT_TRUE(parentA.is<ParentA>());
        EXPECT_FALSE(parentA.is<ParentB>());
        EXPECT_TRUE(parentA.is<GrandParent>());

        ParentB parentB;
        EXPECT_EQ(parentB.typeId(), RTTI::TypeInfo<ParentB>::Id());
        EXPECT_FALSE(parentB.is<ChildA>());
        EXPECT_FALSE(parentB.is<ChildB>());
        EXPECT_FALSE(parentB.is<ParentA>());
        EXPECT_TRUE(parentB.is<ParentB>());
        EXPECT_TRUE(parentB.is<GrandParent>());

        GrandParent grandParent;
        EXPECT_EQ(grandParent.typeId(), RTTI::TypeInfo<GrandParent>::Id());
        EXPECT_FALSE(grandParent.is<ChildA>());
        EXPECT_FALSE(grandParent.is<ChildB>());
        EXPECT_FALSE(grandParent.is<ParentA>());
        EXPECT_FALSE(grandParent.is<ParentB>());
        EXPECT_TRUE(grandParent.is<GrandParent>());
    }

    TEST(HierarchyTest, UpCasting) {
        ChildA childA;
        EXPECT_EQ(childA.cast<ChildA>(), &childA);
        EXPECT_EQ(childA.cast<ChildB>(), nullptr);
        EXPECT_EQ(childA.cast<ParentA>(), dynamic_cast<ParentA*>(&childA));
        EXPECT_EQ(childA.cast<ParentA>(), static_cast<ParentA*>(&childA));
        EXPECT_EQ(childA.cast<ParentB>(), dynamic_cast<ParentB*>(&childA));
        EXPECT_EQ(childA.cast<ParentB>(), static_cast<ParentB*>(&childA));
        EXPECT_EQ(childA.cast<GrandParent>(), dynamic_cast<GrandParent*>(&childA));
        EXPECT_EQ(childA.cast<GrandParent>(), static_cast<GrandParent*>(&childA));
    }

    TEST(HierarchyTest, UpCastingConst) {
        ChildA const childA;
        EXPECT_EQ(childA.cast<ChildA>(), &childA);
        EXPECT_EQ(childA.cast<ChildB>(), nullptr);
        EXPECT_EQ(childA.cast<ParentA>(), dynamic_cast<ParentA const*>(&childA));
        EXPECT_EQ(childA.cast<ParentA>(), static_cast<ParentA const*>(&childA));
        EXPECT_EQ(childA.cast<ParentB>(), dynamic_cast<ParentB const*>(&childA));
        EXPECT_EQ(childA.cast<ParentB>(), static_cast<ParentB const*>(&childA));
        EXPECT_EQ(childA.cast<GrandParent>(), dynamic_cast<GrandParent const*>(&childA));
        EXPECT_EQ(childA.cast<GrandParent>(), static_cast<GrandParent const*>(&childA));
    }

    TEST(HierarchyTest, DownCasting) {
        ChildA childA;
        GrandParent* grandParent = childA.cast<GrandParent>();

        EXPECT_EQ(grandParent->cast<ChildA>(), &childA);
        EXPECT_EQ(grandParent->cast<ChildB>(), nullptr);
        EXPECT_EQ(grandParent->cast<ParentA>(), dynamic_cast<ParentA*>(&childA));
        EXPECT_EQ(grandParent->cast<ParentA>(), static_cast<ParentA*>(&childA));
        EXPECT_EQ(grandParent->cast<ParentB>(), dynamic_cast<ParentB*>(&childA));
        EXPECT_EQ(grandParent->cast<ParentB>(), static_cast<ParentB*>(&childA));
        EXPECT_EQ(grandParent->cast<GrandParent>(), dynamic_cast<GrandParent*>(&childA));
        EXPECT_EQ(grandParent->cast<GrandParent>(), static_cast<GrandParent*>(&childA));
    }

    TEST(HierarchyTest, DownCastingConst) {
        ChildA const childA;
        GrandParent const* grandParent = childA.cast<GrandParent>();

        EXPECT_EQ(grandParent->cast<ChildA>(), &childA);
        EXPECT_EQ(grandParent->cast<ChildB>(), nullptr);
        EXPECT_EQ(grandParent->cast<ParentA>(), dynamic_cast<ParentA const*>(&childA));
        EXPECT_EQ(grandParent->cast<ParentA>(), static_cast<ParentA const*>(&childA));
        EXPECT_EQ(grandParent->cast<ParentB>(), dynamic_cast<ParentB const*>(&childA));
        EXPECT_EQ(grandParent->cast<ParentB>(), static_cast<ParentB const*>(&childA));
        EXPECT_EQ(grandParent->cast<GrandParent>(), dynamic_cast<GrandParent const*>(&childA));
        EXPECT_EQ(grandParent->cast<GrandParent>(), static_cast<GrandParent const*>(&childA));
    }

}  // namespace
