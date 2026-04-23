#ifndef RAY_H
#define RAY_H

#include "./Vector3.h"

class Ray {
private:
    Vector3 origin;
    Vector3 direction;

public:
    // Constructors
    Ray();
    Ray(const Vector3& origin, const Vector3& direction);

    // Getters
    Vector3 getOrigin() const;
    Vector3 getDirection() const;

    // Setters
    void setOrigin(const Vector3& o);
    void setDirection(const Vector3& d);

    // Get point along the ray: P(t) = origin + t * direction
    Vector3 at(float t) const;
};

#endif
