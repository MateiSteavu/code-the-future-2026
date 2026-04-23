#include "../../inc/Geometry/Cone.h"

// ─── Constructors ─────────────────────────────────────────────────────────────

Cone::Cone()
    : axis(Ray(Vector3(0,0,0), Vector3(0,0,1))), halfAngle(0.5f) {}

Cone::Cone(const Vector3& origin, const Vector3& direction, float angle)
    : axis(Ray(origin, direction)), halfAngle(angle) {}

Cone::Cone(const Ray& r, float angle)
    : axis(r), halfAngle(angle) {}

// ─── Getters ──────────────────────────────────────────────────────────────────

Vector3 Cone::getOrigin() const {
    return axis.getOrigin();
}

Vector3 Cone::getDirection() const {
    return axis.getDirection();
}

Ray Cone::getAxis() const {
    return axis;
}

float Cone::getHalfAngle() const {
    return halfAngle;
}

// ─── Setters ──────────────────────────────────────────────────────────────────

void Cone::setOrigin(const Vector3& o) {
    axis.setOrigin(o);
}

void Cone::setDirection(const Vector3& d) {
    axis.setDirection(d); // keeps normalization
}

void Cone::setAxis(const Ray& r) {
    axis = r;
}

void Cone::setHalfAngle(float angle) {
    halfAngle = angle;
}

// ─── Helpers ──────────────────────────────────────────────────────────────────

float Cone::cosHalfAngle() const {
    return std::cos(halfAngle);
}

float Cone::solidAngle() const {
    return 2.0f * 3.14159265f * (1.0f - std::cos(halfAngle));
}

// Point-in-cone test
bool Cone::containsPoint(const Vector3& point) const {
    Vector3 origin = axis.getOrigin();
    Vector3 dir    = axis.getDirection();

    Vector3 toPoint = (point - origin).normalize();
    float cosAngle  = toPoint.dot(dir);

    return cosAngle >= cosHalfAngle();
}
