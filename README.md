# olcPGEX_TTF

olcPGEX_TTF is an extension for the (PixelGameEngine)[https://github.com/OneLoneCoder/olcPixelGameEngine/] that allows you to 
open fonts and render text with them. It also includes built in functionality to rotate text, render to sprites, decals
or renderables and define fallback fonts in case a character is not included. It should handle unicode, and even some colored emoji fonts.

# Dependencies
The extension uses Freetype2. This is usually present on most systems as it's widely used.

## Windows
To compile a program using this extension, you'll need the freetype2 development binaries. You can download them off of
https://github.com/ubawurinna/freetype-windows-binaries/releases or compile them yourself. For the include libraries download
https://gitlab.freedesktop.org/freetype/freetype. You'll need at least the .lib file from the former link and the include directory from
the latter. You'll also need to place the dll files present in the former next to your resulting executable after building.

### Visual Studio
For VS2019, right click on your project, select properties, there C/C++, and add the previous include folder to your "Additional Include Directories"
setting. Afterwards, go to Linker > General, and add the folder containing your .lib file to the "Additional Library Directories" 
setting.

### MinGW
If you're using MinGW, add `-I/path/to/include -L/path/to/.lib/folder -lfreetype` to your build command.

## Linux
Install the freetype2 library from your distributions package manager then add `-lfreetype` to your build command.

# Usage
To include the PGEX, first define `OLC_PGEX_TTF`, then include the header file:

```cpp
#define OLC_PGEX_TTF
#include "olcPGEX_TTF.h"
```

You'll need to initialize the font library and then open a font.

The former step is done by calling `olc::Font::init()`. After this initialization, you can open a font by doing:

```cpp
//Variable in your class
olc::Font font;

//In OnUserCreate after calling init()
font = olc::Font("myFont.ttf", 20);
```

The 20 is the size the font will be loaded at. If you need a font in multiple sizes, load multiple of it.

If you need symbols from a different font, you can add a fallback, which will be used, if a certain character
doesn't exist in your original font. This is done by calling:

```cpp
font.AddFallbackFont("fallback.ttf");
```

After you've loaded your font, you can use it in two ways, either draw with it directly or get a sprite, decal or renderable
containing your string.

```cpp
//Drawing directly (string, x, y, color = black, rotation = 0)
font.DrawString(U"\x3041\x3042\x3043\x3044 aeiou", 20, 20, olc::BLUE);

//Drawing to a sprite (string, color = black)
font.RenderStringToSprite(U"\x3041\x3042\x3043\x3044 aeiou", olc::RED);
//Decal and Renderable have the same callstyle
```

The extension requires you to use u32strings (U"") for unicode support. To define a special character, use \x and their
hex code.

# Limitations
The extension can't handle bitmap color fonts yet.
