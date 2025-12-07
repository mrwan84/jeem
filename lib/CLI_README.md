# CLI - Jeem Command Line Library

Command line utilities: colors, progress bars, tables, boxes, trees, argument parsing.

## Installation

Copy `cli.jm` to your project's lib folder.

## Usage

```javascript
import "lib/cli.jm" as cli
```

## ANSI Colors

Colors work via the `chr(27)` escape character.

### Text Colors

```javascript
print(cli.fgRed() + "Red text" + cli.reset())
print(cli.fgGreen() + "Green text" + cli.reset())
print(cli.fgBlue() + "Blue text" + cli.reset())
print(cli.fgYellow() + "Yellow text" + cli.reset())
print(cli.fgCyan() + "Cyan text" + cli.reset())
print(cli.fgMagenta() + "Magenta text" + cli.reset())
```

### Background Colors

```javascript
print(cli.bgRed() + "Red background" + cli.reset())
print(cli.bgGreen() + "Green background" + cli.reset())
// etc.
```

### Text Styles

```javascript
print(cli.bold() + "Bold text" + cli.reset())
print(cli.dim() + "Dim text" + cli.reset())
print(cli.underline() + "Underlined" + cli.reset())
print(cli.italic() + "Italic" + cli.reset())
```

### 256 and RGB Colors

```javascript
print(cli.fg256(208) + "256 color" + cli.reset())
print(cli.fgRgb(255, 128, 0) + "True color" + cli.reset())
```

## Status Messages

```javascript
cli.printSuccess("File saved")     // [OK] File saved
cli.printError("Failed to load")   // [X] Failed to load
cli.printWarning("Low memory")     // [!] Low memory
cli.printInfo("Processing...")     // [i] Processing...
cli.printArrow("Next step")        // -> Next step
cli.printStar("Featured")          // [*] Featured
```

## Progress Bar

```javascript
// Create progress bar
pb = cli.ProgressBar(100, 40)  // total=100, width=40

// Update
cli.progressUpdate(pb, 50)   // Set to 50
cli.progressIncrement(pb, 5) // Add 5

// Set label
cli.progressLabel(pb, "Processing...")

// Render
str = cli.progressRender(pb)

// Print
cli.progressPrint(pb)

// Complete
cli.progressComplete(pb)
```

### Custom Progress Bar

```javascript
pb = cli.ProgressBar(100, 30, {
    filled: "#",
    empty: "-",
    left: "[",
    right: "]"
})
```

## Spinner

```javascript
// ASCII styles: "dots", "line", "circle", "square", "arrow", "bounce"
s = cli.Spinner("line")

cli.spinnerMessage(s, "Loading...")
frame = cli.spinnerNext(s)    // Get next frame
str = cli.spinnerRender(s)    // Render spinner + message
```

## Tables

```javascript
// Create table
t = cli.Table(["Name", "Age", "City"])

// Add rows
cli.tableAddRow(t, ["Alice", 30, "Berlin"])
cli.tableAddRow(t, ["Bob", 25, "Munich"])
cli.tableAddRow(t, ["Charlie", 35, "Hamburg"])

// Print
cli.tablePrint(t)
// +---------+-----+---------+
// | Name    | Age | City    |
// +---------+-----+---------+
// | Alice   | 30  | Berlin  |
// | Bob     | 25  | Munich  |
// | Charlie | 35  | Hamburg |
// +---------+-----+---------+
```

## Box Drawing

```javascript
// Styles: "single", "double", "rounded", "heavy" (ASCII)
text = "Hello World!\nThis is a box."
print(cli.box(text, "single", 1))
// +-----------------+
// |                 |
// | Hello World!    |
// | This is a box.  |
// |                 |
// +-----------------+
```

## Argument Parsing

```javascript
args = ["--output", "file.txt", "-v", "--count=5", "input.txt"]
parsed = cli.parseArgs(args, null)

// Result:
// {
//   _: ["input.txt"],          // Positional args
//   flags: {v: true},          // Boolean flags
//   options: {output: "file.txt", count: "5"}
// }

// Helpers
cli.hasFlag(parsed, "v")           // true
cli.getOption(parsed, "output", "default.txt")  // "file.txt"
cli.getOption(parsed, "missing", "default")     // "default"
```

## Lists

```javascript
items = ["First item", "Second item", "Third item"]

// Bulleted list
print(cli.bulletList(items))
//   * First item
//   * Second item
//   * Third item

// Numbered list
print(cli.numberedList(items))
//   1. First item
//   2. Second item
//   3. Third item
```

## Tree Structure

```javascript
tree = [
    {name: "src", children: [
        {name: "main.jm", children: []},
        {name: "utils", children: [
            {name: "helpers.jm", children: []}
        ]}
    ]},
    {name: "README.md", children: []}
]

print(cli.tree(tree))
// |-- src
// |   |-- main.jm
// |   `-- utils
// |       `-- helpers.jm
// `-- README.md
```

## Logging

```javascript
cli.logDebug("Debugging info")   // [DEBUG] Debugging info
cli.logInfo("Information")       // [INFO] Information
cli.logWarn("Warning message")   // [WARN] Warning message
cli.logError("Error occurred")   // [ERROR] Error occurred
cli.logSuccess("Done!")          // [OK] Done!
```

## Horizontal Rules

```javascript
cli.hr()           // ----------------------------------------
cli.hr(20)         // --------------------
cli.hr(20, "=")    // ====================
cli.hrDouble(20)   // ====================
cli.hrDashed(20)   // --------------------
cli.hrDotted(20)   // ....................
```

## Example: CLI Application

```javascript
import "lib/cli.jm" as cli

print(cli.box("My CLI App v1.0", "double", 1))
print("")

// Parse arguments
args = ["--verbose", "-o", "output.txt", "input.txt"]
parsed = cli.parseArgs(args, null)

if (cli.hasFlag(parsed, "verbose")) {
    cli.logDebug("Verbose mode enabled")
}

// Show progress
pb = cli.ProgressBar(100, 30)
cli.progressLabel(pb, "Processing...")

for i in range(101) {
    cli.progressUpdate(pb, i)
    // In real app, do work here
}
cli.progressComplete(pb)

cli.printSuccess("Done!")
```
