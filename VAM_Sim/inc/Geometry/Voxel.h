#ifndef VOXEL_H
#define VOXEL_H

#include "./Vector3.h"
#include "../Print_Setup/Setup.h"
#include <math.h>
#include <algorithm>

class Voxel {
private:
    Vector3 center;   // World-space center
    float energy;     // J 

public:
    // Constructors
    Voxel();
    Voxel(Vector3 center);

    // Getters
    Vector3 getCenter() const;
    float   getEnergy() const;

    // Setters
    void setCenter (const Vector3& c);
    void setEnergy (float energy);

    // Operators
    Voxel& operator= (const Voxel& other);
    bool   operator==(const Voxel& other) const;
    bool   operator!=(const Voxel& other);
    Voxel  operator* (float scalar)       const;        // Scale extents
    Voxel  operator/ (float scalar)       const;        // Shrink extents
    Voxel  operator+ (const Voxel& other) const;        // Merge (avg center, max extents)
    Voxel  operator- (const Voxel& other) const;        // Difference (avg center, min extents)

    // Rotation (rotates center around world origin)
    void Rot_X(float theta);
    void Rot_Y(float theta);
    void Rot_Z(float theta);

    // Math (operate on center vector)
    float   dot(const Voxel& other) const;
    Vector3 cross(const Voxel& other) const;
    float   length() const;                       // Distance of center from origin
    Vector3 normalize() const;                  // Unit vector toward center
    float   distanceTo(const Voxel& other) const; // Center-to-center distance

    // Voxel-specific
    Vector3 half() const;                       // Half-extent as Vector3 (X, Y, Z)
    std::array<Vector3, 8> corners() const;     // 8 AABB corners
    float circumsphereRadius() const;           // Worst-case center-to-corner distance
    float volume() const;                       // 8 * X * Y * Z
    bool contains(const Vector3& point) const;  // AABB point-in-box test
    bool intersects(const Voxel& other) const;  // AABB-AABB overlap test
};

#endif // VOXEL_H
