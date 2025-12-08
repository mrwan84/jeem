# jPlot v2.0 - Charting Library for Jeem

A comprehensive data visualization library featuring ASCII/Unicode terminal charts and SVG export capabilities.

## Features

- **20+ Chart Types**: Line, bar, scatter, pie, histogram, box plot, candlestick, heatmap, and more
- **Three Character Sets**: ASCII, Unicode, and Block characters
- **Six SVG Themes**: Default, Dark, Light, Ocean, Forest, Monochrome
- **Statistical Functions**: Mean, median, standard deviation, linear regression, R²
- **Data Processing**: Normalization, moving averages, trend lines, interpolation

## Quick Start

```javascript
import "lib/jplot.jm" as jplot

// Simple line chart
data = [10, 25, 18, 35, 28, 42]
jplot.plot(data, "My Chart")

// Bar chart
labels = ["A", "B", "C", "D"]
values = [25, 40, 30, 45]
jplot.plotBar(labels, values, "Sales")

// Save as SVG
jplot.saveSvgLine(data, "chart.svg", "Trend")
```

## Configuration

```javascript
config = jplot.Config();

// Dimensions
config.width = 60; // Chart width in characters
config.height = 20; // Chart height in lines

// Labels
config.title = "Chart Title";
config.subtitle = "Subtitle";
config.xLabel = "X Axis";
config.yLabel = "Y Axis";

// Display options
config.showAxis = true; // Show axes
config.showGrid = false; // Show grid lines
config.showLegend = true; // Show legend (multi-series)
config.showValues = true; // Show values on bars

// Scale (null = auto)
config.yMin = null;
config.yMax = null;

// Style
config.charset = "ascii"; // "ascii", "unicode", "blocks"
config.theme = "default"; // For SVG: "default", "dark", "light", "ocean", "forest", "mono"

// Number formatting
config.decimals = 1; // Decimal places
config.siUnits = false; // Use K, M, B suffixes
config.percent = false; // Format as percentage

// Data processing
config.trendLine = false; // Add trend line
config.movingAverage = 0; // Moving average window (0 = disabled)

// Margins
config.marginLeft = 10;
config.marginRight = 2;
```

## Chart Types

### Line Chart

```javascript
data = [10, 15, 12, 18, 22, 19, 25];
config = jplot.Config();
config.title = "Weekly Sales";
config.showGrid = true;
print(jplot.lineChart(data, config));
```

### Multi-Line Chart

```javascript
datasets = [
  { name: "Product A", data: [10, 15, 12, 18] },
  { name: "Product B", data: [8, 12, 15, 20] },
  { name: "Product C", data: [5, 8, 10, 14] },
];
config = jplot.Config();
config.title = "Product Comparison";
print(jplot.multiLineChart(datasets, config));
```

### Area Chart

```javascript
data = [5, 12, 8, 15, 20, 18, 25];
config = jplot.Config();
config.title = "Growth Over Time";
print(jplot.areaChart(data, config));
```

### Horizontal Bar Chart

```javascript
labels = ["North", "South", "East", "West"];
values = [245, 198, 312, 267];
config = jplot.Config();
config.title = "Sales by Region";
print(jplot.barChart(labels, values, config));
```

### Vertical Bar Chart

```javascript
labels = ["Q1", "Q2", "Q3", "Q4"];
values = [150, 200, 175, 225];
config = jplot.Config();
config.title = "Quarterly Revenue";
print(jplot.barChartVertical(labels, values, config));
```

### Grouped Bar Chart

```javascript
labels = ["Jan", "Feb", "Mar"];
datasets = [
  { name: "2023", data: [100, 120, 115] },
  { name: "2024", data: [110, 135, 140] },
];
config = jplot.Config();
config.title = "Year-over-Year";
print(jplot.groupedBarChart(labels, datasets, config));
```

### Stacked Bar Chart

```javascript
labels = ["Q1", "Q2", "Q3", "Q4"];
datasets = [
  { name: "Online", data: [50, 60, 55, 70] },
  { name: "Retail", data: [80, 75, 90, 85] },
  { name: "B2B", data: [30, 40, 35, 45] },
];
config = jplot.Config();
config.title = "Revenue by Channel";
print(jplot.stackedBarChart(labels, datasets, config));
```

### Scatter Plot

```javascript
xData = [1, 2, 3, 4, 5, 6, 7, 8];
yData = [2.1, 4.2, 5.8, 8.1, 10.2, 12.0, 14.1, 16.3];
config = jplot.Config();
config.title = "Correlation Analysis";
config.trendLine = true; // Shows R² value
print(jplot.scatterPlot(xData, yData, config));
```

### Histogram

```javascript
data = [12, 15, 18, 22, 25, 28, 30, 32, 35, 38, 40, 42, 45, 48];
config = jplot.Config();
config.title = "Distribution";
print(jplot.histogram(data, 6, config)); // 6 bins
```

### Box Plot

```javascript
data = [10, 15, 20, 22, 25, 28, 30, 35, 40, 45, 50, 55, 60];
config = jplot.Config();
config.title = "Five-Number Summary";
print(jplot.boxPlot(data, config));
```

### Pie Chart

```javascript
labels = ["Chrome", "Firefox", "Safari", "Edge", "Other"];
values = [65, 15, 10, 7, 3];
config = jplot.Config();
config.title = "Browser Market Share";
print(jplot.pieChart(labels, values, config));
```

### Candlestick Chart (OHLC)

```javascript
ohlcData = [
  { open: 100, high: 105, low: 98, close: 103 },
  { open: 103, high: 110, low: 101, close: 108 },
  { open: 108, high: 115, low: 105, close: 112 },
];
config = jplot.Config();
config.title = "Stock Price";
print(jplot.candlestick(ohlcData, config));
```

### Heatmap

```javascript
matrix = [
  [0.2, 0.5, 0.8],
  [0.4, 0.7, 0.6],
  [0.6, 0.3, 0.4],
];
config = jplot.Config();
config.title = "Correlation Matrix";
print(jplot.heatmap(matrix, config));
```

### Gauge

```javascript
config = jplot.Config();
config.title = "CPU Usage";
print(jplot.gauge(73, 0, 100, config));
```

### Progress Bar

```javascript
config = jplot.Config();
config.width = 40;
print("Download: " + jplot.progressBar(67, 100, config));
```

### Data Table

```javascript
headers = ["Name", "Sales", "Revenue"];
rows = [
  ["Alice", 150, 4500],
  ["Bob", 120, 3600],
  ["Carol", 180, 5400],
];
config = jplot.Config();
config.title = "Team Performance";
print(jplot.table(headers, rows, config));
```

## Sparklines

Compact inline visualizations:

```javascript
data = [5, 8, 3, 9, 4, 7, 2, 8];

// Unicode blocks (best quality)
print("Trend: " + jplot.sparkline(data));
// Output: Trend: ▃▆▁█▂▅▁▆

// ASCII characters
print("Trend: " + jplot.sparklineAscii(data));
// Output: Trend: -^_*.-_^

// Block density
print("Trend: " + jplot.sparklineBars(data));
// Output: Trend: ▒█░█░▓░█
```

## SVG Export

Generate publication-quality vector graphics:

```javascript
// Line chart SVG
config = jplot.Config();
config.title = "Monthly Trend";
config.theme = "default"; // or "dark", "ocean", "forest"
svg = jplot.svgLineChart(data, config);
writeFile("line.svg", svg);

// Bar chart SVG
svg = jplot.svgBarChart(labels, values, config);
writeFile("bar.svg", svg);

// Pie chart SVG (donut style)
svg = jplot.svgPieChart(labels, values, config);
writeFile("pie.svg", svg);

// Multi-line chart SVG
svg = jplot.svgMultiLineChart(datasets, config);
writeFile("multi.svg", svg);
```

### SVG Themes

| Theme     | Background | Description             |
| --------- | ---------- | ----------------------- |
| `default` | White      | Clean professional look |
| `dark`    | #1a1a2e    | Dark mode               |
| `light`   | White      | Light with soft colors  |
| `ocean`   | #0a192f    | Deep blue theme         |
| `forest`  | #1b2d1b    | Green nature theme      |
| `mono`    | White      | Black and white         |

## Statistics Functions

```javascript
data = [10, 15, 20, 25, 30, 35, 40];

jplot.minVal(data); // 10
jplot.maxVal(data); // 40
jplot.sum(data); // 175
jplot.mean(data); // 25
jplot.median(data); // 25
jplot.stdDev(data); // 10.8
jplot.variance(data); // 116.67
jplot.quartile(data, 0.25); // Q1
jplot.quartile(data, 0.75); // Q3

// Linear regression
xData = [1, 2, 3, 4, 5];
yData = [2.1, 4.0, 6.2, 7.9, 10.1];
reg = jplot.linearRegression(xData, yData);
// reg.slope = 2.0
// reg.intercept = 0.1
// reg.r2 = 0.998
```

## Data Processing

```javascript
data = [10, 15, 12, 18, 22, 19, 25];

// Normalize to 0-1 range
norm = jplot.normalize(data);

// Moving average (smoothing)
smooth = jplot.movingAverage(data, 3);

// Trend line values
trend = jplot.trendLine(data);

// Interpolate to new length
interp = jplot.interpolate(data, 14);

// Cumulative sum
cumulative = jplot.cumSum(data);

// Differences
diffs = jplot.diff(data);

// Percent change
pctChange = jplot.pctChange(data);
```

## Character Sets

### ASCII (Default)

```
Sales Performance
         |
   250.0 |            *
         |         *-*
   200.0 |      *-*
         |   *-*
   150.0 |*-*
         +------------------
```

### Unicode

```
Sales Performance
         │
   250.0 │            ●
         │         ●─●
   200.0 │      ●─●
         │   ●─●
   150.0 │●─●
         └──────────────────
```

### Blocks

```
Sales Performance
         │
   250.0 │            █
         │         █▀█
   200.0 │      █▀█
         │   █▀█
   150.0 │█▀█
         └──────────────────
```

## Quick Reference

### Quick Plot Functions

```javascript
jplot.plot(data, title); // Line chart
jplot.plotBar(labels, values, title); // Bar chart
jplot.plotScatter(xData, yData, title); // Scatter plot
jplot.plotHistogram(data, bins, title); // Histogram
jplot.plotMulti(datasets, title); // Multi-line
```

### Quick SVG Export

```javascript
jplot.saveSvgLine(data, filename, title);
jplot.saveSvgBar(labels, values, filename, title);
jplot.saveSvgPie(labels, values, filename, title);
jplot.saveSvgMulti(datasets, filename, title);
```

### Utility Functions

```javascript
jplot.formatNum(value, config); // Format number
jplot.formatDecimal(value, places); // Format with decimals
jplot.padLeft(str, width); // Left-pad string
jplot.padRight(str, width); // Right-pad string
jplot.padCenter(str, width); // Center string
jplot.repeatChar(char, count); // Repeat character
```

## Examples

### Dashboard with Sparklines

```javascript
import "lib/jplot.jm" as jplot

sales = [120, 135, 128, 145, 160, 155, 175]
revenue = [80, 92, 85, 98, 110, 105, 120]
users = [1000, 1200, 1150, 1350, 1500, 1450, 1600]

print("Sales:   " + jplot.sparkline(sales) + " " + str(sales[len(sales)-1]))
print("Revenue: " + jplot.sparkline(revenue) + " $" + str(revenue[len(revenue)-1]) + "K")
print("Users:   " + jplot.sparkline(users) + " " + str(users[len(users)-1]))
```

### Report with Multiple Charts

```javascript
import "lib/jplot.jm" as jplot

config = jplot.Config()
config.charset = "unicode"
config.width = 50
config.height = 12

// Sales trend
config.title = "Monthly Sales"
print(jplot.lineChart(salesData, config))
print("")

// Regional breakdown
config.title = "By Region"
print(jplot.barChart(regions, regionSales, config))
print("")

// Market share
config.title = "Market Share"
print(jplot.pieChart(companies, shares, config))
```

### Financial Analysis

```javascript
import "lib/jplot.jm" as jplot

// Stock data
ohlc = [
    {open: 150, high: 155, low: 148, close: 153},
    // ... more data
]

config = jplot.Config()
config.title = "ACME Inc. (ACME)"
config.height = 15
print(jplot.candlestick(ohlc, config))

// Statistics
prices = [153, 156, 152, 158, 160]
print("")
print("Mean:    $" + jplot.formatDecimal(jplot.mean(prices), 2))
print("Std Dev: $" + jplot.formatDecimal(jplot.stdDev(prices), 2))
```

## Version History

- **v2.0**: Complete rewrite with professional features

  - 20+ chart types
  - SVG export with 6 themes
  - Statistical functions
  - Data processing utilities
  - Three character sets

- **v1.0**: Initial release
  - Basic ASCII charts
  - Simple SVG output

---
