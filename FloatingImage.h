#include <windows.h>
#include <gdiplus.h>

class FloatingImage {
public:
    Gdiplus::Image *image = nullptr;

    POINTFLOAT position = POINTFLOAT{0, 0};
    SIZE size = SIZE{0, 0};

    FloatingImage() = default;

    FloatingImage(POINTFLOAT position, SIZE size, Gdiplus::Image *image) noexcept {
        this->position = position;
        this->size = size;
        this->image = image;
    }

    void draw(HDC hdc) {
        Gdiplus::Graphics(hdc).DrawImage(image, Gdiplus::Rect(position.x, position.y, size.cx, size.cy));
    }
};
