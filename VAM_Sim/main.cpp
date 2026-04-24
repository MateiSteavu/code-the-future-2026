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


void processAngles(int start, int end, const vector<vector<Voxel>>& obj) {
    float thingy;

    // Each thread works on its own local copy — no locking needed during computation
    vector<vector<Voxel>> local_obj = obj;

    for (int i = start; i < end; i++) {

        float theta = i * (PI / 180.0f);

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
            Vector3 base = Custom_Math::pixelBaseX(img_x);
            base.Rot_Z(theta);

            for (int img_y = 0; img_y < 128; img_y++) {
                f >> thingy;

                if (thingy != 0) {
                    
                    float py = Custom_Math::pixelOffsetY(img_y);
                    Vector3 origin = base + Vector3(0.0f, py, 0.0f);
                    origin.Rot_Z(theta);
                    Vector3 dir = (Vector3(0,0,DIST_DMD_PV) - origin).normalize();
                    Cone cone(origin, dir, RAY_Angle);

                    for (int vi = 0; vi < (int)local_obj.size(); vi++) {
                        for (int vj = 0; vj < (int)local_obj[vi].size(); vj++) {
                            

                            // @TODO: check if ray intersects this voxel
                            // @TODO: compute actual backprojection contribution

                            // Accumulate energy into the local copy
                            float current = local_obj[vi][vj].getEnergy();
                            local_obj[vi][vj].setEnergy(current + thingy);
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
        for (int vi = 0; vi < (int)Obj.size(); vi++) {
            for (int vj = 0; vj < (int)Obj[vi].size(); vj++) {
                float merged = Obj[vi][vj].getEnergy() + local_obj[vi][vj].getEnergy();
                Obj[vi][vj].setEnergy(merged);
            }
        }
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
