#include "../../inc/Geometry/Voxel.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>

// ─── Constructors ─────────────────────────────────────────────────────────────

Voxel::Voxel()
    : center(0.0f, 0.0f, 0.0f), energy(0) {}

Voxel::Voxel(Vector3 center)
    : center(center), energy(0) {}

// ─── Getters ──────────────────────────────────────────────────────────────────

Vector3 Voxel::getCenter() const { return center; }
float   Voxel::getEnergy() const { return energy; }

// ─── Setters ──────────────────────────────────────────────────────────────────

void Voxel::setCenter (const Vector3& c) { center = c; }
void Voxel::setEnergy (float energy) { this->energy = energy; }

// ─── Operators ────────────────────────────────────────────────────────────────

Voxel& Voxel::operator=(const Voxel& other) {
    if (this != &other) {
        center = other.center;
    }
    return *this;
}

bool Voxel::operator==(const Voxel& other) const {
    return center == other.center;
}

bool Voxel::operator!=(const Voxel& other) {
    return !(*this == other);
}

// Scale all extents uniformly
Voxel Voxel::operator*(float scalar) const {
    return Voxel(center * scalar);
}

// Shrink all extents uniformly
Voxel Voxel::operator/(float scalar) const {
    if (scalar == 0.0f)
        throw std::runtime_error("Voxel: division by zero");
    return Voxel(center);
}

// Merge: average the centers, take the max extents on each axis
Voxel Voxel::operator+(const Voxel& other) const {
    Vector3 avgCenter = (center + other.center) / 2.0f;
    return Voxel(avgCenter);
}

// Difference: average the centers, take the min extents on each axis
Voxel Voxel::operator-(const Voxel& other) const {
    Vector3 avgCenter = (center - other.center) / 2.0f;
    return Voxel(avgCenter);
}

// ─── Rotation (rotates the center point around the world origin) ──────────────

void Voxel::Rot_X(float theta) { center.Rot_X(theta); }
void Voxel::Rot_Y(float theta) { center.Rot_Y(theta); }
void Voxel::Rot_Z(float theta) { center.Rot_Z(theta); }

// ─── Math (operate on the center vector) ─────────────────────────────────────

float Voxel::dot(const Voxel& other) const {
    return center.dot(other.center);
}

Vector3 Voxel::cross(const Voxel& other) const {
    return center.cross(other.center);
}

// Distance of the center from the world origin
float Voxel::length() const {
    return center.length();
}

// Unit vector pointing from origin toward the center
Vector3 Voxel::normalize() const {
    return center.normalize();
}

// Center-to-center Euclidean distance
float Voxel::distanceTo(const Voxel& other) const {
    return center.distanceTo(other.center);
}

// ─── Voxel-Specific ───────────────────────────────────────────────────────────

// Half-extent as a Vector3 — same layout as Voxel struct's half()

// The 8 AABB corners, enumerated bottom-then-top, back-to-front:
//   index  offset          label
//     0    (-X, -Y, -Z)   bottom-left-back
//     1    (+X, -Y, -Z)   bottom-right-back
//     2    (-X, +Y, -Z)   top-left-back
//     3    (+X, +Y, -Z)   top-right-back
//     4    (-X, -Y, +Z)   bottom-left-front
//     5    (+X, -Y, +Z)   bottom-right-front
//     6    (-X, +Y, +Z)   top-left-front
//     7    (+X, +Y, +Z)   top-right-front
std::array<Vector3, 8> Voxel::corners() const {
    float cx = center.getX();
    float cy = center.getY();
    float cz = center.getZ();
    return {{
        Vector3(cx - Voxel_Size/2, cy - Voxel_Size/2, cz - Voxel_Size/2),
        Vector3(cx + Voxel_Size/2, cy - Voxel_Size/2, cz - Voxel_Size/2),
        Vector3(cx - Voxel_Size/2, cy + Voxel_Size/2, cz - Voxel_Size/2),
        Vector3(cx + Voxel_Size/2, cy + Voxel_Size/2, cz - Voxel_Size/2),
        Vector3(cx - Voxel_Size/2, cy - Voxel_Size/2, cz + Voxel_Size/2),
        Vector3(cx + Voxel_Size/2, cy - Voxel_Size/2, cz + Voxel_Size/2),
        Vector3(cx - Voxel_Size/2, cy + Voxel_Size/2, cz + Voxel_Size/2),
        Vector3(cx + Voxel_Size/2, cy + Voxel_Size/2, cz + Voxel_Size/2),
    }};
}

// Worst-case center-to-corner distance: sqrt(X^2 + Y^2 + Z^2)
float Voxel::circumsphereRadius() const {
    return std::sqrt(3*Voxel_Size*Voxel_Size);
}

// Volume of the AABB: (2X) * (2Y) * (2Z)
float Voxel::volume() const {
    return Voxel_Size * Voxel_Size * Voxel_Size;
}

// Returns true if the given point lies inside (or on the boundary of) the AABB
bool Voxel::contains(const Vector3& point) const {
    return std::abs(point.getX() - center.getX()) <= Voxel_Size &&
           std::abs(point.getY() - center.getY()) <= Voxel_Size &&
           std::abs(point.getZ() - center.getZ()) <= Voxel_Size;
}

// Returns true if this voxel's AABB overlaps another voxel's AABB on all 3 axes
bool Voxel::intersects(const Voxel& other) const {
    return std::abs(center.getX() - other.center.getX()) <= (Voxel_Size*2) &&
           std::abs(center.getY() - other.center.getY()) <= (Voxel_Size*2) &&
           std::abs(center.getZ() - other.center.getZ()) <= (Voxel_Size*2);
}
