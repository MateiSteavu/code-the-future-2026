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
#include <thread>
#include <mutex>
#include <atomic>

using std::vector;

// Global voxel grid
vector<vector<Voxel>> Obj;
std::mutex obj_mutex;       // Protects Obj during merges
std::mutex csv_mutex;       // Protects CSV writes
std::atomic<int> angles_done{0};

Vector3 zAxis = Vector3(0, 0, 1);
Vector3 worldOrigin = Vector3(0,0,0);
Vector3 imgOrigin = Vector3(DIST_DMD_PV, -DMD_Y_NR*MD_DIM_Y/2, DMD_Z_NR*MD_DIM_Z/2);
Vector3 imgCenter = Vector3(DIST_DMD_PV, 0, Z_Offset_DMD);

Vector3 current_pixel = imgOrigin;

// Process a single angle, returns a local energy delta grid
vector<vector<float>> processAngle(int i, Custom_Math& cm) {
    // Local energy accumulator (same dimensions as Obj)
    vector<vector<float>> local_delta(Obj.size());
    for (int vi = 0; vi < (int)Obj.size(); vi++)
        local_delta[vi].assign(Obj[vi].size(), 0.0f);

    float theta = i * (PI / 180.0f);

    std::ostringstream oss;
    oss << "./projections/proj_angle_" << std::setfill('0') << std::setw(3) << i << ".txt";
    std::ifstream f(oss.str());

    if (!f.is_open()) {
        std::cerr << "Skipping missing file: " << oss.str() << "\n";
        return local_delta;
    }
    Vector3 v = imgCenter;
    v.Rot_Vector3(zAxis, theta);
    std::cout<<v.getX();
    float thingy;
    for (int img_x = 0; img_x < 128; img_x++) {
        if(img_x%20 == 0)
            std::cout<<"processed "<<img_x<< " lines for image "<<i<<'\n';
        for (int img_y = 0; img_y < 128; img_y++) {
            f >> thingy;

            if (thingy != 0) {
                Vector3 rotated_origin = imgCenter;
                rotated_origin.Rot_Vector3(zAxis, theta); // rotate origin around z axis

                Ray direction_ray(rotated_origin, Vector3(-rotated_origin.getX(), -rotated_origin.getY(), 0)); // generate ray dir
                Cone direction_cone(direction_ray, RAY_Angle);

                current_pixel.setY(imgOrigin.getY() + img_x * MD_DIM_Y);
                current_pixel.setZ(imgOrigin.getZ() - img_y * MD_DIM_Z);
                current_pixel.Rot_Vector3(zAxis, theta);

                Ray pixel_ray(current_pixel, Vector3(-rotated_origin.getX(), -rotated_origin.getY(), 0));
                Cone pixel_cone(pixel_ray, RAY_Angle);

                for (int vi = 0; vi < (int)Obj.size(); vi++) {
                    for (int vj = 0; vj < (int)Obj[vi].size(); vj++) {
                        if (cm.voxelInCone_CenterOnly(Obj[vi][vj], pixel_cone)) {
                            // Write to local delta only — no shared state touched
                            local_delta[vi][vj] += cm.voxelEnergyFromConeLight(
                                Obj[vi][vj], pixel_cone, thingy * P_t_L, Exp_t);
                        }
                    }
                }
            }
        }
    }

    f.close();
    return local_delta;
}

void mergeAndSaveCSV(const vector<vector<float>>& delta, std::ofstream& g) {
    std::lock_guard<std::mutex> lock(obj_mutex);
    for (int vi = 0; vi < (int)Obj.size(); vi++) {
        for (int vj = 0; vj < (int)Obj[vi].size(); vj++) {
            Obj[vi][vj].setEnergy(Obj[vi][vj].getEnergy() + delta[vi][vj]);
        }
    }

    // Rewrite CSV with updated state
    {
        std::lock_guard<std::mutex> csv_lock(csv_mutex);
        g.seekp(0);
        g.clear();
        for (int vi = 0; vi < (int)Obj.size(); vi++) {
            for (int vj = 0; vj < (int)Obj[vi].size(); vj++) {
                if (Obj[vi][vj].getEnergy() > Energy_for_solid_Voxel) {
                    g << Obj[vi][vj].getCenter().getX() << ','
                      << Obj[vi][vj].getCenter().getY() << ','
                      << Obj[vi][vj].getCenter().getZ() << '\n';
                }
            }
        }
        g.flush();
    }
}

void workerThread(vector<int>& angles, Custom_Math cm, std::ofstream& g) {
    for (int i : angles) {
        auto delta = processAngle(i, cm);
        mergeAndSaveCSV(delta, g);

        int done = ++angles_done;
        if (done % 10 == 0)
            std::cout << "Progress: " << done << "/" << 360 << " angles processed.\n";
    }
}

int main() {
    const int TOTAL_ANGLES = 360;
    const int NUM_THREADS = 4;
    Custom_Math cm;

    // 1. Initialize Voxel Grid
    Obj = cm.Radius_to_Voxel_array();
    if (Obj.empty() || Obj[0].empty()) {
        std::cerr << "Error: Voxel array failed to initialize.\n";
        return 1;
    }

    std::cout << "Grid Dimensions: " << Obj.size() << "x" << Obj[0].size() << std::endl;
    std::cout << "Starting processing on " << NUM_THREADS << " threads...\n";

    // 2. Split angles across threads
    vector<vector<int>> thread_angles(NUM_THREADS);
    for (int i = 0; i < TOTAL_ANGLES; i++)
        thread_angles[i % NUM_THREADS].push_back(i);

    // 3. Open CSV once
    std::ofstream g("./python/test.csv");

    // 4. Launch threads
    vector<std::thread> threads;
    for (int t = 0; t < NUM_THREADS; t++)
        threads.emplace_back(workerThread, std::ref(thread_angles[t]), cm, std::ref(g));

    for (auto& t : threads)
        t.join();

    g.close();
    std::cout << "Done! Final results saved to ./generated/test.csv" << std::endl;
    return 0;
}