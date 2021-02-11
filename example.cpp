#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_TTF
#include "olcPGEX_TTF.h"

class Example : public olc::PixelGameEngine {
    olc::Font font;

    bool OnUserCreate() override {
        olc::Font::init();
        font = olc::Font{"./res/Freshman.ttf", 40};
        font.AddFallbackFont("./res/NotoSerifJP-Regular.otf");
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override {
        Clear(olc::BLACK);
        font.DrawString(U"\x3041\x3042\x3043\x3044 aeiou", 20, 80, olc::GREEN);
        return true;
    }
};

int main() {
    Example example;
    example.Construct(600, 600, 1, 1, false, true, false);
    example.Start();

    return 0;
}
