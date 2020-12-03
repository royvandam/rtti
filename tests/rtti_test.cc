#include <rtti.hh>

#include <gtest/gtest.h>

namespace {

    struct Shape : RTTI::Base<Shape> {};
    struct Square : RTTI::Extends<Square, Shape> {};
    struct Circle : RTTI::Extends<Circle, Shape> {};

    struct SpecialBase : RTTI::Base<SpecialBase> {};
    struct SpecialCircle : RTTI::Extends<SpecialCircle, Circle, SpecialBase> {
        using Shape::cast;
    };

    class RTTITest : public ::testing::Test
    {};

    TEST_F(RTTITest, TypeIdentification) {
        Shape shape;
        EXPECT_EQ(shape.typeId(), RTTI::TypeInfo<Shape>::Id());
        EXPECT_TRUE(shape.isA(RTTI::TypeInfo<Shape>::Id()));
        EXPECT_FALSE(shape.isA(RTTI::TypeInfo<int>::Id()));

        Square square;
        EXPECT_EQ(square.typeId(), RTTI::TypeInfo<Square>::Id());
        EXPECT_TRUE(square.isA(RTTI::TypeInfo<Shape>::Id()));
        EXPECT_TRUE(square.isA(RTTI::TypeInfo<Square>::Id()));
        EXPECT_FALSE(square.isA(RTTI::TypeInfo<Circle>::Id()));

        Circle circle;
        EXPECT_EQ(circle.typeId(), RTTI::TypeInfo<Circle>::Id());
        EXPECT_TRUE(circle.isA(RTTI::TypeInfo<Shape>::Id()));
        EXPECT_TRUE(circle.isA(RTTI::TypeInfo<Circle>::Id()));
        EXPECT_FALSE(circle.isA(RTTI::TypeInfo<Square>::Id()));

        SpecialCircle specialCircle;
        EXPECT_EQ(specialCircle.typeId(), RTTI::TypeInfo<SpecialCircle>::Id());
        EXPECT_TRUE(specialCircle.isA(RTTI::TypeInfo<Shape>::Id()));
        EXPECT_TRUE(specialCircle.isA(RTTI::TypeInfo<Circle>::Id()));
        EXPECT_TRUE(specialCircle.isA(RTTI::TypeInfo<SpecialBase>::Id()));
        EXPECT_TRUE(specialCircle.isA(RTTI::TypeInfo<SpecialCircle>::Id()));
        EXPECT_FALSE(specialCircle.isA(RTTI::TypeInfo<Square>::Id()));
    }

    TEST_F(RTTITest, Casting) {
        SpecialCircle circle;
        EXPECT_EQ(circle.cast<Shape*>(), dynamic_cast<Shape*>(&circle));
        EXPECT_EQ(circle.cast<Shape*>(), static_cast<Shape*>(&circle));
        EXPECT_EQ(circle.cast<Circle*>(), &circle);
        EXPECT_EQ(circle.cast<Square*>(), nullptr);
        EXPECT_EQ(circle.cast<SpecialBase*>(), dynamic_cast<SpecialBase*>(&circle));

        Shape* shape = circle.cast<Shape*>();
        EXPECT_TRUE(shape->isA(RTTI::TypeInfo<Circle>::Id()));
        EXPECT_TRUE(shape->isA(RTTI::TypeInfo<SpecialCircle>::Id()));
        EXPECT_EQ(shape->cast<Circle*>(), dynamic_cast<Circle*>(shape));
        EXPECT_EQ(shape->cast<Circle*>(), &circle);
        EXPECT_EQ(shape->cast<Shape*>(), shape);
        EXPECT_EQ(shape->cast<Square*>(), nullptr);

        SpecialBase* base = circle.cast<SpecialBase*>();
        EXPECT_TRUE(base->isA(RTTI::TypeInfo<Shape>::Id()));
        EXPECT_TRUE(base->isA(RTTI::TypeInfo<Circle>::Id()));
        EXPECT_TRUE(base->isA(RTTI::TypeInfo<SpecialCircle>::Id()));
        EXPECT_EQ(base->cast<Circle*>(), dynamic_cast<Circle*>(shape));
        EXPECT_EQ(base->cast<Circle*>(), &circle);
        EXPECT_EQ(base->cast<Shape*>(), shape);
        EXPECT_EQ(base->cast<Square*>(), nullptr);
    }
}