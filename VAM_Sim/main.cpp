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
#include <thread>
#include <vector>
#include <mutex>

using std::vector;

// Shared global voxel grid and its mutex
vector<vector<Voxel>> Obj;
std::mutex obj_mutex;

Vector3 worldOrigin = Vector3(0,0,0);
// image corner relative to world center
Vector3 imgOrigin = Vector3(DIST_DMD_PV, -DMD_Y_NR*MD_DIM_Y  , DMD_Z_NR*MD_DIM_Z);
Vector3 imgCenter = Vector3(DIST_DMD_PV, -DMD_Y_NR*MD_DIM_Y/2, DMD_Z_NR*MD_DIM_Z/2);

void processAngles(int start, int end) {
    float thingy;

    // Each thread works on its own local copy — no locking needed during computation
    vector<vector<float>> local_energy(Obj.size(), vector<float>(Obj[0].size(), 0.0f));

    Custom_Math cm;
    for (int i = start; i < end; i++) {
        float theta = i*(PI/180); // unghi in rad

        std::ostringstream oss;
        oss << "./projections/proj_angle_"
            << std::setfill('0') << std::setw(3) << i
            << ".txt";

        std::string filename = oss.str();
        std::ifstream f(filename);

        if (!f.is_open()) {
            std::cerr << "ERROR: Could not open file: " << filename << '\n';
            continue;
        }

        for (int img_x = 0; img_x < 128; img_x++) {

            for (int img_y = 0; img_y < 128; img_y++) {
                f >> thingy;

                if (thingy != 0) {

                    Vector3 current_pixel  = imgOrigin;
                    Vector3 rotated_origin = imgCenter;
                    rotated_origin.Rot_Vector3(worldOrigin, theta);

                    Ray  direction_ray  (rotated_origin, worldOrigin);
                    Cone direction_cone (direction_ray,  RAY_Angle);

                    current_pixel.setY(imgOrigin.getY()+img_x*MD_DIM_Y);
                    current_pixel.setZ(imgOrigin.getZ()-img_y*MD_DIM_Z);
                    current_pixel.Rot_Vector3(worldOrigin, theta);

                    Ray  pixel_ray      (current_pixel,  worldOrigin);
                    Cone pixel_cone     (pixel_ray,      RAY_Angle);

                    for (int vi = 0; vi < (int)Obj.size(); vi++) {
                        for (int vj = 0; vj < (int)Obj[vi].size(); vj++) {
                            
                            // @TODO: check if ray intersects this voxel
                            // @TODO: compute actual backprojection contribution

                            // Accumulate energy into the local copy
                            if(cm.voxelInCone_CenterOnly(Obj[vi][vj], pixel_cone)){
                                local_energy[vi][vj] = cm.voxelEnergyFromConeLight(Obj[vi][vj], direction_cone, thingy * P_t_L, Exp_t);
                            }
                        }
                    }
                }
            }
        }

        f.close();
        std::cout << "Thread [" << start << "-" << (end - 1) << "] processed angle " << i << '\n';
    }

    // Merge local result into global Obj — lock once per thread, not inside the loop
    {
        std::lock_guard<std::mutex> lock(obj_mutex);
        std::ofstream g("./generated/test.csv");
        for (int vi = 0; vi < (int)Obj.size(); vi++) {
            for (int vj = 0; vj < (int)Obj[vi].size(); vj++) {
                float merged = Obj[vi][vj].getEnergy() + local_energy[vi][vj];
                Obj[vi][vj].setEnergy(merged);
                if(Obj[vi][vj].getEnergy()>Energy_for_solid_Voxel)
                    g   << Obj[vi][vj].getCenter().getX() << ','
                        << Obj[vi][vj].getCenter().getY() << ','
                        << Obj[vi][vj].getCenter().getZ() << '\n';
            }
        }
        g.close();
    }
}

int main() {
    const int NUM_THREADS   = 4;
    const int TOTAL_ANGLES  = 360;
    const int CHUNK         = TOTAL_ANGLES / NUM_THREADS; // 90 angles per thread

    Custom_Math cm;
    Obj = cm.Radius_to_Voxel_array();

    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    for (int t = 0; t < NUM_THREADS; t++) {
        int start = t * CHUNK;
        int end   = (t == NUM_THREADS - 1) ? TOTAL_ANGLES : start + CHUNK;
        threads.emplace_back(processAngles, start, end, std::cref(Obj));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "All angles processed.\n";

    // @TODO: save or use Obj here

    return 0;
}
