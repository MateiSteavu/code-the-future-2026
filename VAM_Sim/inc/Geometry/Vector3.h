#ifndef VECTOR3_H
#define VECTOR3_H

#include <array>

class Vector3 {
protected:
    float x, y, z;

public:
    // Constructors
    Vector3();
    Vector3(float a, float b, float c);

    // Getters
    float getX() const;
    float getY() const;
    float getZ() const;

    // Setters
    void setX(float x);
    void setY(float y); 
    void setZ(float z);

    // Operators
    Vector3& operator= (const Vector3& other) ;
    bool     operator==(const Vector3& other) const;
    bool     operator!=(const Vector3& other) ;
    Vector3  operator* (float scalar)         const;
    Vector3  operator/ (float scalar)         const;
    Vector3  operator+ (const Vector3& other) const;
    Vector3  operator- (const Vector3& other) const;

    // Rotation
    void Rot_X(float theta);
    void Rot_Y(float theta);
    void Rot_Z(float theta);

    // Math
    float   dot   (const Vector3& other) const;
    Vector3 cross (const Vector3& other) const;
    float   length()    const;
    Vector3 normalize() const;
    float   distanceTo  (const Vector3& other) const;
    Vector3 clamped     (const Vector3& lo, const Vector3& hi) const;
};

#endif // VECTOR3_H
