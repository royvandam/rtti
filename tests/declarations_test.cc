#include <gtest/gtest.h>

#include <rtti.hh>
#include <type_traits>
#include <utility>

namespace {
    struct PolymorphicBase : virtual RTTI::Enable {
        RTTI_DECLARE_TYPEINFO(PolymorphicBase);
    };

    struct PolymorphicChild : PolymorphicBase {
        RTTI_DECLARE_TYPEINFO(PolymorphicChild, PolymorphicBase);
    };

    struct TrivialBaseA {
        RTTI_DECLARE_TRIVIAL(TrivialBaseA);
    public:
        int valueA;
    };

    struct TrivialBaseB {
        RTTI_DECLARE_TRIVIAL(TrivialBaseB);
    public:
        int valueB;
    };

    struct TrivialChild : TrivialBaseA, TrivialBaseB {
        RTTI_DECLARE_TRIVIAL(TrivialChild, TrivialBaseA, TrivialBaseB);
    };

    TEST(TypeDeclarations, PolymorphicTypesAreNotCopyableOrMovable) {
        static_assert(!std::is_copy_constructible_v<PolymorphicBase>);
        static_assert(!std::is_copy_assignable_v<PolymorphicBase>);
        static_assert(!std::is_move_constructible_v<PolymorphicBase>);
        static_assert(!std::is_move_assignable_v<PolymorphicBase>);
        static_assert(!std::is_copy_constructible_v<PolymorphicChild>);
        static_assert(!std::is_copy_assignable_v<PolymorphicChild>);
        static_assert(!std::is_move_constructible_v<PolymorphicChild>);
        static_assert(!std::is_move_assignable_v<PolymorphicChild>);
    }

    TEST(TypeDeclarations, TrivialHierarchyTypeIdentification) {
        static_assert(TrivialChild::TypeInfo::Is(TrivialChild::TypeInfo::Id()));
        static_assert(TrivialChild::TypeInfo::Is(TrivialBaseA::TypeInfo::Id()));
        static_assert(TrivialChild::TypeInfo::Is(TrivialBaseB::TypeInfo::Id()));
        static_assert(!TrivialBaseA::TypeInfo::Is(TrivialChild::TypeInfo::Id()));
        static_assert(!TrivialChild::TypeInfo::Is(PolymorphicChild::TypeInfo::Id()));
        static_assert(!TrivialChild::TypeInfo::Name().empty());
    }

    TEST(TypeDeclarations, IdsDoNotDependOnParentList) {
        static_assert(PolymorphicChild::TypeInfo::Id() == RTTI::TypeInfo<PolymorphicChild>::Id());
        static_assert(TrivialChild::TypeInfo::Id() == RTTI::TypeInfo<TrivialChild>::Id());
    }

    TEST(TypeDeclarations, DynamicCastingIsAvailableAtCompileTime) {
        constexpr TrivialChild child{};
        static_assert(TrivialChild::TypeInfo::DynamicCast(TrivialChild::TypeInfo::Id(), &child) == &child);
        static_assert(TrivialChild::TypeInfo::DynamicCast(TrivialBaseA::TypeInfo::Id(), &child)
            == static_cast<TrivialBaseA const*>(&child));
        static_assert(TrivialChild::TypeInfo::DynamicCast(TrivialBaseB::TypeInfo::Id(), &child)
            == static_cast<TrivialBaseB const*>(&child));
        static_assert(TrivialChild::TypeInfo::DynamicCast(RTTI::TypeInfo<int>::Id(), &child) == nullptr);
    }

    TEST(TypeDeclarations, TrivialHierarchyKeepsValueSemantics) {
        static_assert(std::is_trivial_v<TrivialBaseA>);
        static_assert(std::is_trivial_v<TrivialBaseB>);
        static_assert(std::is_trivial_v<TrivialChild>);
        static_assert(!std::is_polymorphic_v<TrivialChild>);
        static_assert(!std::is_base_of_v<RTTI::Enable, TrivialChild>);
        static_assert(std::is_copy_constructible_v<TrivialChild>);
        static_assert(std::is_copy_assignable_v<TrivialChild>);
        static_assert(std::is_move_constructible_v<TrivialChild>);
        static_assert(std::is_move_assignable_v<TrivialChild>);

        TrivialChild original{};
        original.valueA = 17;
        original.valueB = 42;
        TrivialChild copy = original;
        TrivialChild moved = std::move(copy);
        TrivialChild assigned{};
        assigned = original;
        TrivialChild moveAssigned{};
        moveAssigned = std::move(assigned);

        EXPECT_EQ(moved.valueA, original.valueA);
        EXPECT_EQ(moved.valueB, original.valueB);
        EXPECT_EQ(moveAssigned.valueA, original.valueA);
        EXPECT_EQ(moveAssigned.valueB, original.valueB);
    }
}  // namespace
