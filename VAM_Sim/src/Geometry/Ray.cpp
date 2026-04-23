#include "../../inc/Geometry/Ray.h"

// ─── Constructors ─────────────────────────────────────────────────────────────

Ray::Ray() : origin(), direction(0,0,1) {}

Ray::Ray(const Vector3& o, const Vector3& d)
    : origin(o), direction(d.normalize()) {}

// ─── Getters ──────────────────────────────────────────────────────────────────

Vector3 Ray::getOrigin()    const { return origin; }
Vector3 Ray::getDirection() const { return direction; }

// ─── Setters ──────────────────────────────────────────────────────────────────

void Ray::setOrigin(const Vector3& o) { origin = o; }

void Ray::setDirection(const Vector3& d) {
    direction = d.normalize();
}

// ─── Helper functions ─────────────────────────────────────────────────────────

// Point along ray
Vector3 Ray::at(float t) const {
    return origin + direction * t;
} 
