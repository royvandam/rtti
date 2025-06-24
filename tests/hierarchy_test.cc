#include <gtest/gtest.h>

#include <rtti.hh>
#include <type_traits>

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

struct NonVirtualParentA : GrandParent {
    RTTI_DECLARE_TYPEINFO(NonVirtualParentA, GrandParent);
};

struct NonVirtualParentB : GrandParent {
    RTTI_DECLARE_TYPEINFO(NonVirtualParentB, GrandParent);
};

struct NonVirtualChild
    : NonVirtualParentA
    , NonVirtualParentB {
    RTTI_DECLARE_TYPEINFO(NonVirtualChild, NonVirtualParentA, NonVirtualParentB);
};

namespace {
    template <typename T>
    void ExpectMostDerivedObject(T* ptr, void const* expected) {
        using Void = std::conditional_t<std::is_const_v<T>, void const, void>;
        static_assert(std::is_same_v<decltype(ptr->template cast<void>()), Void*>);
        static_assert(std::is_same_v<decltype(ptr->template cast<const void>()), void const*>);

        EXPECT_EQ(ptr->template cast<void>(), expected);
        EXPECT_EQ(ptr->template cast<const void>(), expected);
        EXPECT_EQ(ptr->template cast<void>(), dynamic_cast<Void*>(ptr));
        EXPECT_EQ(ptr->template cast<const void>(), dynamic_cast<void const*>(ptr));
    }

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

    TEST(HierarchyTest, NonVirtualDiamondCasting) {
        NonVirtualChild child;
        NonVirtualParentA* parentA = &child;
        NonVirtualParentB* parentB = &child;
        GrandParent* grandParentA = parentA;
        GrandParent* grandParentB = parentB;

        EXPECT_TRUE(child.is<GrandParent>());
        EXPECT_EQ(child.cast<GrandParent>(), nullptr);
        EXPECT_EQ(child.cast<NonVirtualChild>(), &child);
        EXPECT_EQ(child.cast<NonVirtualParentA>(), parentA);
        EXPECT_EQ(child.cast<NonVirtualParentB>(), parentB);
        EXPECT_EQ(child.cast<ChildA>(), nullptr);

        EXPECT_EQ(grandParentA->cast<NonVirtualChild>(), &child);
        EXPECT_EQ(grandParentB->cast<NonVirtualChild>(), &child);
        EXPECT_EQ(parentA->cast<NonVirtualParentB>(), parentB);
        EXPECT_EQ(parentB->cast<NonVirtualParentA>(), parentA);
        EXPECT_EQ(grandParentA->cast<GrandParent>(), nullptr);
        EXPECT_EQ(grandParentB->cast<GrandParent>(), nullptr);
    }

    TEST(HierarchyTest, NonVirtualDiamondCastingConst) {
        NonVirtualChild const child;
        NonVirtualParentA const* parentA = &child;
        NonVirtualParentB const* parentB = &child;
        GrandParent const* grandParentA = parentA;
        GrandParent const* grandParentB = parentB;

        EXPECT_TRUE(child.is<GrandParent>());
        EXPECT_EQ(child.cast<GrandParent>(), nullptr);
        EXPECT_EQ(child.cast<NonVirtualChild>(), &child);
        EXPECT_EQ(child.cast<NonVirtualParentA>(), parentA);
        EXPECT_EQ(child.cast<NonVirtualParentB>(), parentB);
        EXPECT_EQ(child.cast<ChildA>(), nullptr);

        EXPECT_EQ(grandParentA->cast<NonVirtualChild>(), &child);
        EXPECT_EQ(grandParentB->cast<NonVirtualChild>(), &child);
        EXPECT_EQ(parentA->cast<NonVirtualParentB>(), parentB);
        EXPECT_EQ(parentB->cast<NonVirtualParentA>(), parentA);
        EXPECT_EQ(grandParentA->cast<GrandParent>(), nullptr);
        EXPECT_EQ(grandParentB->cast<GrandParent>(), nullptr);
    }

    TEST(HierarchyTest, MostDerivedObjectCasting) {
        ChildA child;
        ExpectMostDerivedObject(&child, &child);
        ExpectMostDerivedObject(static_cast<ParentA*>(&child), &child);
        ExpectMostDerivedObject(static_cast<ParentB*>(&child), &child);
        ExpectMostDerivedObject(static_cast<GrandParent*>(&child), &child);
        ExpectMostDerivedObject(static_cast<RTTI::Enable*>(&child), &child);
    }

    TEST(HierarchyTest, MostDerivedObjectCastingConst) {
        ChildA const child;
        ExpectMostDerivedObject(&child, &child);
        ExpectMostDerivedObject(static_cast<ParentA const*>(&child), &child);
        ExpectMostDerivedObject(static_cast<ParentB const*>(&child), &child);
        ExpectMostDerivedObject(static_cast<GrandParent const*>(&child), &child);
        ExpectMostDerivedObject(static_cast<RTTI::Enable const*>(&child), &child);
    }

    TEST(HierarchyTest, NonVirtualDiamondMostDerivedObjectCasting) {
        NonVirtualChild child;
        NonVirtualParentA* parentA = &child;
        NonVirtualParentB* parentB = &child;
        ExpectMostDerivedObject(&child, &child);
        ExpectMostDerivedObject(parentA, &child);
        ExpectMostDerivedObject(parentB, &child);
        ExpectMostDerivedObject(static_cast<GrandParent*>(parentA), &child);
        ExpectMostDerivedObject(static_cast<GrandParent*>(parentB), &child);
        ExpectMostDerivedObject(static_cast<RTTI::Enable*>(&child), &child);
    }

    TEST(HierarchyTest, NonVirtualDiamondMostDerivedObjectCastingConst) {
        NonVirtualChild const child;
        NonVirtualParentA const* parentA = &child;
        NonVirtualParentB const* parentB = &child;
        ExpectMostDerivedObject(&child, &child);
        ExpectMostDerivedObject(parentA, &child);
        ExpectMostDerivedObject(parentB, &child);
        ExpectMostDerivedObject(static_cast<GrandParent const*>(parentA), &child);
        ExpectMostDerivedObject(static_cast<GrandParent const*>(parentB), &child);
        ExpectMostDerivedObject(static_cast<RTTI::Enable const*>(&child), &child);
    }

}  // namespace
