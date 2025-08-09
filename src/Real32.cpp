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
    uint32_t* r = real_r.real;

    bool add_pa = a[0] == 0u;
    bool add_pb = b[0] == 0u;

    if (add_pa == add_pb)
    {
        uint32_t add_carry = 0u;

        for (int add_i = PRECISION; add_i > 0; add_i--)
        {
            uint32_t add_next = 0u;

            if (a[add_i] + b[add_i] < a[add_i])
                add_next = 1u;

            add_buffer[add_i] = a[add_i] + b[add_i] + add_carry;
            add_carry = add_next;
        }
        add_buffer[0] = add_pa ? 0u : 1u;
    }
    else
    {
        bool add_flip = false;

        for (int add_i = 1; add_i < ARRAY_SIZE; add_i++)
        {
            if (b[add_i] > a[add_i])
            {
                add_flip = true;
                break;
            }
            if (a[add_i] > b[add_i])
                break;
        }
        if (add_flip)
        {
            uint32_t add_borrow = 0u;

            for (int add_i = PRECISION; add_i > 0; add_i--)
            {
                add_buffer[add_i] = b[add_i] - a[add_i] - add_borrow;
                add_borrow = b[add_i] < a[add_i] + add_borrow ? add_borrow = 1u : add_borrow = 0u;
                if (b[add_i] < a[add_i] + add_borrow)
                    add_borrow = 1u;
                else
                    add_borrow = 0u;
            }
        }
        else {
            uint32_t add_borrow = 0u;

            for (int add_i = PRECISION; add_i > 0; add_i--) 
            {
                add_buffer[add_i] = a[add_i] - b[add_i] - add_borrow;
                if (a[add_i] < b[add_i] || a[add_i] < b[add_i] + add_borrow)
                    add_borrow = 1u;
                else
                    add_borrow = 0u;
            }
        }
        add_buffer[0] = add_pa == add_flip ? 0u : 1u;
        if (add_pa == add_flip) {
            add_buffer[0] = 1u;
        }
        else {
            add_buffer[0] = 0u;
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
    uint32_t* r = real_r.real;

    uint32_t mul_product[2 * PRECISION - 1];

    for (int mul_i = 0; mul_i < 2 * PRECISION - 1; mul_i++)
        mul_product[mul_i] = 0u;

    for (int mul_i = 0; mul_i < PRECISION; mul_i++) {
        uint32_t mul_carry = 0u;

        for (int mul_j = 0; mul_j < PRECISION; mul_j++) {
            uint32_t mul_next = 0;
            uint32_t mul_value = a[PRECISION - mul_i] * b[PRECISION - mul_j];

            if (mul_product[mul_i + mul_j] + mul_value < mul_product[mul_i + mul_j])
                mul_next++;

            mul_product[mul_i + mul_j] += mul_value;

            if (mul_product[mul_i + mul_j] + mul_carry < mul_product[mul_i + mul_j])
                mul_next++;

            mul_product[mul_i + mul_j] += mul_carry;
            uint32_t mul_lower_a = a[PRECISION - mul_i] & 0xFFFF;
            uint32_t mul_upper_a = a[PRECISION - mul_i] >> 16;
            uint32_t mul_lower_b = b[PRECISION - mul_j] & 0xFFFF;
            uint32_t mul_upper_b = b[PRECISION - mul_j] >> 16;
            uint32_t mul_lower = mul_lower_a * mul_lower_b;
            uint32_t mul_upper = mul_upper_a * mul_upper_b;
            uint32_t mul_mid = mul_lower_a * mul_upper_b;
            mul_upper += mul_mid >> 16; mul_mid = mul_mid << 16;

            if (mul_lower + mul_mid < mul_lower)
                mul_upper++;

            mul_lower += mul_mid;
            mul_mid = mul_lower_b * mul_upper_a;
            mul_upper += mul_mid >> 16;
            mul_mid = mul_mid << 16;

            if (mul_lower + mul_mid < mul_lower)
                mul_upper++;

            mul_carry = mul_upper + mul_next;
        }
        if (mul_i + PRECISION < 2 * PRECISION - 1)
            mul_product[mul_i + PRECISION] += mul_carry;
    }
    if (PRECISION > 1 && mul_product[PRECISION - 2] >= HALF_BASE) {

        for (int mul_i = PRECISION - 1; mul_i < 2 * PRECISION - 1; mul_i++) {

            if (mul_product[mul_i] + 1 > mul_product[mul_i]) {
                mul_product[mul_i]++;
                break;
            }
            mul_product[mul_i]++;
        }
    }

    for (int mul_i = 0; mul_i < PRECISION; mul_i++)
        mul_buffer[mul_i + 1] = mul_product[2 * PRECISION - 2 - mul_i];

    if ((a[0] == 0u) != (b[0] == 0u))
        mul_buffer[0] = 1u;

    assign(real_r, temp);
}