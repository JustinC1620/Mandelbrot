#include <Real32.h>
#include <iostream>

const float Real32::BASE = 4294967296.0;

Real32::Real32() : real{} { zero(); }

Real32::Real32(int n) : Real32() { load(static_cast<float>(n)); }

Real32::Real32(float n) : Real32() { load(n); }

Real32::Real32(double n) : Real32() { load(static_cast<float>(n)); }

Real32::Real32(const Real32& real_n) : Real32() { assign(*this, real_n); }

uint32_t* Real32::getReal() const { 
    uint32_t* copy = new uint32_t[ARRAY_SIZE];
    std::memcpy(copy, real, ARRAY_SIZE * sizeof(uint32_t));
    return copy;
}

int Real32::getSize() const { return ARRAY_SIZE; }

float Real32::getFloat() const
{
    float num = static_cast<float>(real[1]) + static_cast<float>(real[2]) / BASE;
    return num * (real[0] == 0u ? 1 : -1);
}


bool Real32::operator==(const Real32& rhs) const
{
    for (int i = 0; i < ARRAY_SIZE; i++)
        if (this->real[i] != rhs.real[i])
            return false;
    return true;
}

bool Real32::operator!=(const Real32& rhs) const
{
    return !(*this == rhs);
}

Real32& Real32::operator=(const Real32& rhs) {
    if (*this != rhs) { assign(*this, rhs); }
    return *this;
}

Real32 Real32::operator+(Real32 const& rhs) const 
{
    Real32 new_real;
    add(*this, rhs, new_real);
    return new_real;
}

Real32& Real32::operator+=(Real32 const& rhs)
{
    add(*this, rhs, *this);
    return *this;
}

Real32& Real32::operator++() {
    *this += Real32(1);
    return *this;
}

Real32 Real32::operator-() const
{
    Real32 new_real = *this;
    negate(new_real);
    return new_real;
}

Real32 Real32::operator-(Real32 const& rhs) const
{
    Real32 new_real = *this + (-rhs);
    return new_real;
}

Real32& Real32::operator-=(Real32 const& rhs)
{
    *this += -rhs;
    return *this;
}

Real32& Real32::operator--() {
    *this += Real32(-1);
    return *this;
}

Real32 Real32::operator*(Real32 const& rhs) const
{
    Real32 new_real = Real32();
    mul(*this, rhs, new_real);
    return new_real;
}

Real32& Real32::operator*=(Real32 const& rhs)
{
    mul(*this, rhs, *this);
    return *this;
}


void Real32::assign(Real32& real_x, const Real32& real_y)
{
    for (int i = 0; i < ARRAY_SIZE; i++)
        real_x.real[i] = real_y.real[i];
}

void Real32::zero() 
{
    for (int i = 0; i < ARRAY_SIZE; i++)
        real[i] = 0u;
}

void Real32::load(const float value) 
{
    float load_value = value;

    if (load_value < 0.0) {
        real[0] = 1u;
        load_value *= -1.0;
    }
    else {
        real[0] = 0u;
    }
    for (int i = 1; i < ARRAY_SIZE; i++) {
        real[i] = static_cast<uint32_t>(load_value);
        load_value -= static_cast<float>(real[i]);
        load_value *= BASE;
    }
}

void Real32::shift(Real32& real_x, int n)
{
    for (int i = n + 1; i < ARRAY_SIZE; i++)
        real_x.real[i] = real_x.real[i - n];

    for (int i = 1; i <= n; i++)
        real_x.real[i] = 0u;
}

void Real32::negate(Real32& real_x)
{
    real_x.real[0] = (real_x.real[0] == 0u) ? 1u : 0u;
}

void Real32::add(const Real32& real_a, const Real32& real_b, Real32& real_r)
{
    const uint32_t* a = real_a.real;
    const uint32_t* b = real_b.real;

    Real32 temp = Real32();
    uint32_t* add_buffer = temp.real;

    bool add_pa = a[0] == 0u;
    bool add_pb = b[0] == 0u;

    if (add_pa == add_pb)
    {
        uint64_t add_carry = 0ULL;

        for (int add_i = PRECISION; add_i > 0; add_i--)
        {
            uint64_t add_temp = uint64_t(a[add_i]) + uint64_t(b[add_i]) + add_carry;
            add_buffer[add_i] = uint32_t(add_temp & 0xFFFFFFFFULL);
            add_carry = add_temp >> 32;
        }
        add_buffer[0] = add_pa ? 0u : 1u;
    }
    else
    {
        bool add_flip = false;

        for (int add_i = 1; add_i < ARRAY_SIZE; add_i++)
        {
            if (a[add_i] != b[add_i])
            {
                add_flip = (a[add_i] < b[add_i]);
                break;
            }
        }

        uint64_t add_borrow = 0ULL;
        if (add_flip)
        {
            for (int add_i = PRECISION; add_i > 0; add_i--)
            {
                uint64_t add_temp = uint64_t(b[add_i]) - uint64_t(a[add_i]) - add_borrow;
                add_buffer[add_i] = uint32_t(add_temp & 0xFFFFFFFFULL);
                add_borrow = (add_temp >> 32) & 1ULL;
            }
            add_buffer[0] = add_pb ? 0u : 1u;
        }
        else
        {
            for (int add_i = PRECISION; add_i > 0; add_i--)
            {
                uint64_t add_temp = uint64_t(a[add_i]) - uint64_t(b[add_i]) - add_borrow;
                add_buffer[add_i] = uint32_t(add_temp & 0xFFFFFFFFULL);
                add_borrow = (add_temp >> 32) & 1ULL;
            }
            add_buffer[0] = add_pa ? 0u : 1u;
        }
    }
    assign(real_r, temp);
}

void Real32::mul(const Real32& real_a, const Real32& real_b, Real32& real_r)
{
    const uint32_t* a = real_a.real;
    const uint32_t* b = real_b.real;

    Real32 temp = Real32();
    uint32_t* mul_buffer = temp.real;

    uint64_t mul_product[2 * PRECISION - 1];

    for (int mul_i = 0; mul_i < 2 * PRECISION - 1; mul_i++)
        mul_product[mul_i] = 0ULL;

    for (int mul_i = 0; mul_i < PRECISION; mul_i++)
    {
        uint64_t mul_carry = 0ULL;

        for (int mul_j = 0; mul_j < PRECISION; mul_j++)
        {
            uint64_t mul_temp = uint64_t(a[PRECISION - mul_i]) * uint64_t(b[PRECISION - mul_j]);
            mul_product[mul_i + mul_j] += mul_temp + mul_carry;
            mul_carry = mul_product[mul_i + mul_j] >> 32;
            mul_product[mul_i + mul_j] &= 0xFFFFFFFFULL;
        }

        if (mul_i + PRECISION < 2 * PRECISION - 1)
            mul_product[mul_i + PRECISION] += mul_carry;
    }

    if (PRECISION > 1 && mul_product[PRECISION - 2] >= HALF_BASE)
    {
        uint64_t mul_carry = 1ULL;
        for (int mul_i = PRECISION - 1; mul_i < 2 * PRECISION - 1 && mul_carry > 0; mul_i++)
        {
            mul_product[mul_i] += mul_carry;
            mul_carry = mul_product[mul_i] >> 32;
            mul_product[mul_i] &= 0xFFFFFFFFULL;
        }
    }

    for (int mul_i = 0; mul_i < PRECISION; mul_i++)
        mul_buffer[mul_i + 1] = uint32_t(mul_product[2 * PRECISION - 2 - mul_i]);

    mul_buffer[0] = ((a[0] == 0u) != (b[0] == 0u)) ? 1u : 0u;

    assign(real_r, temp);
}