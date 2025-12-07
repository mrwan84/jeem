# FlexTable - Jeem Data Library

A comprehensive data analysis library for the Jeem programming language.

## Installation

Copy `flextable.jm` to your project's modules folder.

## Usage

```javascript
import "modules/flextable.jm" as ft

// Create a DataFrame
df = ft.DataFrame({
    name: ["Alice", "Bob", "Charlie"],
    age: [25, 30, 35],
    salary: [50000, 60000, 75000]
})

ft.printDF(df)
```

## Series

A Series is a one-dimensional labeled array.

### Creating Series

```javascript
// From array
s = ft.Series([10, 20, 30, 40, 50], "values");

// From range
s = ft.SeriesRange(0, 10, "numbers"); // [0,1,2,3,4,5,6,7,8,9]
```

### Series Operations

| Function            | Description                  |
| ------------------- | ---------------------------- |
| `sLen(s)`           | Get length                   |
| `sGet(s, idx)`      | Get value at index           |
| `sSet(s, idx, val)` | Set value at index           |
| `sHead(s, n)`       | First n elements (default 5) |
| `sTail(s, n)`       | Last n elements (default 5)  |

### Series Statistics

| Function       | Description              |
| -------------- | ------------------------ |
| `sSum(s)`      | Sum of values            |
| `sMean(s)`     | Mean (average)           |
| `sMin(s)`      | Minimum value            |
| `sMax(s)`      | Maximum value            |
| `sStd(s)`      | Standard deviation       |
| `sMedian(s)`   | Median value             |
| `sDescribe(s)` | All statistics as object |

```javascript
s = ft.Series([10, 20, 30, 40, 50], "values");
print(ft.sSum(s)); // 150
print(ft.sMean(s)); // 30
print(ft.sMin(s)); // 10
print(ft.sMax(s)); // 50
```

### Series Transformations

| Function                | Description                    |
| ----------------------- | ------------------------------ |
| `sApply(s, callback)`   | Apply function to each element |
| `sFilter(s, predicate)` | Filter by condition            |
| `sMap(s, mapping)`      | Map values using object        |
| `sSort(s, ascending)`   | Sort values                    |
| `sUnique(s)`            | Get unique values              |
| `sValueCounts(s)`       | Count occurrences              |

```javascript
s = ft.Series([1, 2, 3, 4, 5], "nums");

// Double each value
s2 = ft.sApply(s, (x) => x * 2);

// Filter values > 2
s3 = ft.sFilter(s, (x) => x > 2);

// Map values
s4 = ft.sMap(s, { 1: "one", 2: "two" });
```

### Series Null Handling

| Function            | Description        |
| ------------------- | ------------------ |
| `sFillna(s, value)` | Fill null values   |
| `sDropna(s)`        | Remove null values |

### Series Arithmetic

| Function       | Description                 |
| -------------- | --------------------------- |
| `sAdd(s, val)` | Add value or Series         |
| `sSub(s, val)` | Subtract value or Series    |
| `sMul(s, val)` | Multiply by value or Series |
| `sDiv(s, val)` | Divide by value or Series   |

```javascript
s1 = ft.Series([1, 2, 3], "a");
s2 = ft.Series([10, 20, 30], "b");

s3 = ft.sAdd(s1, 5); // [6, 7, 8]
s4 = ft.sAdd(s1, s2); // [11, 22, 33]
s5 = ft.sMul(s1, 2); // [2, 4, 6]
```

## DataFrame

A DataFrame is a two-dimensional table with labeled columns.

### Creating DataFrames

```javascript
// From object of arrays
df = ft.DataFrame({
    name: ["Alice", "Bob"],
    age: [25, 30],
    city: ["Berlin", "Munich"]
})

// From records (array of objects)
df = ft.dfFromRecords([
    {name: "Alice", age: 25},
    {name: "Bob", age: 30}
])

// From CSV string
csv = "name,age,score
Alice,25,85
Bob,30,90"
df = ft.dfFromCSV(csv, ",")

// From CSV file
df = ft.readCSV("data.csv", ",")
```

### DataFrame Info

| Function       | Description                    |
| -------------- | ------------------------------ |
| `shape(df)`    | Get [rows, cols]               |
| `info(df)`     | Get column info                |
| `head(df, n)`  | First n rows (default 5)       |
| `tail(df, n)`  | Last n rows (default 5)        |
| `describe(df)` | Statistics for numeric columns |

```javascript
ft.printDF(ft.head(df, 3));
print(ft.shape(df)); // [5, 4]
print(ft.info(df));
```

### Accessing Data

| Function               | Description                       |
| ---------------------- | --------------------------------- |
| `col(df, name)`        | Get column as Series              |
| `cols(df, names)`      | Get multiple columns as DataFrame |
| `row(df, idx)`         | Get row as object                 |
| `iloc(df, start, end)` | Get rows by index range           |

```javascript
// Get column
ages = ft.col(df, "age");

// Get multiple columns
subset = ft.cols(df, ["name", "salary"]);

// Get single row
r = ft.row(df, 0); // {name: "Alice", age: 25, ...}

// Get rows by index
subset = ft.iloc(df, 0, 3); // rows 0-2
```

### Column Operations

| Function                        | Description                     |
| ------------------------------- | ------------------------------- |
| `addCol(df, name, values)`      | Add new column                  |
| `dropCol(df, name)`             | Remove column                   |
| `renameCol(df, old, new)`       | Rename column                   |
| `apply(df, col, transformer)`   | Transform column values         |
| `assign(df, name, transformer)` | Create column from row function |

```javascript
// Add column from array
df = ft.addCol(df, "bonus", [5000, 6000, 7500]);

// Add column from scalar
df = ft.addCol(df, "country", "Germany");

// Add calculated column
df = ft.assign(df, "total", (r) => r.salary + r.bonus);

// Transform existing column
df = ft.apply(df, "name", (x) => x.upper());

// Remove column
df = ft.dropCol(df, "bonus");

// Rename column
df = ft.renameCol(df, "total", "compensation");
```

### Filtering

| Function                       | Description            |
| ------------------------------ | ---------------------- |
| `query(df, predicate)`         | Filter by row function |
| `filterEq(df, col, val)`       | column == value        |
| `filterNe(df, col, val)`       | column != value        |
| `filterGt(df, col, val)`       | column > value         |
| `filterLt(df, col, val)`       | column < value         |
| `filterGe(df, col, val)`       | column >= value        |
| `filterLe(df, col, val)`       | column <= value        |
| `filterContains(df, col, str)` | column contains string |
| `filterIn(df, col, list)`      | column in list         |

```javascript
// Filter by condition
berlin = ft.filterEq(df, "city", "Berlin");

// Filter by comparison
highPay = ft.filterGe(df, "salary", 60000);

// Filter by custom function
result = ft.query(df, (r) => r.age > 25 && r.city == "Berlin");

// Filter by list
cities = ft.filterIn(df, "city", ["Berlin", "Munich"]);
```

### Sorting

| Function                     | Description    |
| ---------------------------- | -------------- |
| `sortBy(df, col, ascending)` | Sort by column |

```javascript
// Sort ascending
sorted = ft.sortBy(df, "age", true);

// Sort descending
sorted = ft.sortBy(df, "salary", false);
```

### Grouping & Aggregation

| Function                           | Description           |
| ---------------------------------- | --------------------- |
| `groupBy(df, col)`                 | Create GroupBy object |
| `agg(gb, col, func)`               | Aggregate column      |
| `groupSum(df, groupCol, sumCol)`   | Group and sum         |
| `groupMean(df, groupCol, meanCol)` | Group and mean        |
| `groupCount(df, groupCol)`         | Group and count       |

Aggregation functions: `"sum"`, `"mean"`, `"avg"`, `"min"`, `"max"`, `"count"`, `"first"`, `"last"`

```javascript
// Group and sum
result = ft.groupSum(df, "city", "salary");

// Group and mean
result = ft.groupMean(df, "city", "age");

// Group and count
result = ft.groupCount(df, "city");

// Custom aggregation
gb = ft.groupBy(df, "city");
result = ft.agg(gb, "salary", "max");
```

### Joining & Merging

| Function                    | Description            |
| --------------------------- | ---------------------- |
| `merge(df1, df2, onCol)`    | Inner join             |
| `leftJoin(df1, df2, onCol)` | Left join              |
| `concat(dfs)`               | Concatenate vertically |

```javascript
// Inner join
result = ft.merge(employees, departments, "dept_id");

// Left join
result = ft.leftJoin(employees, departments, "dept_id");

// Concatenate
combined = ft.concat([df1, df2, df3]);
```

### Null Handling

| Function            | Description            |
| ------------------- | ---------------------- |
| `fillna(df, value)` | Fill null values       |
| `dropna(df)`        | Remove rows with nulls |

```javascript
// Fill nulls with 0
filled = ft.fillna(df, 0);

// Drop rows with any null
clean = ft.dropna(df);
```

### Duplicates

| Function                     | Description           |
| ---------------------------- | --------------------- |
| `dropDuplicates(df, subset)` | Remove duplicate rows |

```javascript
// Remove duplicates based on all columns
unique = ft.dropDuplicates(df, null);

// Remove duplicates based on specific columns
unique = ft.dropDuplicates(df, ["name", "city"]);
```

### Sampling

| Function        | Description             |
| --------------- | ----------------------- |
| `sample(df, n)` | Random sample of n rows |

```javascript
sampled = ft.sample(df, 10);
```

### Pivot Tables

| Function                                     | Description        |
| -------------------------------------------- | ------------------ |
| `pivot(df, index, columns, values, aggFunc)` | Create pivot table |

```javascript
sales = ft.DataFrame({
  region: ["North", "North", "South", "South"],
  product: ["A", "B", "A", "B"],
  amount: [100, 150, 200, 120],
});

// Pivot with sum
pivoted = ft.pivot(sales, "region", "product", "amount", "sum");
// Result:
// | region | A   | B   |
// | North  | 100 | 150 |
// | South  | 200 | 120 |
```

### Value Counts

| Function               | Description            |
| ---------------------- | ---------------------- |
| `valueCounts(df, col)` | Count values in column |

```javascript
counts = ft.valueCounts(df, "city");
// Returns DataFrame with 'value' and 'count' columns
```

### Correlation

| Function               | Description             |
| ---------------------- | ----------------------- |
| `corr(df, col1, col2)` | Correlation coefficient |

```javascript
r = ft.corr(df, "age", "salary");
```

### Export

| Function                     | Description           |
| ---------------------------- | --------------------- |
| `toCSV(df, sep)`             | Convert to CSV string |
| `saveCSV(df, filename, sep)` | Save to CSV file      |

```javascript
// To CSV string
csv = ft.toCSV(df, ",");
print(csv);

// Save to file
ft.saveCSV(df, "output.csv", ",");
```

### Display

| Function      | Description         |
| ------------- | ------------------- |
| `dfStr(df)`   | DataFrame as string |
| `printDF(df)` | Print DataFrame     |
| `sStr(s)`     | Series as string    |
| `printS(s)`   | Print Series        |

## Complete Example

```javascript
import "modules/flextable.jm" as ft

// Create employee data
employees = ft.DataFrame({
    name: ["Alice", "Bob", "Charlie", "Diana", "Eve"],
    dept: ["Eng", "Sales", "Eng", "HR", "Sales"],
    salary: [75000, 60000, 80000, 55000, 65000],
    years: [3, 5, 7, 2, 4]
})

print("Original Data:")
ft.printDF(employees)

// Filter high earners
highEarners = ft.filterGe(employees, "salary", 65000)
print("\nHigh Earners (>=65000):")
ft.printDF(highEarners)

// Add bonus column
employees = ft.assign(employees, "bonus", (r) => r.salary * 0.1)
print("\nWith Bonus:")
ft.printDF(employees)

// Group by department
print("\nSalary by Department:")
ft.printDF(ft.groupSum(employees, "dept", "salary"))

print("\nAverage Years by Department:")
ft.printDF(ft.groupMean(employees, "dept", "years"))

// Sort by salary descending
print("\nSorted by Salary:")
ft.printDF(ft.sortBy(employees, "salary", false))

// Statistics
print("\nStatistics:")
ft.printDF(ft.describe(employees))

// Export
csv = ft.toCSV(employees, ",")
print("\nCSV Export:")
print(csv)
```

## License

MIT License - Feel free to use in your Jeem projects.
