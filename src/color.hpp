#ifndef COLOR_HPP_
#define COLOR_HPP_

class Color
{
public:
    Color() : color(NoColor) {};
    Color(int newColor) : color(newColor) {};

    enum {
        White = 0, Black = 1, NoColor = 2
    };

    operator int() const { return color; }
    operator int&() { return color; }
private:
    int color;
};

// Checks if the color is okay, i.e. black or white. 
inline bool isColorOk(Color c)
{
    return (c == Color::Black || c == Color::White);
}

#endif