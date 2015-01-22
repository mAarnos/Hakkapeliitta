#ifndef COLOR_HPP_
#define COLOR_HPP_

#include <cstdint>

// Represents a single color.
class Color
{
public:
    Color() : color(NoColor) {};
    Color(const int8_t newColor) : color(newColor) {};

    enum : int8_t
    {
        White = 0, Black = 1, NoColor = 2
    };

    operator int8_t() const { return color; }
    operator int8_t&() { return color; }
    Color operator!() const { return color ^ 1; } // ! has a branch normally, in our case we can eliminate it by just using a xor.
private:
    int8_t color;
};

// Checks if the color is in the specified limits, i.e. Black, White, or NoColor. 
inline bool colorIsOkLoose(const Color c)
{
    return (c >= Color::Black && c <= Color::NoColor);
}

// Checks if the color is okay for playing, i.e. black or white. 
inline bool colorIsOkStrict(const Color c)
{
    return (c == Color::Black || c == Color::White);
}

#endif