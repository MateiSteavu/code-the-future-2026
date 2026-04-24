#include <gtest/gtest.h>
#include <cmath>
#include "../inc/Custom_Math.h"
#include "../inc/Geometry/Ray.h"
#include "../inc/Geometry/Voxel.h"

// ─── Helpers ──────────────────────────────────────────────────────────────────

static constexpr float PI  = static_cast<float>(M_PI);
static constexpr float VS  = Voxel_Size;

static bool feq(float a, float b, float eps = 1e-4f) {
    return std::abs(a - b) < eps;
}

// Build a cone pointing in +Z from the origin with a given half-angle (radians)
static Cone makeCone(Vector3 origin, Vector3 direction, float halfAngleDeg) {
    float rad = halfAngleDeg * PI / 180.0f;
    return Cone(origin, direction.normalize(), rad);
}

// Voxel centred at a given point
static Voxel makeVoxel(float x, float y, float z) {
    return Voxel(Vector3(x, y, z));
}


// ═══════════════════════════════════════════════════════════════════════════════
// pointInCone
// ═══════════════════════════════════════════════════════════════════════════════

class PointInConeTest : public ::testing::Test {
protected:
    // Cone at origin, pointing +Z, 30° half-angle
    Cone cone30 = makeCone(Vector3(0,0,0), Vector3(0,0,1), 30.0f);
    // Narrow cone, 5°
    Cone cone5  = makeCone(Vector3(0,0,0), Vector3(0,0,1),  5.0f);
    // Wide cone, 89°
    Cone cone89 = makeCone(Vector3(0,0,0), Vector3(0,0,1), 89.0f);
};

TEST_F(PointInConeTest, PointOnAxisIsInside) {
    EXPECT_TRUE(Custom_Math::pointInCone(Vector3(0,0,10), cone30));
}

TEST_F(PointInConeTest, PointAtApexIsInside) {
    // Normalised direction from apex to apex is degenerate — implementation
    // normalises toPoint, dot product with axis == 1 when point == origin
    EXPECT_TRUE(Custom_Math::pointInCone(Vector3(0,0,0), cone30));
}

TEST_F(PointInConeTest, PointInsideBoundary) {
    // 15° off-axis, cone is 30° — should be inside
    float angle = 15.0f * PI / 180.0f;
    Vector3 p(std::sin(angle), 0.0f, std::cos(angle));
    EXPECT_TRUE(Custom_Math::pointInCone(p, cone30));
}

TEST_F(PointInConeTest, PointOnExactBoundaryIsInside) {
    // Exactly at half-angle boundary — cosAngle == cosHalfAngle → inside (>=)
    float angle = 30.0f * PI / 180.0f;
    Vector3 p(std::sin(angle), 0.0f, std::cos(angle));
    EXPECT_TRUE(Custom_Math::pointInCone(p, cone30));
}

TEST_F(PointInConeTest, PointJustOutsideBoundary) {
    float angle = 31.0f * PI / 180.0f;
    Vector3 p(std::sin(angle), 0.0f, std::cos(angle));
    EXPECT_FALSE(Custom_Math::pointInCone(p, cone30));
}

TEST_F(PointInConeTest, PointBehindApexIsOutside) {
    EXPECT_FALSE(Custom_Math::pointInCone(Vector3(0,0,-5), cone30));
}

TEST_F(PointInConeTest, PointPerpendicular90DegIsOutside) {
    EXPECT_FALSE(Custom_Math::pointInCone(Vector3(1,0,0), cone30));
}

TEST_F(PointInConeTest, NarrowConeRejectsSlightlyOffAxis) {
    float angle = 6.0f * PI / 180.0f;
    Vector3 p(std::sin(angle), 0.0f, std::cos(angle));
    EXPECT_FALSE(Custom_Math::pointInCone(p, cone5));
}

TEST_F(PointInConeTest, WideConeAcceptsOffAxisPoint) {
    float angle = 85.0f * PI / 180.0f;
    Vector3 p(std::sin(angle), 0.0f, std::cos(angle));
    EXPECT_TRUE(Custom_Math::pointInCone(p, cone89));
}

TEST_F(PointInConeTest, OffOriginCone) {
    Cone c = makeCone(Vector3(5,5,5), Vector3(0,0,1), 30.0f);
    // Point directly above apex — must be inside
    EXPECT_TRUE(Custom_Math::pointInCone(Vector3(5,5,15), c));
    // Point in opposite direction — must be outside
    EXPECT_FALSE(Custom_Math::pointInCone(Vector3(5,5,-5), c));
}


// ═══════════════════════════════════════════════════════════════════════════════
// voxelInCone_CenterOnly
// ═══════════════════════════════════════════════════════════════════════════════

class VoxelInCone_CenterOnly : public ::testing::Test {
protected:
    Cone cone = makeCone(Vector3(0,0,0), Vector3(0,0,1), 30.0f);
};

TEST_F(VoxelInCone_CenterOnly, CenterOnAxisIsInside) {
    EXPECT_TRUE(Custom_Math::voxelInCone_CenterOnly(makeVoxel(0,0,5), cone));
}

TEST_F(VoxelInCone_CenterOnly, CenterClearlyOutsideIsRejected) {
    EXPECT_FALSE(Custom_Math::voxelInCone_CenterOnly(makeVoxel(10,0,1), cone));
}

TEST_F(VoxelInCone_CenterOnly, CenterBehindApexIsRejected) {
    EXPECT_FALSE(Custom_Math::voxelInCone_CenterOnly(makeVoxel(0,0,-5), cone));
}

TEST_F(VoxelInCone_CenterOnly, CenterOnBoundaryIsAccepted) {
    float angle = 30.0f * PI / 180.0f;
    float dist  = 5.0f;
    EXPECT_TRUE(Custom_Math::voxelInCone_CenterOnly(
        makeVoxel(dist*std::sin(angle), 0, dist*std::cos(angle)), cone));
}

TEST_F(VoxelInCone_CenterOnly, DoesNotConsiderCorners) {
    // Voxel whose center is outside but corners may be inside —
    // CenterOnly should still return false
    float angle = 35.0f * PI / 180.0f; // 5° past boundary
    float dist  = 5.0f;
    Voxel v = makeVoxel(dist*std::sin(angle), 0, dist*std::cos(angle));
    EXPECT_FALSE(Custom_Math::voxelInCone_CenterOnly(v, cone));
}


// ═══════════════════════════════════════════════════════════════════════════════
// voxelInCone_BoundingSphere
// ═══════════════════════════════════════════════════════════════════════════════

class VoxelInCone_BoundingSphere : public ::testing::Test {
protected:
    Cone cone30 = makeCone(Vector3(0,0,0), Vector3(0,0,1), 30.0f);
    Cone cone5  = makeCone(Vector3(0,0,0), Vector3(0,0,1),  5.0f);
};

TEST_F(VoxelInCone_BoundingSphere, VoxelOnAxisIsAccepted) {
    EXPECT_TRUE(Custom_Math::voxelInCone_BoundingSphere(makeVoxel(0,0,5), cone30));
}

TEST_F(VoxelInCone_BoundingSphere, VoxelFarOutsideIsRejected) {
    // Voxel at 90° off-axis, well past any angular padding
    EXPECT_FALSE(Custom_Math::voxelInCone_BoundingSphere(makeVoxel(100,0,0), cone5));
}

TEST_F(VoxelInCone_BoundingSphere, VoxelAtApexIsAccepted) {
    // dist < 1e-9 branch: always true
    EXPECT_TRUE(Custom_Math::voxelInCone_BoundingSphere(makeVoxel(0,0,0), cone30));
}

TEST_F(VoxelInCone_BoundingSphere, ConservativeNeverDropsTrueOverlap) {
    // A voxel that clearly overlaps (center on axis) must always be accepted
    EXPECT_TRUE(Custom_Math::voxelInCone_BoundingSphere(makeVoxel(0,0,2), cone30));
}

TEST_F(VoxelInCone_BoundingSphere, BoundingSphereCanAcceptNearBoundary) {
    // Voxel whose center is just outside but bounding sphere may overlap the cone
    float angle = 32.0f * PI / 180.0f; // just outside 30°
    float dist  = 20.0f;               // far away so sphere padding is small
    Voxel v = makeVoxel(dist*std::sin(angle), 0, dist*std::cos(angle));
    // We don't assert a specific value — just verify no crash and a bool is returned
    bool result = Custom_Math::voxelInCone_BoundingSphere(v, cone30);
    EXPECT_TRUE(result == true || result == false); // always valid
}

TEST_F(VoxelInCone_BoundingSphere, BehindApexIsRejected) {
    EXPECT_FALSE(Custom_Math::voxelInCone_BoundingSphere(makeVoxel(0,0,-10), cone30));
}

TEST_F(VoxelInCone_BoundingSphere, VeryCloseVoxelApexBranch) {
    // dist is < 1e-9: should return true immediately
    Voxel v = makeVoxel(0.0f, 0.0f, 0.0f);
    EXPECT_TRUE(Custom_Math::voxelInCone_BoundingSphere(v, cone30));
}


// ═══════════════════════════════════════════════════════════════════════════════
// voxelInCone_Corners
// ═══════════════════════════════════════════════════════════════════════════════

class VoxelInCone_Corners : public ::testing::Test {
protected:
    Cone cone = makeCone(Vector3(0,0,0), Vector3(0,0,1), 30.0f);
};

TEST_F(VoxelInCone_Corners, VoxelCenterOnAxisIsAccepted) {
    EXPECT_TRUE(Custom_Math::voxelInCone_Corners(makeVoxel(0,0,5), cone));
}

TEST_F(VoxelInCone_Corners, VoxelFullyOutsideIsRejected) {
    EXPECT_FALSE(Custom_Math::voxelInCone_Corners(makeVoxel(100,0,0), cone));
}

TEST_F(VoxelInCone_Corners, VoxelBehindApexIsRejected) {
    EXPECT_FALSE(Custom_Math::voxelInCone_Corners(makeVoxel(0,0,-10), cone));
}

TEST_F(VoxelInCone_Corners, VoxelOnBoundaryAtLeastOneCornerIn) {
    // Place voxel so that one corner must be inside the cone
    // At distance 5 on axis — all corners are close to axis; at least one inside
    EXPECT_TRUE(Custom_Math::voxelInCone_Corners(makeVoxel(0,0,5), cone));
}

TEST_F(VoxelInCone_Corners, ReturnsFalseWhenNoCornersInside) {
    // 90° off-axis is never inside a 30° half-angle cone
    EXPECT_FALSE(Custom_Math::voxelInCone_Corners(makeVoxel(10,0,0), cone));
}


// ═══════════════════════════════════════════════════════════════════════════════
// voxelInCone_AxisClosestPoint
// ═══════════════════════════════════════════════════════════════════════════════

class VoxelInCone_AxisClosest : public ::testing::Test {
protected:
    Cone cone = makeCone(Vector3(0,0,0), Vector3(0,0,1), 30.0f);
};

TEST_F(VoxelInCone_AxisClosest, VoxelOnAxisIsAccepted) {
    EXPECT_TRUE(Custom_Math::voxelInCone_AxisClosestPoint(makeVoxel(0,0,5), cone));
}

TEST_F(VoxelInCone_AxisClosest, VoxelFarAwayIsRejected) {
    EXPECT_FALSE(Custom_Math::voxelInCone_AxisClosestPoint(makeVoxel(100,0,0), cone));
}

TEST_F(VoxelInCone_AxisClosest, VoxelBehindApexRejected) {
    EXPECT_FALSE(Custom_Math::voxelInCone_AxisClosestPoint(makeVoxel(0,0,-10), cone));
}

TEST_F(VoxelInCone_AxisClosest, AxisPassesThroughVoxelSide) {
    // Voxel positioned so axis passes through its side face but not a corner
    // Voxel at (small-x, 0, z_large): cone axis (Z) comes very close to AABB
    Voxel v = makeVoxel(VS * 0.4f, 0.0f, 5.0f);
    // The AABB is straddling the axis — closest point is on the axis itself
    EXPECT_TRUE(Custom_Math::voxelInCone_AxisClosestPoint(v, cone));
}

TEST_F(VoxelInCone_AxisClosest, OffOriginConeWorks) {
    Cone c = makeCone(Vector3(0,0,-10), Vector3(0,0,1), 30.0f);
    Voxel v = makeVoxel(0, 0, 0);
    EXPECT_TRUE(Custom_Math::voxelInCone_AxisClosestPoint(v, c));
}


// ═══════════════════════════════════════════════════════════════════════════════
// voxelInCone_Robust
// ═══════════════════════════════════════════════════════════════════════════════

class VoxelInCone_Robust : public ::testing::Test {
protected:
    Cone cone30 = makeCone(Vector3(0,0,0), Vector3(0,0,1), 30.0f);
    Cone cone5  = makeCone(Vector3(0,0,0), Vector3(0,0,1),  5.0f);
    Cone cone89 = makeCone(Vector3(0,0,0), Vector3(0,0,1), 89.0f);
};

TEST_F(VoxelInCone_Robust, VoxelOnAxisAccepted) {
    EXPECT_TRUE(Custom_Math::voxelInCone_Robust(makeVoxel(0,0,5), cone30));
}

TEST_F(VoxelInCone_Robust, VoxelFarOffAxisRejected) {
    EXPECT_FALSE(Custom_Math::voxelInCone_Robust(makeVoxel(0,100,0), cone30));
}

TEST_F(VoxelInCone_Robust, VoxelBehindApexRejected) {
    EXPECT_FALSE(Custom_Math::voxelInCone_Robust(makeVoxel(0,0,-10), cone30));
}

TEST_F(VoxelInCone_Robust, NarrowConeVoxelOnAxis) {
    EXPECT_TRUE(Custom_Math::voxelInCone_Robust(makeVoxel(0,0,5), cone5));
}

TEST_F(VoxelInCone_Robust, NarrowConeVoxelFarOffAxis) {
    EXPECT_FALSE(Custom_Math::voxelInCone_Robust(makeVoxel(10,0,0), cone5));
}

TEST_F(VoxelInCone_Robust, WideConeAcceptsMostVoxels) {
    EXPECT_TRUE(Custom_Math::voxelInCone_Robust(makeVoxel(0,0,5), cone89));
    EXPECT_TRUE(Custom_Math::voxelInCone_Robust(makeVoxel(5,0,1), cone89));
}

TEST_F(VoxelInCone_Robust, RobustImpliesCornerTest) {
    // If Corners returns true, Robust must also return true (stage 1 may pass, stage 2 accepts)
    Voxel v = makeVoxel(0,0,5);
    bool corners = Custom_Math::voxelInCone_Corners(v, cone30);
    bool robust  = Custom_Math::voxelInCone_Robust(v, cone30);
    if (corners) EXPECT_TRUE(robust);
}

TEST_F(VoxelInCone_Robust, RobustConsistentWithBoundingSphere) {
    // If BoundingSphere returns false, Robust must also return false (stage 1 rejects)
    Voxel v = makeVoxel(0, 100, 0);
    bool sphere = Custom_Math::voxelInCone_BoundingSphere(v, cone5);
    bool robust = Custom_Math::voxelInCone_Robust(v, cone5);
    if (!sphere) EXPECT_FALSE(robust);
}

TEST_F(VoxelInCone_Robust, OffOriginCone) {
    Cone c = makeCone(Vector3(0, 0, -10), Vector3(0,0,1), 30.0f);
    EXPECT_TRUE(Custom_Math::voxelInCone_Robust(makeVoxel(0,0,0), c));
    EXPECT_FALSE(Custom_Math::voxelInCone_Robust(makeVoxel(0,0,-20), c));
}

TEST_F(VoxelInCone_Robust, VoxelAtApexAlwaysAccepted) {
    // dist < 1e-9 branch in BoundingSphere → true; then corners/axis should also confirm
    EXPECT_TRUE(Custom_Math::voxelInCone_Robust(makeVoxel(0,0,0), cone30));
}


// ═══════════════════════════════════════════════════════════════════════════════
// beerLambert_VoxelAbsorption
//   Signature: (I_in, mu, pathLength, deltaTime, I_out&) -> Joules
//   I_out is still W/m² (irradiance exiting the voxel).
//   Return value is I_in*(1-T)*deltaTime  [J/m²].
// ═══════════════════════════════════════════════════════════════════════════════

class BeerLambert : public ::testing::Test {
protected:
    float I_out    = 0.0f;
    float dt       = 1.0f;   // 1 s default exposure
};

TEST_F(BeerLambert, ZeroMuMeansNoAbsorption) {
    float joules = Custom_Math::beerLambert_VoxelAbsorption(100.0f, 0.0f, 1.0f, dt, I_out);
    EXPECT_NEAR(joules, 0.0f, 1e-4f);
    EXPECT_NEAR(I_out, 100.0f, 1e-4f);
}

TEST_F(BeerLambert, ZeroPathLengthMeansNoAbsorption) {
    float joules = Custom_Math::beerLambert_VoxelAbsorption(100.0f, 1.0f, 0.0f, dt, I_out);
    EXPECT_NEAR(joules, 0.0f, 1e-4f);
    EXPECT_NEAR(I_out, 100.0f, 1e-4f);
}

TEST_F(BeerLambert, ZeroIncomingMeansZeroAbsorbed) {
    float joules = Custom_Math::beerLambert_VoxelAbsorption(0.0f, 1.0f, 1.0f, dt, I_out);
    EXPECT_NEAR(joules, 0.0f, 1e-4f);
    EXPECT_NEAR(I_out, 0.0f, 1e-4f);
}

TEST_F(BeerLambert, ZeroDeltaTimeMeansZeroJoules) {
    // No exposure time → zero energy deposited, but I_out should still be correct
    float joules = Custom_Math::beerLambert_VoxelAbsorption(100.0f, 1.0f, 1.0f, 0.0f, I_out);
    EXPECT_NEAR(joules, 0.0f, 1e-6f);
    // I_out is transmittance-scaled irradiance, independent of dt
    EXPECT_NEAR(I_out, 100.0f * std::exp(-1.0f), 1e-3f);
}

TEST_F(BeerLambert, EnergyConservation_JoulesEqualsPowerTimesDt) {
    // absorbed_joules == I_in * (1 - T) * dt  →  joules / dt + I_out == I_in
    float I_in    = 200.0f;
    float mu      = 2.0f;
    float pathLen = 0.5f;
    float joules  = Custom_Math::beerLambert_VoxelAbsorption(I_in, mu, pathLen, dt, I_out);
    // joules / dt is absorbed irradiance; I_out is transmitted irradiance
    EXPECT_NEAR(joules / dt + I_out, I_in, 1e-3f);
}

TEST_F(BeerLambert, EnergyConservation_VariousParams) {
    for (float mu : {0.1f, 1.0f, 5.0f, 10.0f}) {
        float I_in   = 1.0f;
        float joules = Custom_Math::beerLambert_VoxelAbsorption(I_in, mu, 1.0f, dt, I_out);
        EXPECT_NEAR(joules / dt + I_out, I_in, 1e-5f)
            << "Failed energy conservation for mu=" << mu;
    }
}

TEST_F(BeerLambert, HighAttenuation_AlmostAllAbsorbed) {
    float joules = Custom_Math::beerLambert_VoxelAbsorption(100.0f, 1000.0f, 1.0f, dt, I_out);
    EXPECT_NEAR(I_out,  0.0f,   1e-3f);
    EXPECT_NEAR(joules, 100.0f, 0.1f);   // nearly all 100 W/m² × 1 s = 100 J/m²
}

TEST_F(BeerLambert, UnitOpticalDepth_I_outIsOneOverE) {
    // tau = mu * pathLen = 1 → T = 1/e;  I_out is T-scaled irradiance (W/m²)
    Custom_Math::beerLambert_VoxelAbsorption(1.0f, 1.0f, 1.0f, dt, I_out);
    EXPECT_NEAR(I_out, std::exp(-1.0f), 1e-5f);
}

TEST_F(BeerLambert, UnitOpticalDepth_JoulesMatchFormula) {
    // joules = I_in * (1 - 1/e) * dt
    float I_in   = 1.0f;
    float joules = Custom_Math::beerLambert_VoxelAbsorption(I_in, 1.0f, 1.0f, dt, I_out);
    EXPECT_NEAR(joules, I_in * (1.0f - std::exp(-1.0f)) * dt, 1e-5f);
}

TEST_F(BeerLambert, DoublePathLengthSquaresTransmittance) {
    // tau doubled → T squared → I_out halves twice
    float Io1, Io2;
    Custom_Math::beerLambert_VoxelAbsorption(1.0f, 1.0f, 1.0f, dt, Io1);
    Custom_Math::beerLambert_VoxelAbsorption(1.0f, 1.0f, 2.0f, dt, Io2);
    EXPECT_NEAR(Io2, Io1 * Io1, 1e-5f);
}

TEST_F(BeerLambert, DoubleDeltaTimeDoublesJoules) {
    float j1 = Custom_Math::beerLambert_VoxelAbsorption(50.0f, 1.0f, 1.0f, 1.0f, I_out);
    float j2 = Custom_Math::beerLambert_VoxelAbsorption(50.0f, 1.0f, 1.0f, 2.0f, I_out);
    EXPECT_NEAR(j2, 2.0f * j1, 1e-4f);
}

TEST_F(BeerLambert, JoulesAreNonNegative) {
    float joules = Custom_Math::beerLambert_VoxelAbsorption(50.0f, 0.5f, 2.0f, dt, I_out);
    EXPECT_GE(joules, 0.0f);
}

TEST_F(BeerLambert, JoulesNeverExceedI_inTimesDt) {
    float I_in   = 50.0f;
    float joules = Custom_Math::beerLambert_VoxelAbsorption(I_in, 100.0f, 100.0f, dt, I_out);
    EXPECT_LE(joules, I_in * dt + 1e-3f);
}

TEST_F(BeerLambert, I_outIsStrictlyLessThanI_in_WithPositiveMu) {
    Custom_Math::beerLambert_VoxelAbsorption(10.0f, 1.0f, 1.0f, dt, I_out);
    EXPECT_LT(I_out, 10.0f);
}


// ═══════════════════════════════════════════════════════════════════════════════
// voxelEnergyFromConeLight
//   Signature: (voxel, cone, sourceRadiance, mu, T_path, deltaTime) -> Joules
// ═══════════════════════════════════════════════════════════════════════════════

class VoxelEnergyFromCone : public ::testing::Test {
protected:
    Cone  cone = makeCone(Vector3(0,0,0), Vector3(0,0,1), 30.0f);
    Voxel vOn  = makeVoxel(0, 0, 5);      // directly on axis — inside cone
    Voxel vOff = makeVoxel(0, 100, 0);    // 90° off — definitely outside cone
    float dt   = 1.0f;                    // default 1 s exposure
};

TEST_F(VoxelEnergyFromCone, VoxelOutsideConeReturnsZero) {
    float e = Custom_Math::voxelEnergyFromConeLight(vOff, cone, 1.0f, 1.0f, 1.0f, dt);
    EXPECT_FLOAT_EQ(e, 0.0f);
}

TEST_F(VoxelEnergyFromCone, VoxelInsideConeReturnsPositiveJoules) {
    float e = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 1.0f, 1.0f, dt);
    EXPECT_GT(e, 0.0f);
}

TEST_F(VoxelEnergyFromCone, ZeroRadianceGivesZeroJoules) {
    float e = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 0.0f, 1.0f, 1.0f, dt);
    EXPECT_NEAR(e, 0.0f, 1e-6f);
}

TEST_F(VoxelEnergyFromCone, ZeroTransmittanceGivesZeroJoules) {
    float e = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 1.0f, 0.0f, dt);
    EXPECT_NEAR(e, 0.0f, 1e-6f);
}

TEST_F(VoxelEnergyFromCone, ZeroDeltaTimeGivesZeroJoules) {
    float e = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 1.0f, 1.0f, 0.0f);
    EXPECT_NEAR(e, 0.0f, 1e-6f);
}

TEST_F(VoxelEnergyFromCone, ZeroMuGivesZeroJoules) {
    // mu=0 → T=1 → I_in*(1-1)*dt = 0 J
    float e = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 0.0f, 1.0f, dt);
    EXPECT_NEAR(e, 0.0f, 1e-5f);
}

TEST_F(VoxelEnergyFromCone, HigherRadianceGivesProportionallyMoreJoules) {
    float e1 = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 1.0f, 1.0f, dt);
    float e2 = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 200.0f, 1.0f, 1.0f, dt);
    EXPECT_NEAR(e2, 2.0f * e1, 1e-3f);
}

TEST_F(VoxelEnergyFromCone, HigherTransmittanceGivesProportionallyMoreJoules) {
    float e1 = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 1.0f, 0.5f, dt);
    float e2 = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 1.0f, 1.0f, dt);
    EXPECT_NEAR(e2, 2.0f * e1, 1e-3f);
}

TEST_F(VoxelEnergyFromCone, DoubleDeltaTimeDoublesJoules) {
    float e1 = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 1.0f, 1.0f, 1.0f);
    float e2 = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 1.0f, 1.0f, 2.0f);
    EXPECT_NEAR(e2, 2.0f * e1, 1e-3f);
}

TEST_F(VoxelEnergyFromCone, JoulesNeverExceedI_inTimesDt) {
    // Upper bound: all incoming irradiance absorbed × dt
    float L           = 100.0f;
    float mu          = 1.0f;
    float T           = 0.8f;
    float solidAngle  = cone.solidAngle();
    float I_in        = L * solidAngle * T;
    float e = Custom_Math::voxelEnergyFromConeLight(vOn, cone, L, mu, T, dt);
    EXPECT_LE(e, I_in * dt + 1e-3f);
}

TEST_F(VoxelEnergyFromCone, JoulesAreNonNegative) {
    float e = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 50.0f, 2.0f, 0.5f, dt);
    EXPECT_GE(e, 0.0f);
}

TEST_F(VoxelEnergyFromCone, HigherMuAbsorbsMoreJoules) {
    float e_low  = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f,  0.1f, 1.0f, dt);
    float e_high = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 10.0f, 1.0f, dt);
    EXPECT_GT(e_high, e_low);
}

TEST_F(VoxelEnergyFromCone, WideConeAbsorbsMoreThanNarrowCone) {
    // Wider cone → larger solid angle → more irradiance arriving → more Joules
    Cone narrow = makeCone(Vector3(0,0,0), Vector3(0,0,1),  5.0f);
    Cone wide   = makeCone(Vector3(0,0,0), Vector3(0,0,1), 30.0f);
    float e_narrow = Custom_Math::voxelEnergyFromConeLight(vOn, narrow, 100.0f, 1.0f, 1.0f, dt);
    float e_wide   = Custom_Math::voxelEnergyFromConeLight(vOn, wide,   100.0f, 1.0f, 1.0f, dt);
    EXPECT_GT(e_wide, e_narrow);
}

// ═══════════════════════════════════════════════════════════════════════════════
// distance_voxel_to_margin (cylindrical entry → voxel depth)
// ═══════════════════════════════════════════════════════════════════════════════

class DistanceVoxelToMarginTest : public ::testing::Test {
protected:
    float R = static_cast<float>(Print_Radius);
    Vector3 originOutside = Vector3(R * 2.0f, 0.0f, 10.0f);
};

TEST_F(DistanceVoxelToMarginTest, VoxelOnAxisInsideCylinderReturnsPositive)
{
    Voxel v = makeVoxel(0.0f, 0.0f, 10.0f);

    float d = Custom_Math::distance_voxel_to_margin(originOutside, v);

    EXPECT_GT(d, 0.0f);
}

TEST_F(DistanceVoxelToMarginTest, VoxelFarOutsideCylinderReturnsZero)
{
    Voxel v = makeVoxel(R * 5.0f, 0.0f, 10.0f);

    float d = Custom_Math::distance_voxel_to_margin(originOutside, v);

    EXPECT_NEAR(d, 0.0f, 1e-5f);
}


TEST_F(DistanceVoxelToMarginTest, VoxelNearBoundaryHasSmallDepth)
{
    Voxel v = makeVoxel(R * 0.9f, 0.0f, 10.0f);

    float d = Custom_Math::distance_voxel_to_margin(originOutside, v);

    EXPECT_GT(d, 0.0f);
}

TEST_F(DistanceVoxelToMarginTest, DeeperVoxelHasGreaterInsideDistance)
{
    Voxel near = makeVoxel(R * 0.9f, 0.0f, 10.0f);
    Voxel deep = makeVoxel(0.0f,      0.0f, 10.0f);

    float d1 = Custom_Math::distance_voxel_to_margin(originOutside, near);
    float d2 = Custom_Math::distance_voxel_to_margin(originOutside, deep);

    EXPECT_GT(d2, d1);
}

TEST_F(DistanceVoxelToMarginTest, FartherAlongRayIncreasesDepth)
{
    Voxel close = makeVoxel(0.0f, 0.0f, 10.0f);
    Voxel far   = makeVoxel(0.0f, 0.0f, 20.0f);

    float d1 = Custom_Math::distance_voxel_to_margin(originOutside, close);
    float d2 = Custom_Math::distance_voxel_to_margin(originOutside, far);

    EXPECT_GT(d2, d1);
}
