#ifndef CONE_H
#define CONE_H

#include "./Ray.h"
#include <cmath>

class Cone {
private:
    Ray axis;        // internal representation
    float halfAngle; // radians

public:
    // Constructors
    Cone();
    Cone(const Vector3& origin, const Vector3& direction, float angle);
    Cone(const Ray& axis, float angle);

    // Getters (use origin/direction terminology)
    Vector3 getOrigin() const;
    Vector3 getDirection() const;
    Ray getAxis() const;
    float getHalfAngle() const;

    // Setters
    void setOrigin(const Vector3& o);
    void setDirection(const Vector3& d);
    void setAxis(const Ray& r);
    void setHalfAngle(float angle);

    // Helpers
    float cosHalfAngle() const;
    float solidAngle() const;

    // Tests
    bool containsPoint(const Vector3& point) const;
};

#endif
