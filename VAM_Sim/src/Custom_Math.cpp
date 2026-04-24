#include "../inc/Custom_Math.h"
#include "../inc/Geometry/Ray.h"
#include "../inc/Geometry/Voxel.h"
#include "../inc/Print_Setup/Setup.h"

#include <cmath>
#include <algorithm>
#include <array>

vector<vector<Voxel>> Custom_Math::Radius_to_Voxel_array()
{
    const float start  = -Print_Radius;
    const int   n      = static_cast<int>(std::ceil(2.0f * Print_Radius / Voxel_Size));
    const float r2     = Print_Radius * Print_Radius;
    const int   num_levels = Print_Height/Voxel_Size;

    vector<vector<Voxel>> levels;
 
    for (int z = 0; z < num_levels; ++z) {
        vector<Voxel> level_voxels;
        float cz = Voxel_Size * z;
        
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                float cx = start + Voxel_Size * (i + 0.5f);
                float cy = start + Voxel_Size * (j + 0.5f);
                
                if (cx * cx + cy * cy <= r2) {
                    level_voxels.push_back(Voxel(Vector3(cx, cy, cz)));
                }
            }
        }
        
        if (!level_voxels.empty()) {
            levels.push_back(level_voxels);
        }
    }
 
    return levels;
}

bool Custom_Math::pointInCone(const Vector3& point, const Cone& cone)
{
    Vector3 toPoint = (point - cone.getOrigin()).normalize();
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
bool Custom_Math::voxelInCone_CenterOnly(const Voxel& voxel, const Cone& cone)
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
bool Custom_Math::voxelInCone_BoundingSphere(const Voxel& voxel, const Cone& cone)
{
    Vector3  toCenter = voxel.getCenter() - cone.getOrigin();
    float dist     = toCenter.length();

    if (dist < 1e-9f) return true; // apex is inside the voxel

    float r     = voxel.circumsphereRadius();
    float alpha = std::asin(std::clamp(r / dist, -1.0f, 1.0f)); // angular radius of sphere

    // Angle between cone axis and direction-to-voxel-center
    float cosAngle = toCenter.normalize().dot(cone.getDirection());
    float phi      = std::acos(std::clamp(cosAngle, -1.0f, 1.0f));

    // Overlap when the angular gap is within the combined angles
    return phi <= (cone.getHalfAngle() + alpha);
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
bool Custom_Math::voxelInCone_Corners(const Voxel& voxel, const Cone& cone)
{
    for (const Vector3& corner : voxel.corners()) {
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
bool Custom_Math::voxelInCone_AxisClosestPoint(const Voxel& voxel, const Cone& cone)
{
    // Project the voxel center onto the cone axis ray
    Vector3  toCenter = voxel.getCenter() - cone.getOrigin();
    float t        = toCenter.dot(cone.getDirection()); // signed distance along axis

    // Point on the cone axis closest to the voxel center
    Vector3 axisPoint = cone.getOrigin() + cone.getDirection() * t;

    // Clamp that point to the AABB (closest point ON the AABB to the axis)
    Vector3 lo = voxel.getCenter() - Vector3(Voxel_Size/2,Voxel_Size/2,Voxel_Size/2);
    Vector3 hi = voxel.getCenter() + Vector3(Voxel_Size/2,Voxel_Size/2,Voxel_Size/2);
    Vector3 closestOnAABB = axisPoint.clamped(lo, hi);

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
bool Custom_Math::voxelInCone_Robust(const Voxel& voxel, const Cone& cone)
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
//  Beer-Lambert energy deposition for one voxel — in JOULES
// ============================================================
//
//  Added:
//    deltaTime   — exposure duration (seconds)
//
//  Energy (J) = Irradiance (W/m²) × face area (m²) × time (s)
//  Face area of the voxel = Voxel_Size²  (square cross-section)
//
float Custom_Math::beerLambert_VoxelAbsorption(
    float  I_in,        // incoming irradiance (W/m²)
    float  pathLength,  // ray segment length inside voxel (m)
    float  deltaTime    // exposure duration (s)
)
{
    // Optical depth:  τ = μ · Δx  (dimensionless)
    float tau = mu * pathLength;

    // Transmittance: fraction of irradiance that passes through
    float T = std::exp(-tau);

    // Absorbed irradiance (W/m²): what came in minus what left
    float I_absorbed = I_in * (1.0f - T);

    // Cross-sectional face area of the voxel (m²)
    float faceArea = Voxel_Size * Voxel_Size;

    // Convert to energy: Power (W) = Irradiance (W/m²) × Area (m²)
    //                    Energy (J) = Power (W) × Time (s)
    float energy_joules = I_absorbed * faceArea * deltaTime;

    return energy_joules;  // Joules
}


// ============================================================
//  Full energy received by a voxel from a cone light source — in JOULES
// ============================================================
//
//  Added:
//    deltaTime   — exposure duration (seconds)
//
float Custom_Math::voxelEnergyFromConeLight(
    const Voxel&    voxel,
    const Cone&     cone,
    float           sourceRadiance,   // L_i  (W/m²/sr)
    float           deltaTime)        // exposure duration (s)
    // T_path parameter removed – computed internally from geometry
{
    // ---- Step 1: Is this voxel inside the cone? ----
    if (!voxelInCone_Robust(voxel, cone))
        return 0.0f;

    // ---- Step 2: Path transmittance from the cylinder margin to the voxel ----
    //
    //  distance_voxel_to_margin returns BC (m):
    //    the distance from the cylinder boundary to the voxel centre,
    //    measured along the ray from the cone origin.
    //
    //  Beer-Lambert transmittance over that path:
    //    T_path = exp( -μ · BC )
    //
    //  μ (mu) is the linear attenuation coefficient of the medium (1/m).
    //
    float BC      = distance_voxel_to_margin(cone.getOrigin(), voxel);
    float T_path  = std::exp(-mu * BC);

    // ---- Step 3: Irradiance arriving at the voxel (W/m²) ----
    //
    //  Radiance → Irradiance: E = L_i × Ω
    //  Scale by path transmittance (attenuation through the medium): × T_path
    //
    float solidAngle = cone.solidAngle();           // Ω = 2π(1 − cos θ)
    float I_in       = sourceRadiance * solidAngle * T_path;

    // ---- Step 4: Beer-Lambert → absorbed energy in Joules ----
    //
    //  beerLambert_VoxelAbsorption accounts for attenuation
    //  across the voxel itself (one Voxel_Size step).
    //
    float energy_joules = beerLambert_VoxelAbsorption(
        I_in,
        Voxel_Size,   // path length through the voxel (m)
        deltaTime     // exposure duration (s)
    );

    return energy_joules;  // Joules
}

float Custom_Math::distance_voxel_to_margin(const Vector3& origin, const Voxel& voxel)
{
    Vector3 dir = voxel.getCenter() - origin;

    float dist = dir.length();
    if (dist <= 1e-6f)
        return 0.0f;

    dir = dir / dist;
    const float R = static_cast<float>(Print_Radius);

    Vector3 p = origin;

    float dx = dir.getX();
    float dy = dir.getY();
    float ox = origin.getX();
    float oy = origin.getY();

    float a = dx*dx + dy*dy;
    float b = 2.0f * (ox*dx + oy*dy);
    float c = ox*ox + oy*oy - R*R;

    float disc = b*b - 4*a*c;

    if (disc < 0.0f)
        return 0.0f;

    float t1 = (-b - std::sqrt(disc)) / (2.0f * a);

    if (t1 < 0.0f)
        return 0.0f;

    float AB = t1;

    float AC = dist;

    float BC = AC - AB;

    return (BC > 0.0f) ? BC : 0.0f;
}



Vector3 Custom_Math::pixelToWorld(int img_x, int img_y)
{
    float halfZ = DMD_Z_NR * MD_DIM_Z * 0.5f;
    float halfY = DMD_Y_NR * MD_DIM_Y * 0.5f;

    float px = -halfZ + img_x * MD_DIM_Z + MD_DIM_Z * 0.5f;
    float py = -halfY + img_y * MD_DIM_Y + MD_DIM_Y * 0.5f;
    float pz = 0.0f;

    return Vector3(px, py, pz);
}

Vector3 Custom_Math::computeRayDirection(const Vector3& pixelPos)
{
    Vector3 print_center(0.0f, 0.0f, DIST_DMD_PV);
    return (print_center - pixelPos).normalize();
}

void Custom_Math::rotateRay(Vector3& origin, Vector3& direction, float angleDeg)
{
    float theta = angleDeg * (PI / 180.0f);

    origin.Rot_Z(theta);
    direction.Rot_Z(theta);
}

Vector3 Custom_Math::pixelBaseX(int img_x)
{
    float halfZ = DMD_Z_NR * MD_DIM_Z * 0.5f;
    float px = -halfZ + img_x * MD_DIM_Z + MD_DIM_Z * 0.5f;

    return Vector3(px, 0.0f, 0.0f);
}

float Custom_Math::pixelOffsetY(int img_y)
{
    float halfY = DMD_Y_NR * MD_DIM_Y * 0.5f;
    return -halfY + img_y * MD_DIM_Y + MD_DIM_Y * 0.5f;
}
