#ifndef __CUSTOM_MATH_H__
#define __CUSTOM_MATH_H__

#include <vector>
using std::vector;
#include "./Geometry/Voxel.h"
#include "./Geometry/Ray.h"
#include "./Geometry/Cone.h"

class Custom_Math{
    public:
        static bool Ray_Voxel_Intersect(Ray ray, Voxel v);
        static void Beer_Lambert_Energy(Ray ray, Voxel v);
        vector<vector<Voxel>> Radius_to_Voxel_array();
        static float distance_voxel_to_margin(const Vector3& origin, const Voxel& voxel);

        static bool voxelInCone_Robust(const Voxel& voxel, const Cone& cone);
        static bool voxelInCone_AxisClosestPoint(const Voxel& voxel, const Cone& cone);
        static bool voxelInCone_Corners(const Voxel& voxel, const Cone& cone);
        static bool voxelInCone_BoundingSphere(const Voxel& voxel, const Cone& cone);
        static bool voxelInCone_CenterOnly(const Voxel& voxel, const Cone& cone);
        static bool pointInCone(const Vector3& point, const Cone& cone);
        static float voxelEnergyFromConeLight(
            const Voxel& voxel,
            const Cone&  cone,
            float sourceRadiance,                 
            float T_path,           
            float deltaTime);
        static float beerLambert_VoxelAbsorption(
            float  I_in,        // incoming irradiance (W/m²)
            float  pathLength,  // ray segment length inside voxel (m)
            float  deltaTime    // exposure duration (s)
        );
    private:
};

#endif
