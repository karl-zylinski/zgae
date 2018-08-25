#pragma once

struct Color
{
    float r, g, b, a;
};

struct ColorUNorm
{
    unsigned char r, g, b, a;
};

struct ColorRGB
{
    float r, g, b;
};

Color color_random();
bool almost_equal(const Color& c1, const Color& c2);

inline void operator+=(Color& c1, const Color& c2)
{
    c1.r += c2.r;
    c1.g += c2.g;
    c1.b += c2.b;
    c1.a += c2.a;
}

inline void operator+=(ColorRGB& c1, const ColorRGB& c2)
{
    c1.r += c2.r;
    c1.g += c2.g;
    c1.b += c2.b;
}

inline ColorRGB operator+(const ColorRGB& c1, const ColorRGB& c2)
{
    return
    {
        c1.r + c2.r,
        c1.g + c2.g,
        c1.b + c2.b
    };
}

inline ColorRGB operator*(const ColorRGB& c, float s)
{
    return
    {
        c.r * s,
        c.g * s,
        c.b * s
    };
}
