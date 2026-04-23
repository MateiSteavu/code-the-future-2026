#include "../inc/Geometry/Cone.h"
#include "../inc/Geometry/Ray.h"
#include "../inc/Geometry/Vector3.h"
#include "../inc/Geometry/Voxel.h"
#include "../inc/Custom_Math.h"

int main()
{
    // Define a cone light source pointing down the +Z axis
    Cone cone;
    cone.apex      = {0.0f, 0.0f, 0.0f};
    cone.direction = {0.0f, 0.0f, 1.0f};  // must be unit vector
    cone.halfAngle = 0.2618f;              // ~15 degrees in radians

    // A voxel 5 units ahead, centered on the axis
    Voxel voxel;
    voxel.center = {0.0f, 0.0f, 5.0f};
    voxel.dim    = 0.5f;                   // 0.5m cube

    // Light and medium properties
    float sourceRadiance = 100.0f;  // W/m²/sr
    float mu             = 0.1f;    // attenuation coefficient (1/m)
    float T_path         = 1.0f;    // no occlusion before this voxel

    // Run the robust cone test
    bool inCone = voxelInCone_Robust(voxel, cone);
    printf("Voxel in cone: %s\n", inCone ? "YES" : "NO");

    // Compute absorbed energy
    float energy = voxelEnergyFromConeLight(voxel, cone, sourceRadiance, mu, T_path);
    printf("Absorbed irradiance: %.4f W/m²\n", energy);

    // Show the transmittance that would be passed to the NEXT voxel
    float I_in = sourceRadiance * cone.solidAngle() * T_path;
    float tau  = mu * voxel.dim;
    float T_next = std::exp(-tau);
    printf("Transmittance through this voxel (T_next): %.4f\n", T_next);
    printf("Irradiance passed to next voxel: %.4f W/m²\n", I_in * T_next);

    return 0;
}