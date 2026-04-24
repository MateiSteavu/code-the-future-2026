#include <gtest/gtest.h>
#include "../inc/Geometry/Voxel.h"

// ─── Constructors ─────────────────────────────────────────────

TEST(VoxelTest, DefaultConstructor) {
    Voxel v;
    EXPECT_EQ(v.getCenter(), Vector3(0,0,0));
    EXPECT_FLOAT_EQ(v.getX(), 0.0f);
    EXPECT_FLOAT_EQ(v.getY(), 0.0f);
    EXPECT_FLOAT_EQ(v.getZ(), 0.0f);
    EXPECT_FLOAT_EQ(v.getEnergy(), 0.0f);
}

TEST(VoxelTest, ParamConstructorXYZ) {
    Voxel v(Vector3(1,2,3), 4,5,6);

    EXPECT_EQ(v.getCenter(), Vector3(1,2,3));
    EXPECT_FLOAT_EQ(v.getX(), 4);
    EXPECT_FLOAT_EQ(v.getY(), 5);
    EXPECT_FLOAT_EQ(v.getZ(), 6);
}

TEST(VoxelTest, CubeConstructor) {
    Voxel v(Vector3(1,1,1), 2.0f);

    EXPECT_FLOAT_EQ(v.getX(), 1.0f);
    EXPECT_FLOAT_EQ(v.getY(), 1.0f);
    EXPECT_FLOAT_EQ(v.getZ(), 1.0f);
}

// ─── Setters / Getters ───────────────────────────────────────

TEST(VoxelTest, SettersWork) {
    Voxel v;

    v.setCenter(Vector3(5,5,5));
    v.setX(2);
    v.setY(3);
    v.setZ(4);
    v.setEnergy(10);

    EXPECT_EQ(v.getCenter(), Vector3(5,5,5));
    EXPECT_FLOAT_EQ(v.getX(), 2);
    EXPECT_FLOAT_EQ(v.getY(), 3);
    EXPECT_FLOAT_EQ(v.getZ(), 4);
    EXPECT_FLOAT_EQ(v.getEnergy(), 10);
}

// ─── Operators ───────────────────────────────────────────────

TEST(VoxelTest, EqualityOperator) {
    Voxel a(Vector3(1,2,3), 1,1,1);
    Voxel b(Vector3(1,2,3), 1,1,1);

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(VoxelTest, ScalarMultiply) {
    Voxel v(Vector3(0,0,0), 1,2,3);
    Voxel r = v * 2.0f;

    EXPECT_FLOAT_EQ(r.getX(), 2);
    EXPECT_FLOAT_EQ(r.getY(), 4);
    EXPECT_FLOAT_EQ(r.getZ(), 6);
}

TEST(VoxelTest, ScalarDivide) {
    Voxel v(Vector3(0,0,0), 2,4,6);
    Voxel r = v / 2.0f;

    EXPECT_FLOAT_EQ(r.getX(), 1);
    EXPECT_FLOAT_EQ(r.getY(), 2);
    EXPECT_FLOAT_EQ(r.getZ(), 3);
}

TEST(VoxelTest, AdditionMerge) {
    Voxel a(Vector3(0,0,0), 1,2,3);
    Voxel b(Vector3(2,2,2), 4,1,2);

    Voxel c = a + b;

    EXPECT_EQ(c.getCenter(), Vector3(1,1,1)); // average
    EXPECT_FLOAT_EQ(c.getX(), 4); // max
    EXPECT_FLOAT_EQ(c.getY(), 2);
    EXPECT_FLOAT_EQ(c.getZ(), 3);
}

TEST(VoxelTest, SubtractionMinExtents) {
    Voxel a(Vector3(0,0,0), 3,4,5);
    Voxel b(Vector3(2,2,2), 1,2,3);

    Voxel c = a - b;

    EXPECT_EQ(c.getCenter(), Vector3(1,1,1));
    EXPECT_FLOAT_EQ(c.getX(), 1);
    EXPECT_FLOAT_EQ(c.getY(), 2);
    EXPECT_FLOAT_EQ(c.getZ(), 3);
}

// ─── Geometry ────────────────────────────────────────────────

TEST(VoxelTest, HalfExtents) {
    Voxel v(Vector3(0,0,0), 2,4,6);
    Vector3 h = v.half();

    EXPECT_EQ(h, Vector3(2,4,6));
}

TEST(VoxelTest, Volume) {
    Voxel v(Vector3(0,0,0), 1,2,3);

    EXPECT_FLOAT_EQ(v.volume(), 8 * 1 * 2 * 3);
}

TEST(VoxelTest, CircumsphereRadius) {
    Voxel v(Vector3(0,0,0), 1,1,1);

    float r = v.circumsphereRadius();
    EXPECT_NEAR(r, std::sqrt(3.0f), 1e-5f);
}

// ─── Corners ─────────────────────────────────────────────────

TEST(VoxelTest, Has8Corners) {
    Voxel v(Vector3(0,0,0), 1,1,1);

    auto corners = v.corners();
    EXPECT_EQ(corners.size(), 8);
}

TEST(VoxelTest, CornerPositions) {
    Voxel v(Vector3(0,0,0), 1,1,1);

    auto corners = v.corners();

    EXPECT_NE(std::find(corners.begin(), corners.end(), Vector3(1,1,1)), corners.end());
    EXPECT_NE(std::find(corners.begin(), corners.end(), Vector3(-1,-1,-1)), corners.end());
}

// ─── Containment ─────────────────────────────────────────────

TEST(VoxelTest, ContainsPointInside) {
    Voxel v(Vector3(0,0,0), 1,1,1);

    EXPECT_TRUE(v.contains(Vector3(0.5f, 0.0f, 0.0f)));
}

TEST(VoxelTest, ContainsPointOutside) {
    Voxel v(Vector3(0,0,0), 1,1,1);

    EXPECT_FALSE(v.contains(Vector3(2,0,0)));
}

// ─── Intersection ────────────────────────────────────────────

TEST(VoxelTest, IntersectsTrue) {
    Voxel a(Vector3(0,0,0), 1,1,1);
    Voxel b(Vector3(1,0,0), 1,1,1);

    EXPECT_TRUE(a.intersects(b));
}

TEST(VoxelTest, IntersectsFalse) {
    Voxel a(Vector3(0,0,0), 1,1,1);
    Voxel b(Vector3(5,0,0), 1,1,1);

    EXPECT_FALSE(a.intersects(b));
}

// ─── Math on center ──────────────────────────────────────────

TEST(VoxelTest, DistanceBetweenCenters) {
    Voxel a(Vector3(0,0,0), 1);
    Voxel b(Vector3(3,4,0), 1);

    EXPECT_FLOAT_EQ(a.distanceTo(b), 5.0f);
}

TEST(VoxelTest, NormalizeCenter) {
    Voxel v(Vector3(0,3,4), 1);
    Vector3 n = v.normalize();

    EXPECT_NEAR(n.length(), 1.0f, 1e-5f);
}
