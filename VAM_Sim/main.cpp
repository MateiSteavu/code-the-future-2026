// 
#include "./inc/Geometry/Cone.h"
#include "./inc/Geometry/Ray.h"
#include "./inc/Geometry/Vector3.h"
#include "./inc/Geometry/Voxel.h"
#include "./inc/Custom_Math.h"
#include "./inc/Print_Setup/Setup.h"

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <vector>

using std::vector;

// Global voxel grid
vector<vector<Voxel>> Obj;

Vector3 worldOrigin = Vector3(0,0,0);
Vector3 imgOrigin = Vector3(DIST_DMD_PV, -DMD_Y_NR*MD_DIM_Y, DMD_Z_NR*MD_DIM_Z);
Vector3 imgCenter = Vector3(DIST_DMD_PV, -DMD_Y_NR*MD_DIM_Y/2, DMD_Z_NR*MD_DIM_Z/2);

int main() {
    const int TOTAL_ANGLES = 360;
    Custom_Math cm;

    // 1. Initialize Voxel Grid
    Obj = cm.Radius_to_Voxel_array();
    if (Obj.empty() || Obj[0].empty()) {
        std::cerr << "Error: Voxel array failed to initialize.\n";
        return 1;
    }

    std::cout << "Grid Dimensions: " << Obj.size() << "x" << Obj[0].size() << std::endl;
    std::cout << "Starting processing..." << std::endl;

    // 2. Main Projection Loop (Angle by Angle)
    for (int i = 0; i < TOTAL_ANGLES; i++) {
        float theta = i * (PI / 180.0f);

        // Open projection file
        std::ostringstream oss;
        oss << "./projections/proj_angle_" << std::setfill('0') << std::setw(3) << i << ".txt";
        std::ifstream f(oss.str());

        if (!f.is_open()) {
            std::cerr << "Skipping missing file: " << oss.str() << "\n";
            continue;
        }

        // Process every pixel in the 128x128 image
        float thingy;
        for (int img_x = 0; img_x < 128; img_x++) {
            for (int img_y = 0; img_y < 128; img_y++) {
                f >> thingy;
                
                // Only process pixels that have light contribution
                if (thingy != 0) {
                    // Geometric Setup
                    Vector3 zAxis = Vector3(0, 0, 1);
                    Vector3 current_pixel = imgOrigin;
                    Vector3 rotated_origin = imgCenter;
                    rotated_origin.Rot_Vector3(zAxis, theta);
                    Ray direction_ray(rotated_origin, Vector3(-rotated_origin.getX(),-rotated_origin.getY(),0));
                    Cone direction_cone(direction_ray, RAY_Angle);
                    current_pixel.setY(imgOrigin.getY() + img_x * MD_DIM_Y);
                    current_pixel.setZ(imgOrigin.getZ() - img_y * MD_DIM_Z);
                    current_pixel.Rot_Vector3(zAxis, theta);
                    Ray pixel_ray(current_pixel, Vector3(-rotated_origin.getX(),-rotated_origin.getY(),0));
                    Cone pixel_cone(pixel_ray, RAY_Angle);
                    // Voxel Update Loop
                    for (int vi = 0; vi < (int)Obj.size(); vi++) {
                        for (int vj = 0; vj < (int)Obj[vi].size(); vj++) {
                            if (cm.voxelInCone_CenterOnly(Obj[vi][vj], pixel_cone)) {
                                // Add energy directly to the global object
                                float current_e = Obj[vi][vj].getEnergy();
                                float delta_e = cm.voxelEnergyFromConeLight(Obj[vi][vj], direction_cone, thingy * P_t_L, Exp_t);
                                Obj[vi][vj].setEnergy(current_e + delta_e);
                            }
                        }
                    }
                }
            }
        }
        f.close();
        
        // Progress update every 10 degrees
        if (i % 10 == 0) {
            std::cout << "Progress: " << i << "/" << TOTAL_ANGLES << " angles processed.\n";
        }
    }

    // 3. Save Final Results
    std::cout << "Processing complete. Saving to CSV...\n";
    std::ofstream g("./generated/test.csv");
    for (int vi = 0; vi < (int)Obj.size(); vi++) {
        for (int vj = 0; vj < (int)Obj[vi].size(); vj++) {
            if (Obj[vi][vj].getEnergy() > Energy_for_solid_Voxel) {
                g << Obj[vi][vj].getCenter().getX() << ','
                  << Obj[vi][vj].getCenter().getY() << ','
                  << Obj[vi][vj].getCenter().getZ() << '\n';
            }
        }
    }
    g.close();

    std::cout << "Done! Final results saved to ./generated/test.csv" << std::endl;
    return 0;
}