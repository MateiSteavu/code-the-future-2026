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
using std::vector;

vector<vector<Voxel>> Obj;

void processAngles(int start, int end) {
    float thingy;

    for (int i = start; i < end; i++) {
        std::ostringstream oss;
        oss << "./projections/proj_angle_"
            << std::setfill('0') << std::setw(3) << i
            << ".txt";

        std::string filename = oss.str();
        std::ifstream f(filename);

        for (int img_x = 0; img_x < 128; img_x++) {
            for (int img_y = 0; img_y < 128; img_y++) {
                f >> thingy;
                if (thingy != 0) {
                    // your logic here
                }
            }
        }

        f.close();
        std::cout << "Thread [" << start << "-" << end-1 << "] processed angle " << i << '\n';
    }
}

int main() {
    const int NUM_THREADS = 4;
    const int TOTAL_ANGLES = 360;
    const int CHUNK = TOTAL_ANGLES / NUM_THREADS; // 90 angles per thread

    Custom_Math cm;
    Obj = cm.Radius_to_Voxel_array();

    std::vector<std::thread> threads;

    for (int t = 0; t < NUM_THREADS; t++) {
        int start = t * CHUNK;
        int end   = (t == NUM_THREADS - 1) ? TOTAL_ANGLES : start + CHUNK;
        threads.emplace_back(processAngles, start, end);
    }

    for (auto& thread : threads) {
        thread.join(); 
    }

    std::cout << "All angles processed.\n";
    return 0;
}