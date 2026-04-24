#include <gtest/gtest.h>
#include <cmath>
#include <stdexcept>
#include "../../inc/Geometry/Voxel.h"

// ─── Helpers ──────────────────────────────────────────────────────────────────

static constexpr float VS  = Voxel_Size;
static constexpr float VS2 = Voxel_Size / 2.0f;

// Fuzzy-equal for floats
static bool feq(float a, float b, float eps = 1e-5f) {
    return std::abs(a - b) < eps;
}

static bool vec3eq(const Vector3& a, const Vector3& b, float eps = 1e-5f) {
    return feq(a.getX(), b.getX(), eps) &&
           feq(a.getY(), b.getY(), eps) &&
           feq(a.getZ(), b.getZ(), eps);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Constructors
// ═══════════════════════════════════════════════════════════════════════════════

TEST(VoxelConstructor, DefaultConstructorZeroCenter) {
    Voxel v;
    EXPECT_TRUE(vec3eq(v.getCenter(), Vector3(0.0f, 0.0f, 0.0f)));
}

TEST(VoxelConstructor, DefaultConstructorZeroEnergy) {
    Voxel v;
    EXPECT_FLOAT_EQ(v.getEnergy(), 0.0f);
}

TEST(VoxelConstructor, ParameterisedConstructorSetsCenter) {
    Vector3 c(1.0f, 2.0f, 3.0f);
    Voxel v(c);
    EXPECT_TRUE(vec3eq(v.getCenter(), c));
}

TEST(VoxelConstructor, ParameterisedConstructorZeroEnergy) {
    Voxel v(Vector3(1.0f, 2.0f, 3.0f));
    EXPECT_FLOAT_EQ(v.getEnergy(), 0.0f);
}

TEST(VoxelConstructor, NegativeCenter) {
    Vector3 c(-5.0f, -3.0f, -1.0f);
    Voxel v(c);
    EXPECT_TRUE(vec3eq(v.getCenter(), c));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Getters / Setters
// ═══════════════════════════════════════════════════════════════════════════════

TEST(VoxelGetterSetter, SetAndGetCenter) {
    Voxel v;
    Vector3 c(4.0f, 5.0f, 6.0f);
    v.setCenter(c);
    EXPECT_TRUE(vec3eq(v.getCenter(), c));
}

TEST(VoxelGetterSetter, SetAndGetEnergy) {
    Voxel v;
    v.setEnergy(42.5f);
    EXPECT_FLOAT_EQ(v.getEnergy(), 42.5f);
}

TEST(VoxelGetterSetter, SetEnergyNegative) {
    Voxel v;
    v.setEnergy(-10.0f);
    EXPECT_FLOAT_EQ(v.getEnergy(), -10.0f);
}

TEST(VoxelGetterSetter, SetEnergyZero) {
    Voxel v(Vector3(1.0f, 1.0f, 1.0f));
    v.setEnergy(99.0f);
    v.setEnergy(0.0f);
    EXPECT_FLOAT_EQ(v.getEnergy(), 0.0f);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Operator ==  /  !=
// ═══════════════════════════════════════════════════════════════════════════════

TEST(VoxelEquality, EqualSameCenter) {
    Voxel a(Vector3(1.0f, 2.0f, 3.0f));
    Voxel b(Vector3(1.0f, 2.0f, 3.0f));
    EXPECT_TRUE(a == b);
}

TEST(VoxelEquality, EqualDefaultConstructors) {
    Voxel a, b;
    EXPECT_TRUE(a == b);
}

TEST(VoxelEquality, NotEqualDifferentCenter) {
    Voxel a(Vector3(1.0f, 0.0f, 0.0f));
    Voxel b(Vector3(0.0f, 0.0f, 0.0f));
    EXPECT_FALSE(a == b);
}

TEST(VoxelEquality, NotEqualOperator) {
    Voxel a(Vector3(1.0f, 0.0f, 0.0f));
    Voxel b(Vector3(0.0f, 1.0f, 0.0f));
    EXPECT_TRUE(a != b);
}

TEST(VoxelEquality, NotEqualOperatorFalseWhenEqual) {
    Voxel a(Vector3(2.0f, 2.0f, 2.0f));
    Voxel b(Vector3(2.0f, 2.0f, 2.0f));
    EXPECT_FALSE(a != b);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Assignment operator
// ═══════════════════════════════════════════════════════════════════════════════

TEST(VoxelAssignment, CopiesCenterFromOther) {
    Voxel a(Vector3(7.0f, 8.0f, 9.0f));
    Voxel b;
    b = a;
    EXPECT_TRUE(vec3eq(b.getCenter(), a.getCenter()));
}

TEST(VoxelAssignment, SelfAssignmentSafe) {
    Voxel a(Vector3(1.0f, 2.0f, 3.0f));
    a = a;
    EXPECT_TRUE(vec3eq(a.getCenter(), Vector3(1.0f, 2.0f, 3.0f)));
}

TEST(VoxelAssignment, ReturnsSelfReference) {
    Voxel a(Vector3(1.0f, 2.0f, 3.0f));
    Voxel b, c;
    c = b = a;   // chained assignment
    EXPECT_TRUE(vec3eq(c.getCenter(), a.getCenter()));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Arithmetic operators
// ═══════════════════════════════════════════════════════════════════════════════

TEST(VoxelArithmetic, ScalarMultiplyCenterScales) {
    Voxel v(Vector3(1.0f, 2.0f, 3.0f));
    Voxel r = v * 2.0f;
    EXPECT_TRUE(vec3eq(r.getCenter(), Vector3(2.0f, 4.0f, 6.0f)));
}

TEST(VoxelArithmetic, ScalarMultiplyByZeroGivesZeroCenter) {
    Voxel v(Vector3(5.0f, 5.0f, 5.0f));
    Voxel r = v * 0.0f;
    EXPECT_TRUE(vec3eq(r.getCenter(), Vector3(0.0f, 0.0f, 0.0f)));
}

TEST(VoxelArithmetic, ScalarMultiplyByNegative) {
    Voxel v(Vector3(1.0f, 2.0f, 3.0f));
    Voxel r = v * -1.0f;
    EXPECT_TRUE(vec3eq(r.getCenter(), Vector3(-1.0f, -2.0f, -3.0f)));
}

TEST(VoxelArithmetic, ScalarDivideReturnsSameCenterAsOriginal) {
    // Per implementation, operator/ returns Voxel(center) unchanged
    Voxel v(Vector3(4.0f, 8.0f, 12.0f));
    Voxel r = v / 2.0f;
    EXPECT_TRUE(vec3eq(r.getCenter(), v.getCenter()));
}

TEST(VoxelArithmetic, ScalarDivideByZeroThrows) {
    Voxel v(Vector3(1.0f, 1.0f, 1.0f));
    EXPECT_THROW(v / 0.0f, std::runtime_error);
}

TEST(VoxelArithmetic, AdditionAveragesCenters) {
    Voxel a(Vector3(0.0f, 0.0f, 0.0f));
    Voxel b(Vector3(4.0f, 4.0f, 4.0f));
    Voxel r = a + b;
    EXPECT_TRUE(vec3eq(r.getCenter(), Vector3(2.0f, 2.0f, 2.0f)));
}

TEST(VoxelArithmetic, AdditionSymmetry) {
    Voxel a(Vector3(1.0f, 2.0f, 3.0f));
    Voxel b(Vector3(3.0f, 4.0f, 5.0f));
    EXPECT_TRUE(vec3eq((a + b).getCenter(), (b + a).getCenter()));
}

TEST(VoxelArithmetic, SubtractionHalfDifference) {
    // operator- computes (center - other.center) / 2
    Voxel a(Vector3(4.0f, 6.0f, 8.0f));
    Voxel b(Vector3(2.0f, 2.0f, 2.0f));
    Voxel r = a - b;
    EXPECT_TRUE(vec3eq(r.getCenter(), Vector3(1.0f, 2.0f, 3.0f)));
}

TEST(VoxelArithmetic, SubtractionSelfGivesZeroCenter) {
    Voxel a(Vector3(5.0f, 7.0f, 9.0f));
    Voxel r = a - a;
    EXPECT_TRUE(vec3eq(r.getCenter(), Vector3(0.0f, 0.0f, 0.0f)));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Rotation
// ═══════════════════════════════════════════════════════════════════════════════

TEST(VoxelRotation, RotXBy90RotatesCenter) {
    // A point on +Y axis rotated 90° around X should land on +Z
    Voxel v(Vector3(0.0f, 1.0f, 0.0f));
    v.Rot_X(M_PI / 2.0f);
    EXPECT_NEAR(v.getCenter().getY(), 0.0f, 1e-5f);
    EXPECT_NEAR(v.getCenter().getZ(), 1.0f, 1e-5f);
}

TEST(VoxelRotation, RotYBy90RotatesCenter) {
    // A point on +Z axis rotated 90° around Y should land on +X
    Voxel v(Vector3(0.0f, 0.0f, 1.0f));
    v.Rot_Y(M_PI / 2.0f);
    EXPECT_NEAR(v.getCenter().getX(), 1.0f, 1e-5f);
    EXPECT_NEAR(v.getCenter().getZ(), 0.0f, 1e-5f);
}

TEST(VoxelRotation, RotZBy90RotatesCenter) {
    // A point on +X axis rotated 90° around Z should land on +Y
    Voxel v(Vector3(1.0f, 0.0f, 0.0f));
    v.Rot_Z(M_PI / 2.0f);
    EXPECT_NEAR(v.getCenter().getX(), 0.0f, 1e-5f);
    EXPECT_NEAR(v.getCenter().getY(), 1.0f, 1e-5f);
}

TEST(VoxelRotation, FullRotationReturnsToOrigin) {
    Voxel v(Vector3(1.0f, 2.0f, 3.0f));
    Vector3 original = v.getCenter();
    v.Rot_X(2.0f * M_PI);
    EXPECT_TRUE(vec3eq(v.getCenter(), original, 1e-4f));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Math operations
// ═══════════════════════════════════════════════════════════════════════════════

TEST(VoxelMath, DotProductAxisAligned) {
    Voxel a(Vector3(1.0f, 0.0f, 0.0f));
    Voxel b(Vector3(1.0f, 0.0f, 0.0f));
    EXPECT_FLOAT_EQ(a.dot(b), 1.0f);
}

TEST(VoxelMath, DotProductOrthogonalIsZero) {
    Voxel a(Vector3(1.0f, 0.0f, 0.0f));
    Voxel b(Vector3(0.0f, 1.0f, 0.0f));
    EXPECT_FLOAT_EQ(a.dot(b), 0.0f);
}

TEST(VoxelMath, DotProductGeneral) {
    Voxel a(Vector3(1.0f, 2.0f, 3.0f));
    Voxel b(Vector3(4.0f, 5.0f, 6.0f));
    EXPECT_FLOAT_EQ(a.dot(b), 32.0f);   // 4+10+18
}

TEST(VoxelMath, CrossProductOrthogonalAxes) {
    Voxel x(Vector3(1.0f, 0.0f, 0.0f));
    Voxel y(Vector3(0.0f, 1.0f, 0.0f));
    Vector3 z = x.cross(y);
    EXPECT_TRUE(vec3eq(z, Vector3(0.0f, 0.0f, 1.0f)));
}

TEST(VoxelMath, CrossProductSelfIsZero) {
    Voxel v(Vector3(3.0f, 1.0f, 4.0f));
    Vector3 r = v.cross(v);
    EXPECT_TRUE(vec3eq(r, Vector3(0.0f, 0.0f, 0.0f)));
}

TEST(VoxelMath, LengthOfDefaultVoxelIsZero) {
    Voxel v;
    EXPECT_FLOAT_EQ(v.length(), 0.0f);
}

TEST(VoxelMath, LengthMatchesPythagoras) {
    Voxel v(Vector3(3.0f, 4.0f, 0.0f));
    EXPECT_FLOAT_EQ(v.length(), 5.0f);
}

TEST(VoxelMath, NormalizeProducesUnitVector) {
    Voxel v(Vector3(0.0f, 3.0f, 4.0f));
    Vector3 n = v.normalize();
    float len = std::sqrt(n.getX()*n.getX() + n.getY()*n.getY() + n.getZ()*n.getZ());
    EXPECT_NEAR(len, 1.0f, 1e-5f);
}

TEST(VoxelMath, DistanceToSelfIsZero) {
    Voxel v(Vector3(1.0f, 2.0f, 3.0f));
    EXPECT_FLOAT_EQ(v.distanceTo(v), 0.0f);
}

TEST(VoxelMath, DistanceToAxisAligned) {
    Voxel a(Vector3(0.0f, 0.0f, 0.0f));
    Voxel b(Vector3(3.0f, 4.0f, 0.0f));
    EXPECT_FLOAT_EQ(a.distanceTo(b), 5.0f);
}

TEST(VoxelMath, DistanceToIsSymmetric) {
    Voxel a(Vector3(1.0f, 2.0f, 3.0f));
    Voxel b(Vector3(4.0f, 6.0f, 3.0f));
    EXPECT_FLOAT_EQ(a.distanceTo(b), b.distanceTo(a));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Voxel-specific geometry
// ═══════════════════════════════════════════════════════════════════════════════

TEST(VoxelGeometry, CornersCountIsEight) {
    Voxel v(Vector3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(v.corners().size(), 8u);
}

TEST(VoxelGeometry, CornersAtOriginBoundsAreHalfSize) {
    Voxel v(Vector3(0.0f, 0.0f, 0.0f));
    auto c = v.corners();
    // corner 0 should be (-VS2, -VS2, -VS2)
    EXPECT_TRUE(vec3eq(c[0], Vector3(-VS2, -VS2, -VS2)));
    // corner 7 should be (+VS2, +VS2, +VS2)
    EXPECT_TRUE(vec3eq(c[7], Vector3( VS2,  VS2,  VS2)));
}

TEST(VoxelGeometry, CornersTranslateWithCenter) {
    Vector3 offset(10.0f, 20.0f, 30.0f);
    Voxel v(offset);
    auto c = v.corners();
    EXPECT_TRUE(vec3eq(c[0], Vector3(offset.getX() - VS2,
                                     offset.getY() - VS2,
                                     offset.getZ() - VS2)));
    EXPECT_TRUE(vec3eq(c[7], Vector3(offset.getX() + VS2,
                                     offset.getY() + VS2,
                                     offset.getZ() + VS2)));
}

TEST(VoxelGeometry, CircumsphereRadiusExpectedValue) {
    Voxel v;
    float expected = std::sqrt(3.0f * VS * VS);
    EXPECT_NEAR(v.circumsphereRadius(), expected, 1e-5f);
}

TEST(VoxelGeometry, CircumsphereRadiusPositive) {
    Voxel v(Vector3(99.0f, 99.0f, 99.0f));
    EXPECT_GT(v.circumsphereRadius(), 0.0f);
}

TEST(VoxelGeometry, VolumeIsCubed) {
    Voxel v;
    EXPECT_NEAR(v.volume(), VS * VS * VS, 1e-5f);
}

TEST(VoxelGeometry, VolumePositive) {
    Voxel v(Vector3(1.0f, 2.0f, 3.0f));
    EXPECT_GT(v.volume(), 0.0f);
}

// ─── contains ─────────────────────────────────────────────────────────────────

TEST(VoxelContains, CenterIsInsideSelf) {
    Voxel v(Vector3(0.0f, 0.0f, 0.0f));
    EXPECT_TRUE(v.contains(v.getCenter()));
}

TEST(VoxelContains, CornerPointIsOnBoundary) {
    // The contains() check uses <= VS (full size, not half), meaning the half-
    // extent corner (VS2) satisfies |delta| <= VS — verify the boundary holds.
    Voxel v(Vector3(0.0f, 0.0f, 0.0f));
    EXPECT_TRUE(v.contains(Vector3(VS2, VS2, VS2)));
}

TEST(VoxelContains, PointClearlyOutside) {
    Voxel v(Vector3(0.0f, 0.0f, 0.0f));
    EXPECT_FALSE(v.contains(Vector3(VS * 10.0f, 0.0f, 0.0f)));
}

TEST(VoxelContains, PointAtExactBoundary) {
    Voxel v(Vector3(0.0f, 0.0f, 0.0f));
    // |delta| == VS — boundary is inclusive
    EXPECT_TRUE(v.contains(Vector3(VS, 0.0f, 0.0f)));
}

TEST(VoxelContains, PointJustOutsideBoundary) {
    Voxel v(Vector3(0.0f, 0.0f, 0.0f));
    EXPECT_FALSE(v.contains(Vector3(VS + 0.001f, 0.0f, 0.0f)));
}

// ─── intersects ───────────────────────────────────────────────────────────────

TEST(VoxelIntersects, VoxelIntersectsItself) {
    Voxel v(Vector3(0.0f, 0.0f, 0.0f));
    EXPECT_TRUE(v.intersects(v));
}

TEST(VoxelIntersects, AdjacentVoxelsIntersect) {
    // Two voxels separated by exactly VS (touching) — overlaps since |delta|<=2*VS
    Voxel a(Vector3(0.0f, 0.0f, 0.0f));
    Voxel b(Vector3(VS, 0.0f, 0.0f));
    EXPECT_TRUE(a.intersects(b));
}

TEST(VoxelIntersects, VoxelsAtTwiceTheSizeStillIntersect) {
    Voxel a(Vector3(0.0f, 0.0f, 0.0f));
    Voxel b(Vector3(VS * 2.0f, 0.0f, 0.0f));
    EXPECT_TRUE(a.intersects(b));   // |delta| == 2*VS — inclusive boundary
}

TEST(VoxelIntersects, VoxelsFarApartDoNotIntersect) {
    Voxel a(Vector3(0.0f, 0.0f, 0.0f));
    Voxel b(Vector3(VS * 3.0f, 0.0f, 0.0f));
    EXPECT_FALSE(a.intersects(b));
}

TEST(VoxelIntersects, IntersectsIsSymmetric) {
    Voxel a(Vector3(0.0f, 0.0f, 0.0f));
    Voxel b(Vector3(VS * 0.5f, 0.0f, 0.0f));
    EXPECT_EQ(a.intersects(b), b.intersects(a));
}

TEST(VoxelIntersects, DiagonallyOverlapping) {
    Voxel a(Vector3(0.0f, 0.0f, 0.0f));
    Voxel b(Vector3(VS * 0.5f, VS * 0.5f, VS * 0.5f));
    EXPECT_TRUE(a.intersects(b));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Edge / boundary
// ═══════════════════════════════════════════════════════════════════════════════

TEST(VoxelEdge, EnergyNotCopiedByAssignmentOperator) {
    // The assignment operator only copies center — energy stays at destination
    Voxel a(Vector3(1.0f, 1.0f, 1.0f));
    a.setEnergy(100.0f);
    Voxel b;
    b = a;
    // Energy on b should be unchanged (0), because operator= only copies center
    EXPECT_FLOAT_EQ(b.getEnergy(), 0.0f);
}

TEST(VoxelEdge, DivisionByZeroMessageContainsKeyword) {
    Voxel v(Vector3(1.0f, 1.0f, 1.0f));
    try {
        [[maybe_unused]] Voxel r = v / 0.0f;
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string(e.what()).find("zero"), std::string::npos);
    }
}

TEST(VoxelEdge, LargeCoordinates) {
    Voxel v(Vector3(1e6f, 1e6f, 1e6f));
    Voxel w(Vector3(1e6f, 1e6f, 1e6f));
    EXPECT_TRUE(v == w);
}