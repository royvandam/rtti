#include <gtest/gtest.h>

#include <rtti.hh>

namespace {
    template <typename Parent>
    struct CountingTypeInfo : RTTI::TypeInfo<Parent> {
        inline static int castCount = 0;
        inline static int isCount = 0;

        static bool Is(RTTI::TypeId id) noexcept {
            ++isCount;
            return RTTI::TypeInfo<Parent>::Is(id);
        }

        template <typename T>
        static void const* DynamicCast(RTTI::TypeId id, T const* ptr) noexcept {
            ++castCount;
            return RTTI::TypeInfo<Parent>::DynamicCast(id, ptr);
        }
    };

    struct FirstParent {
        using TypeInfo = CountingTypeInfo<FirstParent>;
    };

    struct SecondParent {
        using TypeInfo = CountingTypeInfo<SecondParent>;
    };

    struct Child : FirstParent, SecondParent, virtual RTTI::Enable {
        RTTI_DECLARE_TYPEINFO(Child, FirstParent, SecondParent);
    };

    class TraversalTest : public ::testing::Test {
    protected:
        void SetUp() override {
            FirstParent::TypeInfo::castCount = 0;
            FirstParent::TypeInfo::isCount = 0;
            SecondParent::TypeInfo::castCount = 0;
            SecondParent::TypeInfo::isCount = 0;
        }
    };

    TEST_F(TraversalTest, StopsAfterFirstMatchingParent) {
        Child child;
        EXPECT_EQ(child.cast<FirstParent>(), static_cast<FirstParent*>(&child));
        EXPECT_EQ(FirstParent::TypeInfo::castCount, 1);
        EXPECT_EQ(SecondParent::TypeInfo::castCount, 0);
        EXPECT_EQ(FirstParent::TypeInfo::isCount, 0);
        EXPECT_EQ(SecondParent::TypeInfo::isCount, 0);
    }

    TEST_F(TraversalTest, ContinuesAfterUnmatchedParent) {
        Child child;
        EXPECT_EQ(child.cast<SecondParent>(), static_cast<SecondParent*>(&child));
        EXPECT_EQ(FirstParent::TypeInfo::castCount, 1);
        EXPECT_EQ(SecondParent::TypeInfo::castCount, 1);
        EXPECT_EQ(FirstParent::TypeInfo::isCount, 0);
        EXPECT_EQ(SecondParent::TypeInfo::isCount, 0);
    }

    TEST_F(TraversalTest, MissingTypeVisitsEachParentOnce) {
        Child child;
        EXPECT_EQ(child.cast<int>(), nullptr);
        EXPECT_EQ(FirstParent::TypeInfo::castCount, 1);
        EXPECT_EQ(SecondParent::TypeInfo::castCount, 1);
        EXPECT_EQ(FirstParent::TypeInfo::isCount, 0);
        EXPECT_EQ(SecondParent::TypeInfo::isCount, 0);
    }
}  // namespace
