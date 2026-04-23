#include "../../inc/Geometry/Voxel.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>

// ─── Constructors ─────────────────────────────────────────────────────────────

Voxel::Voxel()
    : center(0.0f, 0.0f, 0.0f), X(0.5f), Y(0.5f), Z(0.5f), energy(0) {}

Voxel::Voxel(Vector3 center, float X, float Y, float Z)
    : center(center), X(X), Y(Y), Z(Z), energy(0) {}

Voxel::Voxel(Vector3 center, float width)
    : center(center), X(width), Y(width), Z(width), energy(0) {}

// ─── Getters ──────────────────────────────────────────────────────────────────

Vector3 Voxel::getCenter() const { return center; }
float   Voxel::getX()      const { return X; }
float   Voxel::getY()      const { return Y; }
float   Voxel::getZ()      const { return Z; }
float   Voxel::getEnergy() const { return energy; }

// ─── Setters ──────────────────────────────────────────────────────────────────

void Voxel::setCenter (const Vector3& c) { center = c; }
void Voxel::setX      (float x) { X = x; }
void Voxel::setY      (float y) { Y = y; }
void Voxel::setZ      (float z) { Z = z; }
void Voxel::setEnergy (float energy) { this->energy = energy; }

// ─── Operators ────────────────────────────────────────────────────────────────

Voxel& Voxel::operator=(const Voxel& other) {
    if (this != &other) {
        center = other.center;
        X = other.X;
        Y = other.Y;
        Z = other.Z;
    }
    return *this;
}

bool Voxel::operator==(const Voxel& other) const {
    return center == other.center &&
           X == other.X &&
           Y == other.Y &&
           Z == other.Z;
}

bool Voxel::operator!=(const Voxel& other) {
    return !(*this == other);
}

// Scale all extents uniformly
Voxel Voxel::operator*(float scalar) const {
    return Voxel(center, X * scalar, Y * scalar, Z * scalar);
}

// Shrink all extents uniformly
Voxel Voxel::operator/(float scalar) const {
    if (scalar == 0.0f)
        throw std::runtime_error("Voxel: division by zero");
    return Voxel(center, X / scalar, Y / scalar, Z / scalar);
}

// Merge: average the centers, take the max extents on each axis
Voxel Voxel::operator+(const Voxel& other) const {
    Vector3 avgCenter = (center + other.center) / 2.0f;
    return Voxel(avgCenter,
                 std::max(X, other.X),
                 std::max(Y, other.Y),
                 std::max(Z, other.Z));
}

// Difference: average the centers, take the min extents on each axis
Voxel Voxel::operator-(const Voxel& other) const {
    Vector3 avgCenter = (center + other.center) / 2.0f;
    return Voxel(avgCenter,
                 std::min(X, other.X),
                 std::min(Y, other.Y),
                 std::min(Z, other.Z));
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
Vector3 Voxel::half() const {
    return Vector3(X, Y, Z);
}

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
        Vector3(cx - X, cy - Y, cz - Z),
        Vector3(cx + X, cy - Y, cz - Z),
        Vector3(cx - X, cy + Y, cz - Z),
        Vector3(cx + X, cy + Y, cz - Z),
        Vector3(cx - X, cy - Y, cz + Z),
        Vector3(cx + X, cy - Y, cz + Z),
        Vector3(cx - X, cy + Y, cz + Z),
        Vector3(cx + X, cy + Y, cz + Z),
    }};
}

// Worst-case center-to-corner distance: sqrt(X^2 + Y^2 + Z^2)
float Voxel::circumsphereRadius() const {
    return std::sqrt(X * X + Y * Y + Z * Z);
}

// Volume of the AABB: (2X) * (2Y) * (2Z)
float Voxel::volume() const {
    return 8.0f * X * Y * Z;
}

// Returns true if the given point lies inside (or on the boundary of) the AABB
bool Voxel::contains(const Vector3& point) const {
    return std::abs(point.getX() - center.getX()) <= X &&
           std::abs(point.getY() - center.getY()) <= Y &&
           std::abs(point.getZ() - center.getZ()) <= Z;
}

// Returns true if this voxel's AABB overlaps another voxel's AABB on all 3 axes
bool Voxel::intersects(const Voxel& other) const {
    return std::abs(center.getX() - other.center.getX()) <= (X + other.X) &&
           std::abs(center.getY() - other.center.getY()) <= (Y + other.Y) &&
           std::abs(center.getZ() - other.center.getZ()) <= (Z + other.Z);
}
