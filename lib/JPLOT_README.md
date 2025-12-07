# jPlot - Jeem Charting Library

ASCII terminal charts and SVG file output for data visualization.

## Installation

Copy `jplot.jm` to your project's lib folder.

## Usage

```javascript
import "lib/jjplot.jm" as jplot
```

## Quick Start

```javascript
// Simple line chart
data = [1, 4, 2, 8, 5, 7, 3, 6];
jjplot.plot(data, "My Chart");

// Bar chart
jjplot.plotBar(["A", "B", "C"], [10, 25, 15], "Sales");

// Scatter plot
jjplot.plotScatter([1, 2, 3, 4, 5], [2, 4, 3, 5, 4], "Correlation");
```

## Chart Types

### Line Chart

```javascript
data = [1, 4, 2, 8, 5, 7, 3, 6, 9, 4];
config = jplot.PlotConfig();
config.title = "Monthly Sales";
config.width = 50;
config.height = 15;
print(jplot.lineChart(data, config));
```

Output:

```
Monthly Sales

      9 |                    *
        |                   - -
        |       *          -   -
        |      - -    -*  -           -
      5 |         -  -  -       *    - -  -*
        |  *-      -      -    -      - --
        |    -   -         -  -
        | -   - -           -
      1 |*     *           *
        +--------------------------------------------------
```

### Bar Chart (Horizontal)

```javascript
labels = ["Jan", "Feb", "Mar", "Apr", "May"];
values = [120, 85, 142, 98, 165];
print(jplot.barChart(labels, values, null));
```

Output:

```
Jan |############################## 120
Feb |##################### 85
Mar |#################################### 142
Apr |######################## 98
May |########################################## 165
    +--------------------------------------------------
```

### Bar Chart (Vertical)

```javascript
config = jplot.PlotConfig();
config.height = 10;
print(
  jplot.barChartVertical(["Q1", "Q2", "Q3", "Q4"], [45, 78, 62, 89], config)
);
```

Output:

```
    89 |            ###
   71. |    ###     ###
   53. |    ### ### ###
   35. |### ### ### ###
   17. |### ### ### ###
       +----------------
        Q1  Q2  Q3  Q4
```

### Scatter Plot

```javascript
xData = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
yData = [2, 4, 5, 4, 5, 8, 7, 9, 8, 10];
config = jplot.PlotConfig();
config.title = "X vs Y";
print(jplot.scatterPlot(xData, yData, config));
```

### Histogram

```javascript
data = [1, 2, 2, 3, 3, 3, 4, 4, 5, 5, 5, 5, 6, 7, 8];
print(jplot.histogram(data, 5, null)); // 5 bins
```

### Sparkline

Compact inline visualization:

```javascript
data = [5, 3, 7, 2, 8, 4, 9, 1, 6];
print("Trend: [" + jplot.sparkline(data) + "]");
// Trend: [-_~ ^.* =]
```

### Box Plot

```javascript
data = [2, 3, 5, 7, 8, 10, 11, 12, 13, 15, 18, 20, 22, 25];
config = jplot.PlotConfig();
config.title = "Distribution";
print(jplot.boxPlot(data, config));
```

Output:

```
|------------------------------------------------|
|---------|##########|############|--------------|
|------------------------------------------------|

2                      12                       25
Min                   Median                 Max
```

### Pie Chart

```javascript
labels = ["Chrome", "Firefox", "Safari", "Edge"];
values = [65, 15, 10, 7];
print(jplot.pieChart(labels, values, null));
```

Output:

```
Chrome   |##########################| 65%
Firefox  |======| 15%
Safari   |++++| 10%
Edge     |--| 7%

Total: 97
```

### Multi-Line Chart

```javascript
datasets = [
  { name: "Product A", data: [10, 15, 12, 18, 20] },
  { name: "Product B", data: [5, 8, 12, 10, 15] },
  { name: "Product C", data: [8, 6, 9, 12, 11] },
];
config = jplot.PlotConfig();
config.title = "Comparison";
print(jplot.multiLineChart(datasets, config));
```

## SVG Output

Save charts as SVG files for web/documents:

```javascript
// Line chart SVG
data = [10, 25, 15, 30, 22, 35];
jplot.saveSvgLine(data, "chart.svg", "Sales Trend");

// Bar chart SVG
jplot.saveSvgBar(
  ["Mon", "Tue", "Wed"],
  [150, 230, 180],
  "daily.svg",
  "Revenue"
);

// Pie chart SVG
jplot.saveSvgPie(["A", "B", "C"], [40, 30, 30], "pie.svg", "Share");
```

Or get SVG string directly:

```javascript
svg = jplot.svgLineChart(data, config);
svg = jplot.svgBarChart(labels, values, config);
svg = jplot.svgPieChart(labels, values, config);
```

## Configuration

```javascript
config = jplot.PlotConfig();

// Dimensions
config.width = 60; // Chart width in characters
config.height = 20; // Chart height in characters

// Labels
config.title = "My Chart";
config.xLabel = "Time";
config.yLabel = "Value";

// Display options
config.showAxis = true;
config.showGrid = false;

// Custom characters
config.chars = {
  point: "*",
  line: "-",
  bar: "#",
  empty: " ",
  axis: "|",
  baseline: "-",
  corner: "+",
  grid: ".",
};
```

## Quick Plot Functions

Convenience functions that print directly:

```javascript
jplot.plot(data, "Title"); // Line chart
jplot.plotBar(labels, values, "Title"); // Bar chart
jplot.plotScatter(x, y, "Title"); // Scatter plot
jplot.plotHistogram(data, bins, "Title"); // Histogram
```

## Helper Functions

```javascript
jplot.minVal(array); // Find minimum value
jplot.maxVal(array); // Find maximum value
jplot.formatNum(n, decimals); // Format number
```

## Example: Dashboard

```javascript
import "lib/jplot.jm" as plot

print("=== Sales Dashboard ===\n")

// Monthly trend
sales = [120, 135, 98, 145, 160, 142, 178, 165, 190, 185, 210, 225]
jplot.plot(sales, "Monthly Sales 2024")

print("")

// Category breakdown
jplot.plotBar(
    ["Electronics", "Clothing", "Food", "Other"],
    [450, 320, 280, 150],
    "Sales by Category"
)

print("")

// Quick metrics
metrics = [85, 90, 78, 92, 88, 95, 91]
print("Weekly Performance: [" + jplot.sparkline(metrics) + "]")
```
