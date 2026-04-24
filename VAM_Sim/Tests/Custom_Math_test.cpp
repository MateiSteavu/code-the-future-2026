// test_Custom_Math.cpp
//
// Tests for Custom_Math
// Framework: Google Test (gtest)
// Build:  g++ -std=c++17 test_Custom_Math.cpp -lgtest -lgtest_main -o tests && ./tests
//
// Assumed test constants (override via your Print_Setup header):
//   Print_Radius = 5.0f
//   Voxel_Size   = 1.0f
//   mu           = 0.1f   (absorption coefficient)

#include <gtest/gtest.h>
#include <cmath>
#include "../inc/Custom_Math.h"
#include "../inc/Geometry/Ray.h"
#include "../inc/Geometry/Voxel.h"
#include "../inc/Geometry/Cone.h"
#include "../inc/Print_Setup/Setup.h"

// ─── Helpers ────────────────────────────────────────────────────────────────

static constexpr float EPS = 1e-4f;

// Cone pointing straight up (+Z), apex at origin, half-angle 30°
static Cone makeCone30()
{
    return Cone(Vector3(0, 0, 0),          // origin
                Vector3(0, 0, 1),          // direction (unit)
                30.0f * M_PI / 180.0f);   // half-angle in radians
}

// Voxel centered at a given point (unit cube, side = Voxel_Size)
static Voxel makeVoxel(float x, float y, float z)
{
    return Voxel(Vector3(x, y, z));
}


// ═══════════════════════════════════════════════════════════════════════════
//  1.  Radius_to_Voxel_array
// ═══════════════════════════════════════════════════════════════════════════

class RadiusToVoxelTest : public ::testing::Test {};

TEST_F(RadiusToVoxelTest, ReturnsNonEmptyGrid)
{
    Custom_Math cm;
    auto grid = cm.Radius_to_Voxel_array();
    EXPECT_FALSE(grid.empty());
}

TEST_F(RadiusToVoxelTest, AllVoxelCentersWithinRadius)
{
    Custom_Math cm;
    auto grid = cm.Radius_to_Voxel_array();
    const float r2 = Print_Radius * Print_Radius;

    for (const auto& row : grid) {
        for (const auto& v : row) {
            float cx = v.getCenter().getX();
            float cy = v.getCenter().getY();
            EXPECT_LE(cx * cx + cy * cy, r2 + EPS)
                << "Voxel center (" << cx << ", " << cy << ") outside radius";
        }
    }
}

TEST_F(RadiusToVoxelTest, VoxelCentersAreOnGrid)
{
    // Each center should lie on the half-integer lattice:
    // start + Voxel_Size * (i + 0.5)
    Custom_Math cm;
    auto grid = cm.Radius_to_Voxel_array();
    const float start = -Print_Radius;

    for (const auto& row : grid) {
        for (const auto& v : row) {
            float cx = v.getCenter().getX();
            float cy = v.getCenter().getY();

            float ix = (cx - start) / Voxel_Size - 0.5f;
            float iy = (cy - start) / Voxel_Size - 0.5f;

            EXPECT_NEAR(ix, std::round(ix), EPS) << "X center not on grid";
            EXPECT_NEAR(iy, std::round(iy), EPS) << "Y center not on grid";
        }
    }
}



// ═══════════════════════════════════════════════════════════════════════════
//  2.  pointInCone
// ═══════════════════════════════════════════════════════════════════════════

class PointInConeTest : public ::testing::Test
{
protected:
    Cone cone = makeCone30();
};

TEST_F(PointInConeTest, PointOnAxisIsInside)
{
    // Straight up the axis — always inside
    EXPECT_TRUE(Custom_Math::pointInCone(Vector3(0, 0, 5), cone));
}

TEST_F(PointInConeTest, PointWellOutsideIsRejected)
{
    // Perpendicular to the axis — 90° from direction, far outside 30° cone
    EXPECT_FALSE(Custom_Math::pointInCone(Vector3(5, 0, 0), cone));
}

TEST_F(PointInConeTest, PointExactlyOnBoundary)
{
    // At half-angle (30°): cos(30°) ≈ 0.866
    float angle = 30.0f * M_PI / 180.0f;
    Vector3 boundary(std::sin(angle), 0, std::cos(angle));
    EXPECT_TRUE(Custom_Math::pointInCone(boundary, cone));
}

TEST_F(PointInConeTest, PointJustOutsideBoundary)
{
    float angle = 31.0f * M_PI / 180.0f;
    Vector3 outside(std::sin(angle), 0, std::cos(angle));
    EXPECT_FALSE(Custom_Math::pointInCone(outside, cone));
}

TEST_F(PointInConeTest, NegativeAxisRejected)
{
    // Behind the apex (−Z direction)
    EXPECT_FALSE(Custom_Math::pointInCone(Vector3(0, 0, -5), cone));
}


// ═══════════════════════════════════════════════════════════════════════════
//  3.  voxelInCone_CenterOnly
// ═══════════════════════════════════════════════════════════════════════════

class VoxelInConeCenterTest : public ::testing::Test
{
protected:
    Cone cone = makeCone30();
};

TEST_F(VoxelInConeCenterTest, CenterOnAxisAccepted)
{
    EXPECT_TRUE(Custom_Math::voxelInCone_CenterOnly(makeVoxel(0, 0, 5), cone));
}

TEST_F(VoxelInConeCenterTest, CenterFarOutsideRejected)
{
    EXPECT_FALSE(Custom_Math::voxelInCone_CenterOnly(makeVoxel(10, 0, 0), cone));
}

TEST_F(VoxelInConeCenterTest, CenterOnBoundaryAccepted)
{
    float a = 30.0f * M_PI / 180.0f;
    float scale = 5.0f;
    EXPECT_TRUE(Custom_Math::voxelInCone_CenterOnly(
        makeVoxel(scale * std::sin(a), 0, scale * std::cos(a)), cone));
}


// ═══════════════════════════════════════════════════════════════════════════
//  4.  voxelInCone_BoundingSphere
// ═══════════════════════════════════════════════════════════════════════════

class VoxelInConeSphereTest : public ::testing::Test
{
protected:
    Cone cone = makeCone30();
};

TEST_F(VoxelInConeSphereTest, VoxelOnAxisAccepted)
{
    EXPECT_TRUE(Custom_Math::voxelInCone_BoundingSphere(makeVoxel(0, 0, 5), cone));
}

TEST_F(VoxelInConeSphereTest, VoxelFarOutsideRejected)
{
    // 90° away, far enough that even the circumsphere can't bridge the gap
    EXPECT_FALSE(Custom_Math::voxelInCone_BoundingSphere(makeVoxel(50, 0, 0), cone));
}

TEST_F(VoxelInConeSphereTest, NeverFalseNegativeForClearlyInsideVoxel)
{
    // A voxel solidly inside the cone must never be rejected
    EXPECT_TRUE(Custom_Math::voxelInCone_BoundingSphere(makeVoxel(0, 0, 10), cone));
}

TEST_F(VoxelInConeSphereTest, BoundaryVoxelIsConservative)
{
    // At the rim the sphere test may accept a voxel the center test rejects —
    // verify it doesn't REJECT what the center test accepts.
    Voxel v = makeVoxel(0, 0, 5);
    if (Custom_Math::voxelInCone_CenterOnly(v, cone))
        EXPECT_TRUE(Custom_Math::voxelInCone_BoundingSphere(v, cone));
}


// ═══════════════════════════════════════════════════════════════════════════
//  5.  voxelInCone_Corners
// ═══════════════════════════════════════════════════════════════════════════

class VoxelInConeCornersTest : public ::testing::Test
{
protected:
    Cone cone = makeCone30();
};

TEST_F(VoxelInConeCornersTest, VoxelClearlyInsideAccepted)
{
    EXPECT_TRUE(Custom_Math::voxelInCone_Corners(makeVoxel(0, 0, 5), cone));
}

TEST_F(VoxelInConeCornersTest, VoxelClearlyOutsideRejected)
{
    EXPECT_FALSE(Custom_Math::voxelInCone_Corners(makeVoxel(50, 0, 0), cone));
}

TEST_F(VoxelInConeCornersTest, CornerTestNeverRejectsWhatCenterTestAccepts)
{
    // If center is inside, at least one corner should typically also be inside
    // (may not hold for enormous voxels, but holds for unit voxels far from apex)
    Voxel v = makeVoxel(0, 0, 10);
    if (Custom_Math::voxelInCone_CenterOnly(v, cone))
        EXPECT_TRUE(Custom_Math::voxelInCone_Corners(v, cone));
}


// ═══════════════════════════════════════════════════════════════════════════
//  6.  voxelInCone_AxisClosestPoint
// ═══════════════════════════════════════════════════════════════════════════

class VoxelInConeAxisTest : public ::testing::Test
{
protected:
    Cone cone = makeCone30();
};

TEST_F(VoxelInConeAxisTest, VoxelStraddlingAxisAccepted)
{
    // Voxel centered just off-axis but straddling it
    EXPECT_TRUE(Custom_Math::voxelInCone_AxisClosestPoint(makeVoxel(0.1f, 0, 5), cone));
}

TEST_F(VoxelInConeAxisTest, VoxelFarFromAxisRejected)
{
    EXPECT_FALSE(Custom_Math::voxelInCone_AxisClosestPoint(makeVoxel(50, 0, 1), cone));
}


// ═══════════════════════════════════════════════════════════════════════════
//  7.  voxelInCone_Robust  (consistency suite)
// ═══════════════════════════════════════════════════════════════════════════

class VoxelInConeRobustTest : public ::testing::Test
{
protected:
    Cone cone = makeCone30();
};

TEST_F(VoxelInConeRobustTest, VoxelOnAxisAccepted)
{
    EXPECT_TRUE(Custom_Math::voxelInCone_Robust(makeVoxel(0, 0, 5), cone));
}

TEST_F(VoxelInConeRobustTest, VoxelFarOutsideRejected)
{
    EXPECT_FALSE(Custom_Math::voxelInCone_Robust(makeVoxel(100, 0, 0), cone));
}

TEST_F(VoxelInConeRobustTest, RobustNeverMoreRestrictiveThanBoundingSphere)
{
    // If bounding sphere says NO, robust must also say NO (sphere is used as
    // first-stage reject, so this tests the pipeline contract).
    Voxel v = makeVoxel(100, 0, 0);
    bool sphere = Custom_Math::voxelInCone_BoundingSphere(v, cone);
    bool robust = Custom_Math::voxelInCone_Robust(v, cone);

    if (!sphere) EXPECT_FALSE(robust);
}

TEST_F(VoxelInConeRobustTest, RobustAtLeastAsPermissiveAsCorners)
{
    // If corners says YES, robust must also say YES.
    std::vector<Voxel> samples = {
        makeVoxel(0, 0, 5), makeVoxel(1, 0, 5), makeVoxel(0, 1, 5)
    };
    for (const auto& v : samples) {
        if (Custom_Math::voxelInCone_Corners(v, cone))
            EXPECT_TRUE(Custom_Math::voxelInCone_Robust(v, cone))
                << "Robust rejected a voxel whose corner was inside the cone";
    }
}

TEST_F(VoxelInConeRobustTest, BackFacingConeRejectsForwardVoxel)
{
    // Cone pointing down (−Z): a voxel at +Z should be rejected
    Cone downCone(Vector3(0,0,0), Vector3(0,0,-1), 30.0f * M_PI / 180.0f);
    EXPECT_FALSE(Custom_Math::voxelInCone_Robust(makeVoxel(0, 0, 5), downCone));
}


// ═══════════════════════════════════════════════════════════════════════════
//  8.  beerLambert_VoxelAbsorption
// ═══════════════════════════════════════════════════════════════════════════

class BeerLambertTest : public ::testing::Test {};

TEST_F(BeerLambertTest, ZeroPathLengthYieldsZeroEnergy)
{
    float e = Custom_Math::beerLambert_VoxelAbsorption(1000.0f, 0.0f, 1.0f);
    EXPECT_NEAR(e, 0.0f, EPS);
}

TEST_F(BeerLambertTest, ZeroTimeYieldsZeroEnergy)
{
    float e = Custom_Math::beerLambert_VoxelAbsorption(1000.0f, 1.0f, 0.0f);
    EXPECT_NEAR(e, 0.0f, EPS);
}

TEST_F(BeerLambertTest, ZeroIrradianceYieldsZeroEnergy)
{
    float e = Custom_Math::beerLambert_VoxelAbsorption(0.0f, 1.0f, 1.0f);
    EXPECT_NEAR(e, 0.0f, EPS);
}

TEST_F(BeerLambertTest, AbsorbedEnergyIsPositive)
{
    float e = Custom_Math::beerLambert_VoxelAbsorption(1000.0f, 0.01f, 1.0f);
    EXPECT_GT(e, 0.0f);
}

TEST_F(BeerLambertTest, AbsorbedEnergyBoundedByIncoming)
{
    // Can never absorb more than 100 % of incoming power × area × time
    float I_in = 1000.0f, dt = 1.0f, path = 0.01f;
    float maxPossible = I_in * (Voxel_Size * Voxel_Size) * dt;
    float e = Custom_Math::beerLambert_VoxelAbsorption(I_in, path, dt);
    EXPECT_LE(e, maxPossible + EPS);
}

TEST_F(BeerLambertTest, LongerPathAbsorbsMoreEnergy)
{
    float e1 = Custom_Math::beerLambert_VoxelAbsorption(1000.0f, 0.01f, 1.0f);
    float e2 = Custom_Math::beerLambert_VoxelAbsorption(1000.0f, 0.10f, 1.0f);
    EXPECT_GT(e2, e1);
}

TEST_F(BeerLambertTest, LongerExposureScalesEnergyLinearly)
{
    float e1 = Custom_Math::beerLambert_VoxelAbsorption(1000.0f, 0.01f, 1.0f);
    float e2 = Custom_Math::beerLambert_VoxelAbsorption(1000.0f, 0.01f, 2.0f);
    EXPECT_NEAR(e2, 2.0f * e1, EPS);
}

TEST_F(BeerLambertTest, KnownValueCheck)
{
    // Manual: τ = mu * path = 0.1 * 0.01 = 0.001
    //         T = exp(-0.001) ≈ 0.9990005
    //         I_abs = 1000 * (1 - 0.9990005) ≈ 0.9995
    //         area  = Voxel_Size^2  (e.g. 0.001^2 m² if Voxel_Size=0.001)
    //         energy = I_abs * area * 1.0
    // Adjust expected value to match your Voxel_Size and mu constants.
    float tau      = mu * 0.01f;
    float I_abs    = 1000.0f * (1.0f - std::exp(-tau));
    float expected = I_abs * Voxel_Size * Voxel_Size * 1.0f;

    float actual = Custom_Math::beerLambert_VoxelAbsorption(1000.0f, 0.01f, 1.0f);
    EXPECT_NEAR(actual, expected, EPS);
}


// ═══════════════════════════════════════════════════════════════════════════
//  9.  voxelEnergyFromConeLight
// ═══════════════════════════════════════════════════════════════════════════

class VoxelEnergyTest : public ::testing::Test
{
protected:
    Cone  cone     = makeCone30();
    float radiance = 1000.0f;  // W/m²/sr
    float dt       = 1.0f;     // 1 second exposure
    // T_path removed – now derived internally from geometry
};

TEST_F(VoxelEnergyTest, VoxelOutsideConeReceivesZeroEnergy)
{
    float e = Custom_Math::voxelEnergyFromConeLight(
        makeVoxel(100, 0, 0), cone, radiance, dt);
    EXPECT_NEAR(e, 0.0f, EPS);
}

TEST_F(VoxelEnergyTest, VoxelInsideConeReceivesPositiveEnergy)
{
    float e = Custom_Math::voxelEnergyFromConeLight(
        makeVoxel(0, 0, 5), cone, radiance, dt);
    EXPECT_GT(e, 0.0f);
}

TEST_F(VoxelEnergyTest, ZeroRadianceYieldsZeroEnergy)
{
    float e = Custom_Math::voxelEnergyFromConeLight(
        makeVoxel(0, 0, 5), cone, 0.0f, dt);
    EXPECT_NEAR(e, 0.0f, EPS);
}

// Replaces ZeroTransmittanceYieldsZeroEnergy:
// A voxel sitting exactly on the cylinder margin has BC = 0,
// so T_path = exp(-μ·0) = 1.0 — maximum possible energy.
// A voxel deeper inside must receive strictly less.

TEST_F(VoxelEnergyTest, ZeroExposureTimeYieldsZeroEnergy)
{
    float e = Custom_Math::voxelEnergyFromConeLight(
        makeVoxel(0, 0, 5), cone, radiance, 0.0f);
    EXPECT_NEAR(e, 0.0f, EPS);
}

// Replaces OccludedVoxelReceivesLessEnergy (which relied on T_path as input):
// Transmittance is now geometric — a voxel deeper inside the cylinder
// accumulates more optical depth (larger BC) and must receive less energy.
TEST_F(VoxelEnergyTest, DeeperVoxelReceivesLessEnergy)
{
    float e_near = Custom_Math::voxelEnergyFromConeLight(
        makeVoxel(0, 0, 2), cone, radiance, dt);   // small BC
    float e_far  = Custom_Math::voxelEnergyFromConeLight(
        makeVoxel(0, 0, 8), cone, radiance, dt);   // large BC
    EXPECT_GT(e_near, e_far);
}

TEST_F(VoxelEnergyTest, EnergyScalesLinearlyWithTime)
{
    float e1 = Custom_Math::voxelEnergyFromConeLight(
        makeVoxel(0, 0, 5), cone, radiance, 1.0f);
    float e2 = Custom_Math::voxelEnergyFromConeLight(
        makeVoxel(0, 0, 5), cone, radiance, 3.0f);
    EXPECT_NEAR(e2, 3.0f * e1, EPS);
}


// ═══════════════════════════════════════════════════════════════════════════
//  10.  distance_voxel_to_margin
// ═══════════════════════════════════════════════════════════════════════════

class DistanceToMarginTest : public ::testing::Test
{
protected:
    // Origin at center, voxel on the +X axis
    Vector3 origin{0, 0, 0};
};

TEST_F(DistanceToMarginTest, VoxelAtOriginReturnsZero)
{
    // Coincident origin/voxel → dist = 0 → return 0
    float d = Custom_Math::distance_voxel_to_margin(origin, makeVoxel(0, 0, 0));
    EXPECT_NEAR(d, 0.0f, EPS);
}

TEST_F(DistanceToMarginTest, VoxelOnRadiusBoundaryReturnsNearZero)
{
    // Center exactly on the circle boundary
    float d = Custom_Math::distance_voxel_to_margin(
        origin, makeVoxel(Print_Radius, 0, 0));
    EXPECT_NEAR(d, 0.0f, EPS);
}

TEST_F(DistanceToMarginTest, VoxelInsideCircleReturnsPositiveDistance)
{
    float x = Print_Radius * 0.5f;  // halfway to the edge
    float d = Custom_Math::distance_voxel_to_margin(origin, makeVoxel(x, 0, 0));
    EXPECT_GT(d, 0.0f);
}

TEST_F(DistanceToMarginTest, DistanceApproximatelyHalfRadius)
{
    // Voxel at r/2 on the X-axis: remaining distance to boundary ≈ r/2
    float x        = Print_Radius * 0.5f;
    float expected = Print_Radius - x;  // = Print_Radius / 2
    float d        = Custom_Math::distance_voxel_to_margin(origin, makeVoxel(x, 0, 0));
    EXPECT_NEAR(d, expected, EPS);
}

TEST_F(DistanceToMarginTest, SymmetryInXandY)
{
    float r = Print_Radius * 0.5f;
    float dx = Custom_Math::distance_voxel_to_margin(origin, makeVoxel(r, 0, 0));
    float dy = Custom_Math::distance_voxel_to_margin(origin, makeVoxel(0, r, 0));

    // Both should hit the circle at the same remaining arc-length
    EXPECT_NEAR(dx, dy, EPS);
}

TEST_F(DistanceToMarginTest, OffCenterOrigin)
{
    // Non-trivial origin: ray from (1,0,0) toward (3,0,0)
    Vector3 orig(1, 0, 0);
    Voxel   v = makeVoxel(3, 0, 0);
    float   d = Custom_Math::distance_voxel_to_margin(orig, v);
    EXPECT_GE(d, 0.0f);   // must be non-negative
}

TEST_F(DistanceToMarginTest, NeverNegative)
{
    // Property: distance is always ≥ 0
    std::vector<std::pair<float,float>> pts = {
        {0,0}, {1,0}, {0,2}, {-1,1}, {2.5f, 2.5f}
    };
    for (auto [x, y] : pts) {
        float d = Custom_Math::distance_voxel_to_margin(origin, makeVoxel(x, y, 0));
        EXPECT_GE(d, 0.0f) << "Negative distance for voxel (" << x << "," << y << ")";
    }
}