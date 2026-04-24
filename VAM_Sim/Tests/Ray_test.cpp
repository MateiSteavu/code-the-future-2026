#include <gtest/gtest.h>
#include "Ray.h"

// Constructors
TEST(RayTest, DefaultConstructor) {
    Ray r;
    EXPECT_EQ(r.getOrigin(), Vector3(0,0,0));
    EXPECT_EQ(r.getDirection(), Vector3(0,0,1));
}

TEST(RayTest, ParamConstructorNormalizes) {
    Ray r(Vector3(0,0,0), Vector3(0,0,10));
    EXPECT_NEAR(r.getDirection().length(), 1.0f, 1e-5f);
}

// at(t)
TEST(RayTest, PointAlongRay) {
    Ray r(Vector3(1,2,3), Vector3(0,0,1));
    Vector3 p = r.at(5.0f);
    EXPECT_EQ(p, Vector3(1,2,8));
}

// Setters
TEST(RayTest, SetDirectionNormalizes) {
    Ray r;
    r.setDirection(Vector3(0,0,5));
    EXPECT_NEAR(r.getDirection().length(), 1.0f, 1e-5f);
}
