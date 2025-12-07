# Jeem Math Library

A comprehensive math library for the Jeem programming language providing advanced mathematical operations including trigonometry, complex numbers, vectors, matrices, statistics, and more.

## Installation

Copy `math.jm` to your project's modules folder.

## Usage

```javascript
import "modules/math.jm" as math

// Use math functions
print(math.PI())              // 3.14159
print(math.sin(math.PI() / 2)) // 1
print(math.sqrt(2))           // 1.41421
```

## Constants

Constants are implemented as functions to ensure proper module export.

| Function  | Value         | Description       |
| --------- | ------------- | ----------------- |
| `PI()`    | 3.14159265... | Pi                |
| `E()`     | 2.71828182... | Euler's number    |
| `TAU()`   | 6.28318530... | Tau (2π)          |
| `PHI()`   | 1.61803398... | Golden ratio      |
| `LN2()`   | 0.69314718... | Natural log of 2  |
| `LN10()`  | 2.30258509... | Natural log of 10 |
| `SQRT2()` | 1.41421356... | Square root of 2  |
| `SQRT3()` | 1.73205080... | Square root of 3  |

```javascript
import "modules/math.jm" as m

print(m.PI())    // 3.14159
print(m.E())     // 2.71828
print(m.TAU())   // 6.28319
print(m.PHI())   // 1.61803
```

## Trigonometric Functions

| Function | Description |
| -------- | ----------- |
| `sin(x)` | Sine        |
| `cos(x)` | Cosine      |
| `tan(x)` | Tangent     |
| `cot(x)` | Cotangent   |
| `sec(x)` | Secant      |
| `csc(x)` | Cosecant    |

```javascript
import "modules/math.jm" as m

print(m.sin(0))              // 0
print(m.sin(m.PI() / 2))     // 1
print(m.cos(0))              // 1
print(m.cos(m.PI()))         // -1
print(m.tan(m.PI() / 4))     // 1
```

## Inverse Trigonometric Functions

| Function      | Description                  |
| ------------- | ---------------------------- |
| `asin(x)`     | Arcsine (returns radians)    |
| `acos(x)`     | Arccosine (returns radians)  |
| `atan(x)`     | Arctangent (returns radians) |
| `atan2(y, x)` | Two-argument arctangent      |

```javascript
import "modules/math.jm" as m

print(m.asin(1))       // 1.5708 (π/2)
print(m.acos(0))       // 1.5708 (π/2)
print(m.atan(1))       // 0.7854 (π/4)
print(m.atan2(1, 1))   // 0.7854 (π/4)
```

## Hyperbolic Functions

| Function  | Description        |
| --------- | ------------------ |
| `sinh(x)` | Hyperbolic sine    |
| `cosh(x)` | Hyperbolic cosine  |
| `tanh(x)` | Hyperbolic tangent |

```javascript
import "modules/math.jm" as m

print(m.sinh(0))   // 0
print(m.cosh(0))   // 1
print(m.tanh(1))   // 0.7616
```

## Exponential & Logarithmic Functions

| Function           | Description                |
| ------------------ | -------------------------- |
| `exp(x)`           | e^x                        |
| `ln(x)`            | Natural logarithm          |
| `log10(x)`         | Base-10 logarithm          |
| `log2(x)`          | Base-2 logarithm           |
| `logBase(x, base)` | Logarithm with custom base |

```javascript
import "modules/math.jm" as m

print(m.exp(1))          // 2.71828
print(m.exp(0))          // 1
print(m.ln(m.E()))       // 1
print(m.log10(100))      // 2
print(m.log2(8))         // 3
print(m.logBase(27, 3))  // 3
```

## Complex Numbers

Complex numbers are represented as objects: `{re: real, im: imaginary}`

### Creation

| Function                 | Description                          |
| ------------------------ | ------------------------------------ |
| `complex(re, im)`        | Create from real and imaginary parts |
| `complexPolar(r, theta)` | Create from polar coordinates        |

### Operations

| Function     | Description                 |
| ------------ | --------------------------- |
| `cabs(z)`    | Absolute value (magnitude)  |
| `carg(z)`    | Argument (angle in radians) |
| `conj(z)`    | Complex conjugate           |
| `cadd(a, b)` | Addition                    |
| `csub(a, b)` | Subtraction                 |
| `cmul(a, b)` | Multiplication              |
| `cdiv(a, b)` | Division                    |
| `cpow(z, n)` | Integer power               |
| `csqrt(z)`   | Square root                 |
| `cexp(z)`    | Complex exponential         |
| `cln(z)`     | Complex natural logarithm   |
| `cstr(z)`    | Format as string            |

```javascript
import "modules/math.jm" as m

// Create complex numbers
z1 = m.complex(3, 4)      // 3 + 4i
z2 = m.complex(1, 2)      // 1 + 2i

// Basic properties
print(m.cstr(z1))         // "3 + 4i"
print(m.cabs(z1))         // 5 (magnitude)
print(m.carg(z1))         // 0.927 (angle)

// Arithmetic
print(m.cstr(m.cadd(z1, z2)))  // "4 + 6i"
print(m.cstr(m.csub(z1, z2)))  // "2 + 2i"
print(m.cstr(m.cmul(z1, z2)))  // "-5 + 10i"
print(m.cstr(m.cdiv(z1, z2)))  // "2.2 - 0.4i"

// Advanced
print(m.cstr(m.conj(z1)))      // "3 - 4i"
print(m.cstr(m.cpow(z1, 2)))   // "-7 + 24i"
print(m.cstr(m.csqrt(z1)))     // "2 + 1i"

// Euler's identity: e^(iπ) + 1 = 0
iPi = m.complex(0, m.PI())
eiPi = m.cexp(iPi)
print(m.cstr(eiPi))            // "-1 + 0i"
```

## Vector Operations

Vectors are represented as arrays: `[x, y, z]`

| Function       | Description             |
| -------------- | ----------------------- |
| `dot(a, b)`    | Dot product             |
| `cross(a, b)`  | Cross product (3D only) |
| `magnitude(v)` | Vector length           |
| `normalize(v)` | Unit vector             |
| `vadd(a, b)`   | Vector addition         |
| `vsub(a, b)`   | Vector subtraction      |
| `vscale(v, s)` | Scalar multiplication   |

```javascript
import "modules/math.jm" as m

v1 = [1, 2, 3]
v2 = [4, 5, 6]

print(m.dot(v1, v2))        // 32
print(m.cross(v1, v2))      // [-3, 6, -3]
print(m.magnitude(v1))      // 3.74166
print(m.normalize(v1))      // [0.267, 0.534, 0.801]
print(m.vadd(v1, v2))       // [5, 7, 9]
print(m.vsub(v1, v2))       // [-3, -3, -3]
print(m.vscale(v1, 2))      // [2, 4, 6]
```

## Matrix Operations

Matrices are represented as 2D arrays: `[[row1], [row2], ...]`

### Creation

| Function            | Description         |
| ------------------- | ------------------- |
| `identity(n)`       | n×n identity matrix |
| `zeros(rows, cols)` | Zero matrix         |

### Operations

| Function       | Description               |
| -------------- | ------------------------- |
| `madd(a, b)`   | Matrix addition           |
| `msub(a, b)`   | Matrix subtraction        |
| `mmul(a, b)`   | Matrix multiplication     |
| `mscale(m, s)` | Scalar multiplication     |
| `transpose(m)` | Transpose                 |
| `det(m)`       | Determinant (2×2 and 3×3) |
| `trace(m)`     | Trace (sum of diagonal)   |

```javascript
import "modules/math.jm" as m

// Create matrices
m1 = [[1, 2], [3, 4]]
m2 = [[5, 6], [7, 8]]

print(m.madd(m1, m2))       // [[6, 8], [10, 12]]
print(m.mmul(m1, m2))       // [[19, 22], [43, 50]]
print(m.transpose(m1))      // [[1, 3], [2, 4]]
print(m.det(m1))            // -2
print(m.trace(m1))          // 5

// Identity and zero matrices
print(m.identity(3))        // [[1,0,0], [0,1,0], [0,0,1]]
print(m.zeros(2, 3))        // [[0,0,0], [0,0,0]]

// 3x3 determinant
m3 = [[1, 2, 3], [4, 5, 6], [7, 8, 10]]
print(m.det(m3))            // -3
```

## Statistical Functions

| Function            | Description             |
| ------------------- | ----------------------- |
| `sum(arr)`          | Sum of elements         |
| `product(arr)`      | Product of elements     |
| `mean(arr)`         | Arithmetic mean         |
| `median(arr)`       | Median value            |
| `mode(arr)`         | Most frequent value     |
| `variance(arr)`     | Population variance     |
| `stddev(arr)`       | Standard deviation      |
| `covariance(x, y)`  | Covariance              |
| `correlation(x, y)` | Correlation coefficient |

```javascript
import "modules/math.jm" as m

data = [2, 4, 4, 4, 5, 5, 7, 9]

print(m.sum(data))          // 40
print(m.product(data))      // 201600
print(m.mean(data))         // 5
print(m.median(data))       // 4.5
print(m.mode(data))         // 4
print(m.variance(data))     // 4
print(m.stddev(data))       // 2

// Correlation
x = [1, 2, 3, 4, 5]
y = [2, 4, 5, 4, 5]
print(m.correlation(x, y))  // 0.7746
```

## Combinatorics

| Function             | Description               |
| -------------------- | ------------------------- |
| `factorial(n)`       | n!                        |
| `permutations(n, r)` | P(n, r) = n! / (n-r)!     |
| `combinations(n, r)` | C(n, r) = n! / (r!(n-r)!) |

```javascript
import "modules/math.jm" as m

print(m.factorial(5))       // 120
print(m.factorial(10))      // 3628800
print(m.permutations(5, 3)) // 60
print(m.combinations(5, 3)) // 10
print(m.combinations(10, 5))// 252
```

## Number Theory

| Function          | Description             |
| ----------------- | ----------------------- |
| `gcd(a, b)`       | Greatest common divisor |
| `lcm(a, b)`       | Least common multiple   |
| `isPrime(n)`      | Check if prime          |
| `primeFactors(n)` | List of prime factors   |
| `fib(n)`          | Fibonacci number        |

```javascript
import "modules/math.jm" as m

print(m.gcd(48, 18))        // 6
print(m.lcm(4, 6))          // 12
print(m.isPrime(17))        // true
print(m.isPrime(18))        // false
print(m.primeFactors(60))   // [2, 2, 3, 5]
print(m.fib(10))            // 55
print(m.fib(20))            // 6765
```

## Utility Functions

| Function                                    | Description              |
| ------------------------------------------- | ------------------------ |
| `clamp(x, min, max)`                        | Clamp value to range     |
| `lerp(a, b, t)`                             | Linear interpolation     |
| `mapRange(x, inMin, inMax, outMin, outMax)` | Map value between ranges |
| `radians(deg)`                              | Degrees to radians       |
| `degrees(rad)`                              | Radians to degrees       |
| `sign(x)`                                   | Sign function (-1, 0, 1) |
| `hypot(a, b)`                               | √(a² + b²)               |
| `cbrt(x)`                                   | Cube root                |
| `nthRoot(x, n)`                             | Nth root                 |

```javascript
import "modules/math.jm" as m

print(m.clamp(15, 0, 10))   // 10
print(m.lerp(0, 100, 0.5))  // 50
print(m.mapRange(5, 0, 10, 0, 100))  // 50

print(m.radians(180))       // 3.14159
print(m.degrees(m.PI()))    // 180

print(m.sign(-5))           // -1
print(m.sign(0))            // 0
print(m.sign(5))            // 1

print(m.hypot(3, 4))        // 5
print(m.cbrt(27))           // 3
print(m.nthRoot(16, 4))     // 2
```

## Complete Example

```javascript
import "modules/math.jm" as m

// Calculate trajectory of a projectile
func trajectory(v0, angle, t) {
    rad = m.radians(angle)
    x = v0 * m.cos(rad) * t
    y = v0 * m.sin(rad) * t - 0.5 * 9.81 * t * t
    return [x, y]
}

// Initial velocity 50 m/s, angle 45 degrees
for t in range(0, 8) {
    pos = trajectory(50, 45, t)
    print("t={{t}}s: x={{pos[0]}}, y={{pos[1]}}")
}

// Complex number example: roots of quadratic
// x² + 2x + 5 = 0
// x = (-2 ± √(4-20)) / 2 = -1 ± 2i
a = 1
b = 2
c = 5
disc = b*b - 4*a*c  // -16
sqrtDisc = m.csqrt(m.complex(disc, 0))
x1 = m.cdiv(m.cadd(m.complex(-b, 0), sqrtDisc), m.complex(2*a, 0))
x2 = m.cdiv(m.csub(m.complex(-b, 0), sqrtDisc), m.complex(2*a, 0))
print("Roots:", m.cstr(x1), "and", m.cstr(x2))

// Statistics example
scores = [85, 90, 78, 92, 88, 76, 95, 89]
print("Average:", m.mean(scores))
print("Std Dev:", m.stddev(scores))
print("Median:", m.median(scores))
```

## License

MIT License - Feel free to use in your Jeem projects.
