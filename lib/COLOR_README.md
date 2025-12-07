# Color - Jeem Color Library

RGB, HSL, HSV, CMYK conversion and color manipulation.

## Installation

Copy `color.jm` to your project's lib folder.

## Usage

```javascript
import "lib/color.jm" as color
```

## Color Creation

```javascript
// RGB (0-255)
c = color.RGB(255, 128, 0)
c = color.RGBA(255, 128, 0, 128)

// HSL (h: 0-360, s: 0-100, l: 0-100)
c = color.HSL(30, 100, 50)
c = color.HSLA(30, 100, 50, 0.5)

// From hex string
c = color.fromHex("#ff8000")
c = color.fromHex("ff8000")
c = color.fromHex("#f80")      // Short form
c = color.fromHex("#ff8000ff") // With alpha
```

## Color Conversion

```javascript
c = color.RGB(255, 128, 0)

// To hex
color.toHex(c)      // "#ff8000"
color.toHexA(c)     // "#ff8000ff"

// To HSL
hsl = color.toHsl(c)    // {h: 30, s: 100, l: 50}

// To HSV
hsv = color.toHsv(c)    // {h: 30, s: 100, v: 100}

// To CMYK
cmyk = color.toCmyk(c)  // {c: 0, m: 50, y: 100, k: 0}

// From HSV
c = color.hsvToRgb(30, 100, 100)

// From CMYK
c = color.cmykToRgb(0, 50, 100, 0)
```

## CSS Strings

```javascript
color.toRgbStr(c)   // "rgb(255, 128, 0)"
color.toRgbaStr(c)  // "rgba(255, 128, 0, 1)"
color.toHslStr(c)   // "hsl(30, 100%, 50%)"
```

## Color Manipulation

```javascript
c = color.RGB(255, 128, 0)

// Adjust lightness
color.lighten(c, 20)    // 20% lighter
color.darken(c, 20)     // 20% darker

// Adjust saturation
color.saturate(c, 20)   // 20% more saturated
color.desaturate(c, 20) // 20% less saturated

// Rotate hue
color.rotate(c, 90)     // Rotate hue by 90 degrees

// Other operations
color.invert(c)         // Invert color
color.grayscale(c)      // Convert to grayscale
color.complement(c)     // Complementary color (180° rotation)
color.setAlpha(c, 0.5)  // Set alpha (0-1)
```

## Color Mixing

```javascript
c1 = color.RGB(255, 0, 0)
c2 = color.RGB(0, 0, 255)

// Mix colors
color.mix(c1, c2)       // 50% mix
color.mix(c1, c2, 0.3)  // 30% c2, 70% c1

// Alpha blend
color.blend(bg, fg)     // Blend fg over bg using alpha
```

## Color Palettes

```javascript
base = color.RGB(255, 128, 0)

// Complementary (2 colors)
color.complementary(base)

// Triadic (3 colors, 120° apart)
color.triadic(base)

// Tetradic/Square (4 colors, 90° apart)
color.tetradic(base)

// Analogous (3 colors, similar hues)
color.analogous(base)        // Default 30° spread
color.analogous(base, 15)    // Custom spread

// Split-complementary (3 colors)
color.splitComplementary(base)

// Monochromatic (shades of one color)
color.monochromatic(base, 5)  // 5 shades

// Gradient
color.gradient(c1, c2, 10)    // 10 colors from c1 to c2
```

## Color Analysis

```javascript
c = color.RGB(255, 128, 0)

// Luminance (0-1)
color.luminance(c)

// Contrast ratio between two colors
color.contrastRatio(c1, c2)

// Light or dark?
color.isLight(c)    // true
color.isDark(c)     // false

// Best text color for background
color.textColor(bg) // Returns black or white

// Distance between colors
color.distance(c1, c2)
```

## Predefined Colors

```javascript
// Basic
color.black()    color.white()
color.red()      color.green()     color.blue()
color.yellow()   color.cyan()      color.magenta()
color.orange()   color.purple()    color.pink()
color.brown()    color.gray()

// Web colors
color.coral()    color.salmon()    color.tomato()
color.gold()     color.olive()     color.teal()
color.navy()     color.maroon()    color.lime()
color.aqua()     color.silver()    color.indigo()
color.violet()   color.turquoise() color.skyBlue()
color.steelBlue()  color.forestGreen()
color.seaGreen()   color.chocolate()
```

## Utility

```javascript
color.clone(c)        // Copy color
color.equals(c1, c2)  // Compare colors
color.colorStr(c)     // "RGB(255, 128, 0)"
color.printColor(c)   // Print color info
color.random()        // Random color
```

## Example: Color Palette Generator

```javascript
import "lib/color.jm" as color

base = color.RGB(52, 152, 219)  // Nice blue

print("Base color:", color.toHex(base))

print("\nTriadic palette:")
for c in color.triadic(base) {
    color.printColor(c)
}

print("\nGradient to orange:")
orange = color.orange()
for c in color.gradient(base, orange, 5) {
    print(color.toHex(c))
}
```
