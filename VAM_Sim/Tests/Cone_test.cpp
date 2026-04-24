#include <gtest/gtest.h>
#include "Cone.h"

// Constructors
TEST(ConeTest, DefaultConstructor) {
    Cone c;
    EXPECT_EQ(c.getOrigin(), Vector3(0,0,0));
    EXPECT_NEAR(c.getDirection().length(), 1.0f, 1e-5f);
}

// Angle helpers
TEST(ConeTest, CosHalfAngle) {
    Cone c(Vector3(0,0,0), Vector3(0,0,1), 0.0f);
    EXPECT_FLOAT_EQ(c.cosHalfAngle(), 1.0f);
}

// Solid angle
TEST(ConeTest, SolidAngleZero) {
    Cone c(Vector3(0,0,0), Vector3(0,0,1), 0.0f);
    EXPECT_NEAR(c.solidAngle(), 0.0f, 1e-5f);
}

// Point inside cone
TEST(ConeTest, ContainsPointInside) {
    Cone c(Vector3(0,0,0), Vector3(0,0,1), 0.5f);
    Vector3 p(0,0,5);
    EXPECT_TRUE(c.containsPoint(p));
}

TEST(ConeTest, ContainsPointOutside) {
    Cone c(Vector3(0,0,0), Vector3(0,0,1), 0.3f);
    Vector3 p(5,0,0);
    EXPECT_FALSE(c.containsPoint(p));
}

// Edge case: point exactly on boundary
TEST(ConeTest, ContainsPointOnEdge) {
    float angle = 0.5f;
    Cone c(Vector3(0,0,0), Vector3(0,0,1), angle);

    float z = 10.0f;
    float x = std::tan(angle) * z;

    Vector3 p(x,0,z);
    EXPECT_TRUE(c.containsPoint(p));
}
