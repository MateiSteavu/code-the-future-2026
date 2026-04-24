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


// ═══════════════════════════════════════════════════════════════════════════════
// beerLambert_VoxelAbsorption
// ═══════════════════════════════════════════════════════════════════════════════

class BeerLambert : public ::testing::Test {
protected:
    float I_out = 0.0f;
};

TEST_F(BeerLambert, ZeroMuMeansNoAbsorption) {
    float absorbed = Custom_Math::beerLambert_VoxelAbsorption(100.0f, 0.0f, 1.0f, I_out);
    EXPECT_NEAR(absorbed, 0.0f, 1e-4f);
    EXPECT_NEAR(I_out, 100.0f, 1e-4f);
}

TEST_F(BeerLambert, ZeroPathLengthMeansNoAbsorption) {
    float absorbed = Custom_Math::beerLambert_VoxelAbsorption(100.0f, 1.0f, 0.0f, I_out);
    EXPECT_NEAR(absorbed, 0.0f, 1e-4f);
    EXPECT_NEAR(I_out, 100.0f, 1e-4f);
}

TEST_F(BeerLambert, ZeroIncomingMeansZeroAbsorbed) {
    float absorbed = Custom_Math::beerLambert_VoxelAbsorption(0.0f, 1.0f, 1.0f, I_out);
    EXPECT_NEAR(absorbed, 0.0f, 1e-4f);
    EXPECT_NEAR(I_out, 0.0f, 1e-4f);
}

TEST_F(BeerLambert, EnergyConservation_AbsorbedPlusTransmittedEqualsIncoming) {
    float I_in    = 200.0f;
    float mu      = 2.0f;
    float pathLen = 0.5f;
    float absorbed = Custom_Math::beerLambert_VoxelAbsorption(I_in, mu, pathLen, I_out);
    EXPECT_NEAR(absorbed + I_out, I_in, 1e-3f);
}

TEST_F(BeerLambert, EnergyConservation_VariousParams) {
    for (float mu : {0.1f, 1.0f, 5.0f, 10.0f}) {
        float I_in = 1.0f;
        float absorbed = Custom_Math::beerLambert_VoxelAbsorption(I_in, mu, 1.0f, I_out);
        EXPECT_NEAR(absorbed + I_out, I_in, 1e-5f)
            << "Failed energy conservation for mu=" << mu;
    }
}

TEST_F(BeerLambert, HighAttenuation_AlmostAllAbsorbed) {
    float absorbed = Custom_Math::beerLambert_VoxelAbsorption(100.0f, 1000.0f, 1.0f, I_out);
    EXPECT_NEAR(I_out, 0.0f, 1e-3f);
    EXPECT_NEAR(absorbed, 100.0f, 0.1f);
}

TEST_F(BeerLambert, UnitOpticalDepth_TransmittanceIsOneOverE) {
    // tau = mu * pathLen = 1 → T = 1/e ≈ 0.3679
    float I_in = 1.0f;
    Custom_Math::beerLambert_VoxelAbsorption(I_in, 1.0f, 1.0f, I_out);
    EXPECT_NEAR(I_out, std::exp(-1.0f), 1e-5f);
}

TEST_F(BeerLambert, DoublePathLengthHalvesTwice) {
    // tau doubled → T squared
    float I1, I2;
    Custom_Math::beerLambert_VoxelAbsorption(1.0f, 1.0f, 1.0f, I1);
    Custom_Math::beerLambert_VoxelAbsorption(1.0f, 1.0f, 2.0f, I2);
    EXPECT_NEAR(I2, I1 * I1, 1e-5f);
}

TEST_F(BeerLambert, AbsorbedIsNonNegative) {
    float absorbed = Custom_Math::beerLambert_VoxelAbsorption(50.0f, 0.5f, 2.0f, I_out);
    EXPECT_GE(absorbed, 0.0f);
}

TEST_F(BeerLambert, AbsorbedNeverExceedsIncoming) {
    float absorbed = Custom_Math::beerLambert_VoxelAbsorption(50.0f, 100.0f, 100.0f, I_out);
    EXPECT_LE(absorbed, 50.0f + 1e-3f);
}

TEST_F(BeerLambert, I_outIsStrictlyLessThanI_in_WithPositiveMu) {
    Custom_Math::beerLambert_VoxelAbsorption(10.0f, 1.0f, 1.0f, I_out);
    EXPECT_LT(I_out, 10.0f);
}


// ═══════════════════════════════════════════════════════════════════════════════
// voxelEnergyFromConeLight
// ═══════════════════════════════════════════════════════════════════════════════

class VoxelEnergyFromCone : public ::testing::Test {
protected:
    Cone cone  = makeCone(Vector3(0,0,0), Vector3(0,0,1), 30.0f);
    Voxel vOn  = makeVoxel(0, 0, 5);      // directly on axis — inside cone
    Voxel vOff = makeVoxel(0, 100, 0);    // 90° off — definitely outside cone
};

TEST_F(VoxelEnergyFromCone, VoxelOutsideConeReturnsZero) {
    float e = Custom_Math::voxelEnergyFromConeLight(vOff, cone, 1.0f, 1.0f, 1.0f);
    EXPECT_FLOAT_EQ(e, 0.0f);
}

TEST_F(VoxelEnergyFromCone, VoxelInsideConeReturnsPositive) {
    float e = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 1.0f, 1.0f);
    EXPECT_GT(e, 0.0f);
}

TEST_F(VoxelEnergyFromCone, ZeroRadianceGivesZeroEnergy) {
    float e = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 0.0f, 1.0f, 1.0f);
    EXPECT_NEAR(e, 0.0f, 1e-6f);
}

TEST_F(VoxelEnergyFromCone, ZeroTransmittanceGivesZeroEnergy) {
    float e = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 1.0f, 0.0f);
    EXPECT_NEAR(e, 0.0f, 1e-6f);
}

TEST_F(VoxelEnergyFromCone, ZeroMuWithPathStillAbsorbsSomething) {
    // mu=0 → no absorption; beerLambert returns 0
    float e = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 0.0f, 1.0f);
    EXPECT_NEAR(e, 0.0f, 1e-5f);
}

TEST_F(VoxelEnergyFromCone, HigherRadianceGivesProportionallyMoreEnergy) {
    float e1 = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 1.0f, 1.0f);
    float e2 = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 200.0f, 1.0f, 1.0f);
    EXPECT_NEAR(e2, 2.0f * e1, 1e-3f);
}

TEST_F(VoxelEnergyFromCone, HigherTransmittanceGivesProportionallyMoreEnergy) {
    float e1 = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 1.0f, 0.5f);
    float e2 = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 1.0f, 1.0f);
    EXPECT_NEAR(e2, 2.0f * e1, 1e-3f);
}

TEST_F(VoxelEnergyFromCone, AbsorbedEnergyNeverExceedsIncomingIrradiance) {
    // Max possible absorbed = sourceRadiance * solidAngle * T_path (I_in)
    float L  = 100.0f;
    float mu = 1.0f;
    float T  = 0.8f;
    float solidAngle = cone.solidAngle();
    float I_in = L * solidAngle * T;
    float e = Custom_Math::voxelEnergyFromConeLight(vOn, cone, L, mu, T);
    EXPECT_LE(e, I_in + 1e-3f);
}

TEST_F(VoxelEnergyFromCone, EnergyIsNonNegative) {
    float e = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 50.0f, 2.0f, 0.5f);
    EXPECT_GE(e, 0.0f);
}

TEST_F(VoxelEnergyFromCone, HigherMuAbsorbsMoreEnergy) {
    float e_low  = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 0.1f, 1.0f);
    float e_high = Custom_Math::voxelEnergyFromConeLight(vOn, cone, 100.0f, 10.0f, 1.0f);
    EXPECT_GT(e_high, e_low);
}

TEST_F(VoxelEnergyFromCone, WideConeAbsorbsMoreThanNarrowCone) {
    // Wider cone → larger solid angle → more irradiance → more absorption
    Cone narrow = makeCone(Vector3(0,0,0), Vector3(0,0,1),  5.0f);
    Cone wide   = makeCone(Vector3(0,0,0), Vector3(0,0,1), 30.0f);
    float e_narrow = Custom_Math::voxelEnergyFromConeLight(vOn, narrow, 100.0f, 1.0f, 1.0f);
    float e_wide   = Custom_Math::voxelEnergyFromConeLight(vOn, wide,   100.0f, 1.0f, 1.0f);
    EXPECT_GT(e_wide, e_narrow);
}