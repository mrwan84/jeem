# Geometry - Jeem 2D/3D Geometry Library

Points, vectors, lines, shapes, and collision detection.

## Installation

Copy `geometry.jm` to your project's lib folder.

## Usage

```javascript
import "lib/geometry.jm" as geo
```

## 2D Points

```javascript
// Create point
p = geo.Point(10, 20)

// From polar coordinates
p = geo.pointFromPolar(angle, distance)

// Distance between points
d = geo.pointDistance(p1, p2)

// Midpoint
mid = geo.pointMidpoint(p1, p2)

// Angle from p1 to p2
angle = geo.pointAngle(p1, p2)

// Transform
p = geo.pointTranslate(p, dx, dy)
p = geo.pointRotate(p, angle)        // Around origin
p = geo.pointRotateAround(p, center, angle)
p = geo.pointScale(p, sx, sy)

// Interpolate
p = geo.pointLerp(p1, p2, 0.5)  // 50% between p1 and p2

// Compare
geo.pointEquals(p1, p2)
```

## 3D Points

```javascript
p = geo.Point3D(10, 20, 30)

geo.point3DDistance(p1, p2)
geo.point3DMidpoint(p1, p2)
geo.point3DTranslate(p, dx, dy, dz)
geo.point3DScale(p, sx, sy, sz)
```

## 2D Vectors

```javascript
// Create vector
v = geo.Vec2(1, 0)
v = geo.vec2FromPoints(p1, p2)

// Magnitude
len = geo.vec2Magnitude(v)
lenSq = geo.vec2MagnitudeSq(v)

// Normalize
n = geo.vec2Normalize(v)

// Operations
v = geo.vec2Add(v1, v2)
v = geo.vec2Sub(v1, v2)
v = geo.vec2Scale(v, 2)

// Products
dot = geo.vec2Dot(v1, v2)
cross = geo.vec2Cross(v1, v2)  // Scalar in 2D

// Transform
v = geo.vec2Perpendicular(v)   // 90° rotation
v = geo.vec2Rotate(v, angle)

// Angles
angle = geo.vec2Angle(v)
angle = geo.vec2AngleBetween(v1, v2)

// Reflect and project
v = geo.vec2Reflect(v, normal)
v = geo.vec2Project(v1, v2)
```

## Lines and Segments

### Line Segments

```javascript
// Create segment
seg = geo.Segment(p1, p2)

// Properties
len = geo.segmentLength(seg)
mid = geo.segmentMidpoint(seg)
p = geo.segmentPointAt(seg, 0.5)  // Point at t=0.5

// Intersection
p = geo.segmentIntersection(seg1, seg2)  // null if no intersection

// Distance to point
d = geo.segmentPointDistance(seg, p)
```

### Lines (Infinite)

```javascript
// Create line (ax + by + c = 0)
line = geo.Line(a, b, c)
line = geo.lineFromPoints(p1, p2)
line = geo.lineFromPointDir(p, direction)

// Distance from point
d = geo.linePointDistance(line, p)

// Closest point on line
p = geo.lineClosestPoint(line, point)

// Intersection
p = geo.lineIntersection(line1, line2)  // null if parallel
```

## Circles

```javascript
// Create circle
c = geo.Circle(center, radius)

// Properties
area = geo.circleArea(c)
circ = geo.circleCircumference(c)
p = geo.circlePointAt(c, angle)  // Point on circle

// Contains point
geo.circleContainsPoint(c, p)  // true/false

// Intersections
points = geo.circleCircleIntersection(c1, c2)  // 0, 1, or 2 points
points = geo.lineCircleIntersection(line, circle)
```

## Rectangles

```javascript
// Create rectangle
r = geo.Rect(x, y, width, height)
r = geo.rectFromCenter(cx, cy, width, height)
r = geo.rectFromCorners(p1, p2)

// Properties
area = geo.rectArea(r)
perim = geo.rectPerimeter(r)
center = geo.rectCenter(r)
corners = geo.rectCorners(r)  // [tl, tr, br, bl]

// Contains point
geo.rectContainsPoint(r, p)

// Rectangle intersection
geo.rectIntersects(r1, r2)            // true/false
intersection = geo.rectIntersection(r1, r2)  // Rect or null

// Circle collision
geo.rectCircleIntersects(r, c)
```

## Triangles

```javascript
// Create triangle
t = geo.Triangle(p1, p2, p3)

// Properties
area = geo.triangleArea(t)
perim = geo.trianglePerimeter(t)
center = geo.triangleCentroid(t)

// Contains point
geo.triangleContainsPoint(t, p)

// Circles
circum = geo.triangleCircumcircle(t)  // Smallest circle containing triangle
incircle = geo.triangleIncircle(t)    // Largest circle inside triangle
```

## Polygons

```javascript
// Create polygon
poly = geo.Polygon([p1, p2, p3, p4, ...])

// Regular polygon
poly = geo.regularPolygon(center, radius, sides)

// Properties
area = geo.polygonArea(poly)
perim = geo.polygonPerimeter(poly)
center = geo.polygonCentroid(poly)

// Contains point
geo.polygonContainsPoint(poly, p)

// Convexity
geo.polygonIsConvex(poly)  // true/false

// Bounding box
bbox = geo.polygonBoundingBox(poly)  // Returns Rect
```

## Bounding Box

```javascript
// Get bounding box of points
points = [p1, p2, p3, p4]
bbox = geo.boundingBox(points)  // Returns Rect
```

## Math Helpers

```javascript
// Constants
geo.PI()       // 3.14159...
geo.TAU()      // 6.28318...
geo.EPSILON()  // 0.000001

// Trigonometry
geo.sin(x)    geo.cos(x)    geo.tan(x)
geo.asin(x)   geo.acos(x)   geo.atan(x)
geo.atan2(y, x)

// Conversion
geo.degToRad(degrees)
geo.radToDeg(radians)

// Utilities
geo.sqrt(x)
geo.abs(x)
geo.clamp(v, min, max)
```

## Utility

```javascript
// Clone geometry object
copy = geo.clone(obj)

// Point to string
str = geo.pointStr(p)  // "(10, 20)"
geo.printPoint(p)
```

## Examples

### Collision Detection

```javascript
import "lib/geometry.jm" as geo

// Player and enemy
player = geo.Circle(geo.Point(100, 100), 20)
enemy = geo.Circle(geo.Point(120, 110), 15)

// Check collision
if (geo.circleCircleIntersection(player, enemy).len() > 0) {
    print("Collision!")
}

// Wall collision
wall = geo.Rect(200, 0, 10, 300)
if (geo.rectCircleIntersects(wall, player)) {
    print("Hit wall!")
}
```

### Line of Sight

```javascript
import "lib/geometry.jm" as geo

player = geo.Point(50, 50)
enemy = geo.Point(200, 150)
wall = geo.Segment(geo.Point(100, 0), geo.Point(100, 200))

sight = geo.Segment(player, enemy)
blocked = geo.segmentIntersection(sight, wall)

if (blocked != null) {
    print("Line of sight blocked at", geo.pointStr(blocked))
} else {
    print("Clear line of sight")
}
```

### Regular Polygons

```javascript
import "lib/geometry.jm" as geo

center = geo.Point(150, 150)

// Hexagon
hex = geo.regularPolygon(center, 50, 6)
print("Hexagon area:", geo.polygonArea(hex))

// Pentagon
pent = geo.regularPolygon(center, 50, 5)
print("Is convex:", geo.polygonIsConvex(pent))
```

### Vector Math

```javascript
import "lib/geometry.jm" as geo

// Movement vector
velocity = geo.Vec2(5, 3)
speed = geo.vec2Magnitude(velocity)
direction = geo.vec2Normalize(velocity)

// Bounce off wall (normal pointing up)
wallNormal = geo.Vec2(0, -1)
bounced = geo.vec2Reflect(velocity, wallNormal)

// Angle between vectors
v1 = geo.Vec2(1, 0)
v2 = geo.Vec2(0, 1)
angle = geo.vec2AngleBetween(v1, v2)
print("Angle:", geo.radToDeg(angle), "degrees")  // 90
```

### Closest Point

```javascript
import "lib/geometry.jm" as geo

// Find closest point on line to mouse position
line = geo.lineFromPoints(geo.Point(0, 100), geo.Point(300, 100))
mouse = geo.Point(150, 50)

closest = geo.lineClosestPoint(line, mouse)
print("Closest point:", geo.pointStr(closest))
```
