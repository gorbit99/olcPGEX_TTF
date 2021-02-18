#ifndef OLC_PGEX_TTF_H
#define OLC_PGEX_TTF_H

#include "olcPixelGameEngine.h"

#ifdef WIN32
#include <ft2build.h>
#pragma comment(lib, "freetype.lib")
#else
#include <freetype2/ft2build.h>
#endif

#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace olc {
    struct FontRect {
        olc::vi2d offset;
        olc::vi2d size;
    };

    class Font : public olc::PGEX {
    public:
        Font() = default;

        ~Font() {
            if (fontFace != nullptr) {
                FT_Done_Face(fontFace);
            }
        }

        Font(std::string path, int fontSize) : fontSize(fontSize) {
            FT_Error error = FT_New_Face(library, path.c_str(), 0, &fontFace);
            if (error) {
                const char *errorString = FT_Error_String(error);
                if (errorString == nullptr) {
                    std::cerr
                            << "An unknown error occured while loading the font! Error code: "
                            << error << "\n";
                } else {
                    std::cerr << errorString << "\n";
                }
            }

            error = FT_Set_Pixel_Sizes(fontFace, 0, fontSize);
            if (error) {
                const char *errorString = FT_Error_String(error);
                if (errorString == nullptr) {
                    std::cerr
                            << "An unknown error occured while loading the font!Error Code: "
                            << error << "\n";
                } else {
                    std::cerr << errorString << "\n";
                }
            }
        }

        Font(const Font &other) = delete;
        Font(Font &&other) {
            fontFace = other.fontFace;
            fontSize = other.fontSize;
            fallbacks = std::move(other.fallbacks);
            other.fontFace = nullptr;
            other.fallbacks.clear();
        }

        Font &operator=(const Font &other) = delete;
        Font &operator=(Font &&other) {
            fontFace = other.fontFace;
            fontSize = other.fontSize;
            fallbacks = std::move(other.fallbacks);
            other.fontFace = nullptr;
            other.fallbacks.clear();
            return *this;
        }

        void DrawString(std::u32string string, int x, int y,
                        olc::Pixel color = olc::BLACK, float angle = 0.0f) {
            FT_Matrix rotMat;
            rotMat.xx = (FT_Fixed)(std::cos(angle) * 0x10000L);
            rotMat.xy = (FT_Fixed)(-std::sin(angle) * 0x10000L);
            rotMat.yx = (FT_Fixed)(std::sin(angle) * 0x10000L);
            rotMat.yy = (FT_Fixed)(std::cos(angle) * 0x10000L);

            FT_Vector pen;
            pen.x = x * 64;
            pen.y = (pge->ScreenHeight() - y) * 64;

            olc::Pixel::Mode prevMode = pge->GetPixelMode();
            pge->SetPixelMode(olc::Pixel::ALPHA);

            Font *prevToUse = nullptr;

            for (size_t i = 0; i < string.size(); i++) {
                char32_t chr = string[i];
                Font *toUse = this;
                FT_UInt chrIndex = GetCharIndex(chr);

                if (chrIndex == 0) {
                    for (auto &font : fallbacks) {
                        FT_UInt fbChr = font.GetCharIndex(chr);
                        if (fbChr != 0) {
                            chrIndex = fbChr;
                            toUse = &font;
                        }
                    }
                }

                if (prevToUse == toUse) {
                    FT_Vector kern;
                    FT_Get_Kerning(fontFace, string[i - 1], chr,
                                   FT_KERNING_DEFAULT, &kern);

                    pen.x += kern.x;
                    pen.y += kern.y;
                }

                FT_Set_Transform(toUse->fontFace, &rotMat, &pen);
                FT_Error error = FT_Load_Char(toUse->fontFace, chr,
                                              FT_LOAD_RENDER | FT_LOAD_COLOR);
                if (error) {
                    const char *errorString = FT_Error_String(error);
                    if (errorString == nullptr) {
                        std::cerr
                                << "An unknown error occured while rendering a glyph!  Error code: "
                                << error << "\n";
                    } else {
                        std::cerr << errorString << "\n";
                    }
                    return;
                }

                FT_Bitmap bmp = toUse->fontFace->glyph->bitmap;
                FT_GlyphSlot slot = toUse->fontFace->glyph;
                DrawBitmap(slot->bitmap_left,
                           pge->ScreenHeight() - slot->bitmap_top, bmp, color);

                pen.x += toUse->fontFace->glyph->advance.x;
                pen.y += toUse->fontFace->glyph->advance.y;

                prevToUse = toUse;
            }

            pge->SetPixelMode(prevMode);
        }

        void DrawString(std::u32string string, olc::vi2d pos,
                        olc::Pixel color = olc::BLACK, float angle = 0.0f) {
            DrawString(string, pos.x, pos.y, color, angle);
        }

        FontRect GetStringBounds(std::u32string string, float angle = 0.0f) {
            FT_Matrix rotMat;
            rotMat.xx = (FT_Fixed)(std::cos(angle) * 0x10000L);
            rotMat.xy = (FT_Fixed)(-std::sin(angle) * 0x10000L);
            rotMat.yx = (FT_Fixed)(std::sin(angle) * 0x10000L);
            rotMat.yy = (FT_Fixed)(std::cos(angle) * 0x10000L);

            FT_Vector pen;
            pen.x = 0;
            pen.y = 0;

            olc::FontRect rect;
            int intMax = std::numeric_limits<int>::max();
            int intMin = std::numeric_limits<int>::min();

            int minX = intMax;
            int minY = intMax;
            int maxX = intMin;
            int maxY = intMin;

            Font *prevToUse = nullptr;

            for (size_t i = 0; i < string.size(); i++) {
                char32_t chr = string[i];

                Font *toUse = this;
                FT_UInt chrIndex = GetCharIndex(chr);

                if (chrIndex == 0) {
                    for (auto &font : fallbacks) {
                        FT_UInt fbChr = font.GetCharIndex(chr);
                        if (fbChr != 0) {
                            chrIndex = fbChr;
                            toUse = &font;
                        }
                    }
                }

                if (prevToUse == toUse) {
                    FT_Vector kern;
                    FT_Get_Kerning(fontFace, string[i - 1], chr,
                                   FT_KERNING_DEFAULT, &kern);

                    pen.x += kern.x;
                    pen.y += kern.y;
                }

                FT_Set_Transform(toUse->fontFace, &rotMat, &pen);
                FT_Error error = FT_Load_Char(toUse->fontFace, chr,
                                              FT_LOAD_BITMAP_METRICS_ONLY);
                if (error) {
                    const char *errorString = FT_Error_String(error);
                    if (errorString == nullptr) {
                        std::cerr
                                << "An unknown error occured while loading a glyph!  Error code: "
                                << error << "\n";
                    } else {
                        std::cerr << errorString << "\n";
                    }
                    return olc::FontRect{{0, 0}, {0, 0}};
                }

                FT_GlyphSlot slot = toUse->fontFace->glyph;
                FT_Glyph glyph;
                FT_Get_Glyph(slot, &glyph);

                FT_BBox bbox;
                FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_GRIDFIT, &bbox);

                if (bbox.xMin < minX) {
                    minX = bbox.xMin;
                }
                if (bbox.xMax > maxX) {
                    maxX = bbox.xMax;
                }
                if (bbox.yMin < minY) {
                    minY = bbox.yMin;
                }
                if (bbox.yMax > maxY) {
                    maxY = bbox.yMax;
                }

                pen.x += slot->advance.x;
                pen.y += slot->advance.y;

                FT_Done_Glyph(glyph);
            }

            minX /= 64;
            minY /= 64;
            maxX /= 64;
            maxY /= 64;
            return olc::FontRect{{minX, -maxY}, {maxX - minX, maxY - minY}};
        }

        olc::Sprite *RenderStringToSprite(std::u32string string,
                                          olc::Pixel color) {
            olc::FontRect rect = GetStringBounds(string);
            olc::Sprite *sprite = new olc::Sprite{rect.size.x, rect.size.y};

            for (int x = 0; x < rect.size.x; x++) {
                for (int y = 0; y < rect.size.y; y++) {
                    sprite->SetPixel(x, y, olc::BLANK);
                }
            }

            FT_Vector pen;
            pen.x = -rect.offset.x;
            pen.y = (pge->ScreenHeight() + rect.offset.y) * 64;

            olc::Pixel::Mode prevMode = pge->GetPixelMode();
            pge->SetPixelMode(olc::Pixel::ALPHA);

            olc::Font *prevToUse = nullptr;

            for (size_t i = 0; i < string.size(); i++) {
                char32_t chr = string[i];

                Font *toUse = this;
                FT_UInt chrIndex = GetCharIndex(chr);

                if (chrIndex == 0) {
                    for (auto &font : fallbacks) {
                        FT_UInt fbChr = font.GetCharIndex(chr);
                        if (fbChr != 0) {
                            chrIndex = fbChr;
                            toUse = &font;
                        }
                    }
                }

                if (prevToUse == toUse) {
                    FT_Vector kern;
                    FT_Get_Kerning(fontFace, string[i - 1], chr,
                                   FT_KERNING_DEFAULT, &kern);

                    pen.x += kern.x;
                    pen.y += kern.y;
                }

                FT_Set_Transform(toUse->fontFace, nullptr, &pen);
                FT_Error error = FT_Load_Char(toUse->fontFace, chr,
                                              FT_LOAD_RENDER | FT_LOAD_COLOR);
                if (error) {
                    const char *errorString = FT_Error_String(error);
                    if (errorString == nullptr) {
                        std::cerr
                                << "An unknown error occured while rendering a glyph!  Error code: "
                                << error << "\n";
                    } else {
                        std::cerr << errorString << "\n";
                    }
                    return nullptr;
                }

                FT_Bitmap bmp = toUse->fontFace->glyph->bitmap;
                FT_GlyphSlot slot = toUse->fontFace->glyph;
                DrawBitmapTo(slot->bitmap_left,
                             pge->ScreenHeight() - slot->bitmap_top, bmp, color,
                             sprite);

                pen.x += toUse->fontFace->glyph->advance.x;
                pen.y += toUse->fontFace->glyph->advance.y;
            }

            pge->SetPixelMode(prevMode);

            return sprite;
        }

        olc::Decal *RenderStringToDecal(std::u32string string,
                                        olc::Pixel color) {
            Sprite *sprite = RenderStringToSprite(string, color);
            olc::Decal *decal = new olc::Decal{sprite};
            return decal;
        }

        olc::Renderable RenderStringToRenderable(std::u32string string,
                                                 olc::Pixel color) {
            Sprite *sprite = RenderStringToSprite(string, color);
            olc::Renderable renderable;
            renderable.Create(sprite->width, sprite->height);

            for (int x = 0; x < sprite->width; x++) {
                for (int y = 0; y < sprite->height; y++) {
                    renderable.Sprite()->SetPixel(x, y, sprite->GetPixel(x, y));
                }
            }

            delete sprite;

            renderable.Decal()->Update();

            return renderable;
        }

        void AddFallbackFont(std::string path) {
            fallbacks.emplace_back(path, fontSize);
        }

        static bool init() {
            FT_Error error = FT_Init_FreeType(&library);

            if (error) {
                const char *errorString = FT_Error_String(error);
                if (errorString == nullptr) {
                    std::cerr
                            << "An unknown error occured while loading the font library! "
                               "Error code: "
                            << error << "\n";
                } else {
                    std::cerr << errorString << "\n";
                }

                return false;
            }
            return true;
        }

    private:
        void DrawBitmap(int x, int y, FT_Bitmap bmp, olc::Pixel color) {
            switch (bmp.pixel_mode) {
            case FT_PIXEL_MODE_MONO:
                for (size_t bx = 0; bx < bmp.width; bx++) {
                    for (size_t by = 0; by < bmp.rows; by++) {
                        int byteOffset = bx / 8;
                        char byte = bmp.buffer[by * bmp.pitch + byteOffset];
                        bool val = (byte >> (7 - bx % 8)) & 1;
                        if (val) {
                            pge->Draw(x + bx, y + by, color);
                        }
                    }
                }
                break;
            case FT_PIXEL_MODE_GRAY:
                for (size_t bx = 0; bx < bmp.width; bx++) {
                    for (size_t by = 0; by < bmp.rows; by++) {
                        uint8_t byte = bmp.buffer[by * bmp.pitch + bx];
                        if (byte == 0) {
                            continue;
                        }
                        color.a = byte;
                        pge->Draw(x + bx, y + by, color);
                    }
                }
                break;
            case FT_PIXEL_MODE_GRAY2: break;
            case FT_PIXEL_MODE_GRAY4: break;
            case FT_PIXEL_MODE_LCD: break;
            case FT_PIXEL_MODE_LCD_V: break;
            case FT_PIXEL_MODE_BGRA:
                for (size_t bx = 0; bx < bmp.width; bx++) {
                    for (size_t by = 0; by < bmp.rows; by++) {
                        olc::Pixel pixel{
                                bmp.buffer[by * bmp.pitch + bx * 4 + 2],
                                bmp.buffer[by * bmp.pitch + bx * 4 + 1],
                                bmp.buffer[by * bmp.pitch + bx * 4 + 0],
                                bmp.buffer[by * bmp.pitch + bx * 4 + 3],
                        };
                        pge->Draw(x + bx, y + by, pixel);
                    }
                }
                break;
            }
        }
        void DrawBitmapTo(int x, int y, FT_Bitmap bmp, olc::Pixel color,
                          olc::Sprite *sprite) {
            switch (bmp.pixel_mode) {
            case FT_PIXEL_MODE_MONO:
                for (size_t bx = 0; bx < bmp.width; bx++) {
                    for (size_t by = 0; by < bmp.rows; by++) {
                        int byteOffset = bx / 8;
                        char byte = bmp.buffer[by * bmp.pitch + byteOffset];
                        bool val = (byte >> (7 - bx % 8)) & 1;
                        if (val) {
                            sprite->SetPixel(x + bx, y + by, color);
                        }
                    }
                }
                break;
            case FT_PIXEL_MODE_GRAY:
                for (size_t bx = 0; bx < bmp.width; bx++) {
                    for (size_t by = 0; by < bmp.rows; by++) {
                        uint8_t byte = bmp.buffer[by * bmp.pitch + bx];
                        if (byte == 0) {
                            continue;
                        }
                        color.a = byte;
                        sprite->SetPixel(x + bx, y + by, color);
                    }
                }
                break;
            case FT_PIXEL_MODE_GRAY2: break;
            case FT_PIXEL_MODE_GRAY4: break;
            case FT_PIXEL_MODE_LCD: break;
            case FT_PIXEL_MODE_LCD_V: break;
            case FT_PIXEL_MODE_BGRA:
                for (size_t bx = 0; bx < bmp.width; bx++) {
                    for (size_t by = 0; by < bmp.rows; by++) {
                        olc::Pixel pixel{
                                bmp.buffer[by * bmp.pitch + bx * 4 + 2],
                                bmp.buffer[by * bmp.pitch + bx * 4 + 1],
                                bmp.buffer[by * bmp.pitch + bx * 4 + 0],
                                bmp.buffer[by * bmp.pitch + bx * 4 + 3],
                        };
                        sprite->SetPixel(x + bx, y + by, pixel);
                    }
                }
                break;
            }
        }

        FT_UInt GetCharIndex(char32_t charCode) {
            return FT_Get_Char_Index(fontFace, charCode);
        }

        FT_Face fontFace = nullptr;
        std::vector<Font> fallbacks;
        int fontSize;

        static FT_Library library;
    };
} // namespace olc

#ifdef OLC_PGEX_TTF
#undef OLC_PGEX_TTF

FT_Library olc::Font::library;

#endif

#endif
