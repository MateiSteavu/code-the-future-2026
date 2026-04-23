#include "../../inc/Geometry/Vector3.h"
#include <cmath>
#include <stdexcept>

// ─── Constructors ─────────────────────────────────────────────────────────────

Vector3::Vector3() : x(0.0), y(0.0), z(0.0) {}

Vector3::Vector3(float a, float b, float c) : x(a), y(b), z(c) {}

// ─── Getters ──────────────────────────────────────────────────────────────────

float Vector3::getX() const { return x; }
float Vector3::getY() const { return y; }
float Vector3::getZ() const { return z; }

// ─── Setters ──────────────────────────────────────────────────────────────────

void Vector3::setX(float x) { this->x = x; }
void Vector3::setY(float y) { this->y = y; }
void Vector3::setZ(float z) { this->z = z; }

// ─── Operators ────────────────────────────────────────────────────────────────

Vector3& Vector3::operator=(const Vector3& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
        z = other.z;
    }
    return *this;
}

bool Vector3::operator==(const Vector3& other) const {
    return x == other.x && y == other.y && z == other.z;
}

bool Vector3::operator!=(const Vector3& other) {
    return !(*this == other);
}

Vector3 Vector3::operator*(float scalar) const {
    Vector3 result;
    result.x = x * scalar;
    result.y = y * scalar;
    result.z = z * scalar;
    return result;
}

Vector3 Vector3::operator/(float scalar) const {
    if (scalar == 0.0f)
        throw std::runtime_error("Vector3: division by zero");
    Vector3 result;
    result.x = x / scalar;
    result.y = y / scalar;
    result.z = z / scalar;
    return result;
}

Vector3 Vector3::operator+(const Vector3& other) const {
    Vector3 result;
    result.x = x + other.x;
    result.y = y + other.y;
    result.z = z + other.z;
    return result;
}

Vector3 Vector3::operator-(const Vector3& other) const {
    Vector3 result;
    result.x = x - other.x;
    result.y = y - other.y;
    result.z = z - other.z;
    return result;
}

// ─── Rotation Functions ───────────────────────────────────────────────────────

// Rotate around the X axis by theta radians
void Vector3::Rot_X(float theta) {
    double cosT = std::cos(theta);
    double sinT = std::sin(theta);
    double newY = y * cosT - z * sinT;
    double newZ = y * sinT + z * cosT;
    y = static_cast<float>(newY);
    z = static_cast<float>(newZ);
}

// Rotate around the Y axis by theta radians
void Vector3::Rot_Y(float theta) {
    double cosT = std::cos(theta);
    double sinT = std::sin(theta);
    double newX =  x * cosT + z * sinT;
    double newZ = -x * sinT + z * cosT;
    x = static_cast<float>(newX);
    z = static_cast<float>(newZ);
}

// Rotate around the Z axis by theta radians
void Vector3::Rot_Z(float theta) {
    double cosT = std::cos(theta);
    double sinT = std::sin(theta);
    double newX = x * cosT - y * sinT;
    double newY = x * sinT + y * cosT;
    x = static_cast<float>(newX);
    y = static_cast<float>(newY);
}

// ─── Math Functions ───────────────────────────────────────────────────────────

float Vector3::dot(const Vector3& other) const {
    return x * other.x + y * other.y + z * other.z;
}

Vector3 Vector3::cross(const Vector3& other) const {
    Vector3 result;
    result.x = y * other.z - z * other.y;
    result.y = z * other.x - x * other.z;
    result.z = x * other.y - y * other.x;
    return result;
}

float Vector3::length() const {
    return static_cast<float>(std::sqrt(x * x + y * y + z * z));
}

Vector3 Vector3::normalize() const {
    float len = length();
    if (len == 0.0f)
        throw std::runtime_error("Vector3: cannot normalize a zero-length vector");
    return *this / len;
}

float Vector3::distanceTo(const Vector3& other) const {
    return (*this - other).length();
}

static double clamp(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

Vector3 Vector3::clamped(const Vector3& lo, const Vector3& hi) const {
    float loX = (lo.x < hi.x) ? lo.x : hi.x;
    float hiX = (lo.x > hi.x) ? lo.x : hi.x;

    float loY = (lo.y < hi.y) ? lo.y : hi.y;
    float hiY = (lo.y > hi.y) ? lo.y : hi.y;

    float loZ = (lo.z < hi.z) ? lo.z : hi.z;
    double hiZ = (lo.z > hi.z) ? lo.z : hi.z;

    return {
        static_cast<float>(clamp(x, loX, hiX)),
        static_cast<float>(clamp(y, loY, hiY)),
        static_cast<float>(clamp(z, loZ, hiZ))
    };
}
