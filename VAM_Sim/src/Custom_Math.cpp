#include "../inc/Custom_Math.h"
#include "../inc/Geometry/Ray.h"
#include "../inc/Geometry/Voxel.h"

#include <cmath>
#include <algorithm>
#include <array>

bool Custom_Math::Ray_Voxel_Intersect(Ray r, Voxel V){

}

struct Cone {
    Ray apex_dir;
    float halfAngle;    // θ in radians;  solid angle Ω = 2π(1 - cos θ)

    float cosHalfAngle() const { return std::cos(halfAngle); }

    // Solid angle in steradians subtended by this cone
    float solidAngle() const {
        return 2.0f * 3.14159265f * (1.0f - std::cos(halfAngle));
    }
};

static bool pointInCone(const Vector3& point, const Cone& cone)
{
    Vector3 toPoint = (point - cone.getOrigin()).normalized();
    float cosAngle = toPoint.dot(cone.getDirection()); // direction must be unit
    return cosAngle >= cone.cosHalfAngle();
}

// ============================================================
//  TEST 1 — Center-point test  (fastest, least accurate)
// ============================================================
//
//  Just asks: does the cone's axis point at the voxel center?
//  Misses voxels whose center is outside but whose body overlaps.
//
bool voxelInCone_CenterOnly(const Voxel& voxel, const Cone& cone)
{
    return pointInCone(voxel.getCenter(), cone);
}

// ============================================================
//  TEST 2 — Bounding-sphere test  (conservative / fast cull)
// ============================================================
//
//  Wraps the voxel in its circumsphere of radius r.
//  The sphere "appears" to have an angular radius α = arcsin(r/dist)
//  as seen from the cone apex.
//  Overlap condition: angle between axis and voxel center ≤ (θ + α).
//
//  Result: may produce false positives near the boundary,
//          but NEVER discards a voxel that truly overlaps.
//  Use it as a cheap early-accept / early-reject pass.
//
bool voxelInCone_BoundingSphere(const Voxel& voxel, const Cone& cone)
{
    Vec3  toCenter = voxel.center - cone.apex;
    float dist     = toCenter.length();

    if (dist < 1e-9f) return true; // apex is inside the voxel

    float r     = voxel.circumsphereRadius();
    float alpha = std::asin(std::clamp(r / dist, -1.0f, 1.0f)); // angular radius of sphere

    // Angle between cone axis and direction-to-voxel-center
    float cosAngle = toCenter.normalized().dot(cone.direction);
    float phi      = std::acos(std::clamp(cosAngle, -1.0f, 1.0f));

    // Overlap when the angular gap is within the combined angles
    return phi <= (cone.halfAngle + alpha);
}

// ============================================================
//  TEST 3 — Corner test  (good accuracy, cheap)
// ============================================================
//
//  If ANY of the 8 corners of the AABB is inside the cone,
//  the voxel overlaps the solid angle.
//
//  Caveat: can miss cases where the cone clips an edge/face
//  without touching any corner (rare for small voxels).
//
bool voxelInCone_Corners(const Voxel& voxel, const Cone& cone)
{
    for (const Vec3& corner : voxel.corners()) {
        if (pointInCone(corner, cone))
            return true;
    }
    return false;
}

// ============================================================
//  TEST 4 — Axis-closest-point test  (catches edge/face clips)
// ============================================================
//
//  Projects the cone axis (infinite ray from apex along direction)
//  onto the AABB. If the closest point on the AABB to that ray
//  is inside the cone, the voxel overlaps even if no corner does.
//
bool voxelInCone_AxisClosestPoint(const Voxel& voxel, const Cone& cone)
{
    // Project the voxel center onto the cone axis ray
    Vec3  toCenter = voxel.center - cone.apex;
    float t        = toCenter.dot(cone.direction); // signed distance along axis

    // Point on the cone axis closest to the voxel center
    Vec3 axisPoint = cone.apex + cone.direction * t;

    // Clamp that point to the AABB (closest point ON the AABB to the axis)
    Vec3 lo = voxel.center - voxel.half();
    Vec3 hi = voxel.center + voxel.half();
    Vec3 closestOnAABB = axisPoint.clamped(lo, hi);

    // Check if that closest AABB point is inside the cone
    return pointInCone(closestOnAABB, cone);
}

// ============================================================
//  TEST 5 — Full robust test  (combines all strategies)
// ============================================================
//
//  Pipeline:
//    1. Bounding-sphere: quick reject if the sphere doesn't intersect
//    2. Corner test:     quick accept if any corner is inside
//    3. Axis-closest:    catches remaining edge/face clips
//
bool voxelInCone_Robust(const Voxel& voxel, const Cone& cone)
{
    // --- Stage 1: cheap sphere reject ---
    // If the bounding sphere doesn't even touch the cone, bail out.
    if (!voxelInCone_BoundingSphere(voxel, cone))
        return false;

    // --- Stage 2: corner accept ---
    // Any corner inside → definite overlap.
    if (voxelInCone_Corners(voxel, cone))
        return true;

    // --- Stage 3: axis closest-point ---
    // Handles the cone clipping through a face/edge without hitting a corner.
    return voxelInCone_AxisClosestPoint(voxel, cone);
}

























// ============================================================
//  Beer-Lambert energy deposition for one voxel
// ============================================================
//
//  Given:
//    I_in          — irradiance arriving at this voxel (W/m²)
//                    already attenuated by all prior voxels along the ray
//    mu            — attenuation coefficient of this voxel (m⁻¹)
//                    mu = mu_absorption + mu_scattering
//    pathLength    — length of the ray segment inside the voxel (m)
//                    For a cube voxel on-axis: pathLength ≈ dim
//
//  Returns the energy absorbed by this voxel per unit area (W/m²).
//  I_out is written to the reference so you can pass it to the next voxel.
//
float beerLambert_VoxelAbsorption(
    float  I_in,        // incoming irradiance (W/m²)
    float  mu,          // attenuation coefficient (1/m)
    float  pathLength,  // ray segment length inside voxel (m)
    float& I_out)       // [out] irradiance exiting this voxel
{
    // Optical depth (dimensionless):  τ = μ · Δx
    float tau = mu * pathLength;

    // Transmittance: fraction of light that passes through
    float T = std::exp(-tau);

    // Irradiance that exits the voxel (passed to the next one)
    I_out = I_in * T;

    // Energy absorbed by this voxel = what came in minus what left
    float absorbed = I_in * (1.0f - T);

    return absorbed;
}

// ============================================================
//  Full energy received by a voxel from a cone light source
// ============================================================
//
//  Combines the solid-angle test with Beer-Lambert.
//
//  Parameters:
//    voxel          — the voxel to test
//    cone           — light cone (apex = light position)
//    sourceRadiance — L_i, radiance of the light source (W/m²/sr)
//    mu             — attenuation coefficient of the voxel (1/m)
//    T_path         — accumulated transmittance from light to this voxel
//                     (product of e^{-τ} for all voxels along the path so far)
//
//  Returns the irradiance deposited into the voxel (W/m²),
//  or 0 if the voxel is not illuminated by this cone.
//
float voxelEnergyFromConeLight(
    const Voxel& voxel,
    const Cone&  cone,
    float sourceRadiance,  // L_i  (W/m²/sr)
    float mu,              // attenuation coefficient of this voxel (1/m)
    float T_path)          // transmittance accumulated before this voxel
{
    // ---- Step 1: Is this voxel inside the cone? ----
    if (!voxelInCone_Robust(voxel, cone))
        return 0.0f;

    // ---- Step 2: Irradiance arriving at the voxel ----
    //
    //  Convert radiance → irradiance by integrating over the solid angle:
    //    E = L_i * Ω          (for a uniform source within the cone)
    //  Then apply the path transmittance (all the voxels before this one):
    //    I_in = E * T_path
    //
    float solidAngle = cone.solidAngle();           // Ω = 2π(1 - cos θ)
    float I_in       = sourceRadiance * solidAngle * T_path;

    // ---- Step 3: Beer-Lambert absorption inside this voxel ----
    //
    //  Use the voxel's side length as the path length (assumes ray is
    //  roughly axis-aligned; for oblique rays, adjust to the actual
    //  intersection segment length with the AABB).
    //
    float I_out_unused = 0.0f;
    float absorbed = beerLambert_VoxelAbsorption(I_in, mu, voxel.dim, I_out_unused);

    return absorbed;
}