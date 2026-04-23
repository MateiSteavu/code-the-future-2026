#ifndef __CUSTOM_MATH_H__
#define __CUSTOM_MATH_H__

#include <vector>
using std::vector;
#include "./Geometry/Voxel.h"
#include "./Geometry/Ray.h"

class Custom_Math{
    public:
        bool Ray_Voxel_Intersect(Ray ray, Voxel v);
        void Beer_Lambert_Energy(Ray ray, Voxel v);
        vector<vector<Voxel>> Radius_to_Voxel_array();
    private:
};

#endif