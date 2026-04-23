#ifndef __CUSTOM_MATH_H__
#define __CUSTOM_MATH_H__

#include <vector>
using std::vector;
#include "./Geometry/Voxel.h"
#include "./Geometry/Ray.h"
#include "./Geometry/Cone.h"

class Custom_Math{
    public:
        bool Ray_Voxel_Intersect(Ray ray, Voxel v);
        void Beer_Lambert_Energy(Ray ray, Voxel v);
        vector<vector<Voxel>> Radius_to_Voxel_array();

        bool voxelInCone_Robust(const Voxel& voxel, const Cone& cone);
        bool voxelInCone_AxisClosestPoint(const Voxel& voxel, const Cone& cone);
        bool voxelInCone_Corners(const Voxel& voxel, const Cone& cone);
        bool voxelInCone_BoundingSphere(const Voxel& voxel, const Cone& cone);
        bool voxelInCone_CenterOnly(const Voxel& voxel, const Cone& cone);
        static bool pointInCone(const Vector3& point, const Cone& cone);
        float voxelEnergyFromConeLight(
            const Voxel& voxel,
            const Cone&  cone,
            float sourceRadiance,
            float mu,
            float T_path);
        float beerLambert_VoxelAbsorption(
            float  I_in,        // incoming irradiance (W/m²)
            float  mu,          // attenuation coefficient (1/m)
            float  pathLength,  // ray segment length inside voxel (m)
            float& I_out);
    private:
};

#endif