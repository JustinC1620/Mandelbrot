#pragma once
#include <cstdint>
#include <cstring>

class Real32 {
public:
    Real32();
    Real32(int n);
    Real32(float n);
    Real32(double n);
    Real32(const Real32& n);

    uint32_t* getReal() const;
    int getSize() const;
    float getFloat() const;

    bool operator==(const Real32& rhs) const;
    bool operator!=(const Real32& rhs) const;
    Real32& operator=(const Real32& rhs);
    

    Real32 operator+(Real32 const& rhs) const;
    Real32& operator+=(Real32 const& rhs);
    Real32& operator++();

    Real32 operator-() const;
    Real32 operator-(Real32 const& rhs) const;
    Real32& operator-=(Real32 const& rhs);
    Real32& operator--();

    Real32 operator*(Real32 const& rhs) const;
    Real32& operator*=(Real32 const& rhs);

    static void mull2(Real32& real_a, Real32& real_b);

private:
    static const int PRECISION = 3;
    static const int ARRAY_SIZE = PRECISION + 1;
    static const float BASE;
    static const uint32_t HALF_BASE = 2147483648u;

    uint32_t real[ARRAY_SIZE];

    static void assign(Real32& real_x, const Real32& real_y);
    void zero();
    void load(const float value);
    static void shift(Real32& real_x, int n);
    static void negate(Real32& real_x);
    static void add(const Real32& real_a, const Real32& real_b, Real32& real_r);
    static void mul(const Real32& real_a, const Real32& real_b, Real32& real_r);
};
