#include <gtest/gtest.h>
#include "../inc/Geometry/Ray.h"

// Constructors
TEST(Vector3Test, DefaultConstructor) {
    Vector3 v;
    EXPECT_FLOAT_EQ(v.getX(), 0.0f);
    EXPECT_FLOAT_EQ(v.getY(), 0.0f);
    EXPECT_FLOAT_EQ(v.getZ(), 0.0f);
}

TEST(Vector3Test, ParamConstructor) {
    Vector3 v(1,2,3);
    EXPECT_FLOAT_EQ(v.getX(), 1);
    EXPECT_FLOAT_EQ(v.getY(), 2);
    EXPECT_FLOAT_EQ(v.getZ(), 3);
}

// Operators
TEST(Vector3Test, Addition) {
    Vector3 a(1,2,3), b(4,5,6);
    Vector3 c = a + b;
    EXPECT_EQ(c, Vector3(5,7,9));
}

TEST(Vector3Test, Subtraction) {
    Vector3 a(5,7,9), b(1,2,3);
    EXPECT_EQ(a - b, Vector3(4,5,6));
}

TEST(Vector3Test, ScalarMultiply) {
    Vector3 v(1,2,3);
    EXPECT_EQ(v * 2.0f, Vector3(2,4,6));
}

TEST(Vector3Test, DotProduct) {
    Vector3 a(1,0,0), b(0,1,0);
    EXPECT_FLOAT_EQ(a.dot(b), 0.0f);
}

TEST(Vector3Test, CrossProduct) {
    Vector3 a(1,0,0), b(0,1,0);
    EXPECT_EQ(a.cross(b), Vector3(0,0,1));
}

TEST(Vector3Test, Length) {
    Vector3 v(3,4,0);
    EXPECT_FLOAT_EQ(v.length(), 5.0f);
}

TEST(Vector3Test, Normalize) {
    Vector3 v(0,3,4);
    Vector3 n = v.normalize();
    EXPECT_NEAR(n.length(), 1.0f, 1e-5f);
}
