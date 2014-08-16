#ifndef COLOR_HPP_
#define COLOR_HPP_

class Color
{
public:
    Color();
    Color(int newColor);

    enum {
        White = 0, Black = 1, NoColor = 2
    };

    Color oppositeColor() const { return color ^ 1; }

    operator int() const { return color; }
    operator int&() { return color; }
private:
    int color;
};

// Checks if the color is okay, i.e. black or white. 
bool isColorOk(Color c);

#endif