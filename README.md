# Jeem Programming Language

A lightweight, scripting language written in C.

## Features

- **Modern Syntax**: Arrow functions, template strings, classes
- **Functional Programming**: map, filter, reduce, closures
- **HTTP Support**: Built-in HTTP client and server
- **File I/O**: Read, write, append files
- **JSON**: Parse and stringify JSON data
- **Timers**: setTimeout, setInterval
- **Cross-Platform**: Windows, Linux, macOS

## Installation

### From Source

```bash
# Linux/macOS
gcc -O2 -o jeem jeem.c -lm

# Windows (MinGW)
gcc -O2 -o jeem.exe jeem.c -lm -lws2_32
```

### Add to PATH (Optional)

**Windows:**

```cmd
set PATH=%PATH%;C:\path\to\jeem
```

**Linux/macOS:**

```bash
export PATH=$PATH:/path/to/jeem
```

## Quick Start

### Hello World

```javascript
print("Hello, World!");
```

### Run a Script

```bash
jeem hello.jm
```

### Run Code Directly

```bash
jeem -e 'print("Hello!")'
```

## Command Line

```
jeem <file.jm>        Run a script file
jeem -e "code"        Execute code directly
jeem init [name]      Initialize a new project
jeem start            Run scripts.start from jeem.json
jeem test             Run scripts.test from jeem.json
jeem run <script>     Run custom script from jeem.json
jeem -v, --version    Show version
jeem -h, --help       Show help
```

## Project Structure

```bash
jeem init myproject
```

Creates:

```
myproject/
├── jeem.json
├── main.jm
└── test.jm
```

**jeem.json:**

```json
{
  "name": "myproject",
  "version": "1.0.0",
  "main": "main.jm",
  "scripts": {
    "start": "main.jm",
    "test": "test.jm",
    "build": "build.jm"
  }
}
```

## Language Reference

### Variables

```javascript
// Variables
name = "Alice";
age = 30;
pi = 3.14159;

// Constants (cannot be reassigned)
const MAX_SIZE = 100;
```

### Data Types

```javascript
// Numbers
integer = 42;
float = 3.14;

// Strings
str = "Hello, World!";
str2 = "Single quotes work too";

// Booleans
yes = true;
no = false;

// Null
empty = null;

// Arrays
arr = [1, 2, 3, 4, 5];
mixed = [1, "two", true, null];

// Objects
obj = { name: "Alice", age: 30 };
nested = {
  user: { name: "Bob" },
  scores: [90, 85, 92],
};
```

### Operators

```javascript
// Arithmetic
a + b; // Addition
a - b; // Subtraction
a * b; // Multiplication
a / b; // Division
a % b; // Modulo
a ** b; // Power

// Comparison
a == b; // Equal
a != b; // Not equal
a < b; // Less than
a > b; // Greater than
a <= b; // Less or equal
a >= b; // Greater or equal

// Logical
a && b; // AND
a || b; // OR
!a; // NOT

// Assignment
x = 10;
x += 5; // x = x + 5
x -= 3; // x = x - 3
x *= 2; // x = x * 2
x /= 4; // x = x / 4
x++; // Increment
x--; // Decrement

// Ternary
result = condition ? "yes" : "no";
```

### Control Flow

```javascript
// If/Else
if (x > 10) {
    print("big")
} else if (x > 5) {
    print("medium")
} else {
    print("small")
}

// While Loop
i = 0
while (i < 5) {
    print(i)
    i++
}

// For-In Loop (Arrays)
arr = [1, 2, 3]
for item in arr {
    print(item)
}

// For-In with Index
for item, index in arr {
    print(index, ":", item)
}

// For-In Loop (Range)
for i in range(5) {
    print(i)  // 0, 1, 2, 3, 4
}

for i in range(2, 6) {
    print(i)  // 2, 3, 4, 5
}

for i in range(0, 10, 2) {
    print(i)  // 0, 2, 4, 6, 8
}

// C-Style For Loop
for i = 0; i < 5; i++ {
    print(i)
}

// Break and Continue
for i in range(10) {
    if (i == 3) continue
    if (i == 7) break
    print(i)
}

// Case Statement (Switch)
x = 3
case(x) {
    1:
        print("one")
        break
    2..4:                    // Range: matches 2, 3, or 4
        print("two to four")
        break
    5, 7, 9:                 // Multiple values: matches 5, 7, or 9
        print("five, seven, or nine")
        break
    default:
        print("other")
}

// Case with Strings
day = "Monday"
case(day) {
    "Monday", "Tuesday", "Wednesday", "Thursday", "Friday":
        print("Weekday")
        break
    "Saturday", "Sunday":
        print("Weekend")
        break
}

// Case for Grades
grade = 85
case(grade) {
    90..100:
        print("A")
        break
    80..89:
        print("B")
        break
    70..79:
        print("C")
        break
    default:
        print("F")
}
```

### Functions

```javascript
// Basic Function
func greet(name) {
    return "Hello, " + name + "!"
}
print(greet("World"))

// Multiple Parameters
func add(a, b) {
    return a + b
}

// No Return Value
func sayHi(name) {
    print("Hi, " + name + "!")
}

// Recursive Function
func factorial(n) {
    if (n <= 1) return 1
    return n * factorial(n - 1)
}
```

### Arrow Functions

```javascript
// Single Parameter
double = (x) => x * 2;
print(double(5)); // 10

// Multiple Parameters
add = (a, b) => a + b;
print(add(3, 4)); // 7

// No Parameters
getPI = () => 3.14159;

// Block Body
greet = (name) => {
  msg = "Hello, " + name + "!";
  return msg;
};

// In Array Methods
nums = [1, 2, 3, 4, 5];
doubled = nums.map((x) => x * 2);
evens = nums.filter((x) => x % 2 == 0);
sum = nums.reduce((a, b) => a + b, 0);
```

### Closures

```javascript
func makeCounter(start) {
    count = start
    return () => {
        count = count + 1
        return count
    }
}

func makeMultiplier(factor) {
    return x => x * factor
}

triple = makeMultiplier(3)
print(triple(7))  // 21
```

### Template Strings

```javascript
name = "Alice";
age = 30;

// Using {{expression}}
print("Hello, {{name}}!");
print("Age: {{age}}, Next year: {{age + 1}}");

// Complex expressions
items = ["a", "b", "c"];
print("Items: {{join(items, ', ')}}");
```

### Classes

```javascript
class Person {
  init(name, age) {
    this.name = name;
    this.age = age;
  }

  greet() {
    return "Hello, I'm " + this.name;
  }

  haveBirthday() {
    this.age = this.age + 1;
  }
}

alice = new Person("Alice", 30);
print(alice.greet());
alice.haveBirthday();
print(alice.age); // 31

// Inheritance
class Employee extends Person {
  init(name, age, role) {
    this.name = name;
    this.age = age;
    this.role = role;
  }

  greet() {
    return "Hi, I'm " + this.name + ", " + this.role;
  }
}

bob = new Employee("Bob", 25, "Developer");
print(bob.greet());
```

### Array Methods

```javascript
arr = [1, 2, 3, 4, 5];

// Length
arr.len(); // 5

// Transform
arr.map((x) => x * 2); // [2, 4, 6, 8, 10]

// Filter
arr.filter((x) => x > 2); // [3, 4, 5]

// Reduce
arr.reduce((a, b) => a + b, 0); // 15

// Iterate
arr.forEach((x) => print(x));

// Find
arr.find((x) => x > 3); // 4
arr.findIndex((x) => x > 3); // 3

// Test
arr.every((x) => x > 0); // true
arr.some((x) => x > 4); // true
arr.includes(3); // true

// Modify
arr.push(6); // Add to end
arr.pop(); // Remove from end

// Other
arr.slice(1, 3); // [2, 3]
arr.join(", "); // "1, 2, 3, 4, 5"
arr.reverse(); // Reverse in place
sort(arr); // Sort in place

// Chaining
result = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
  .filter((x) => x % 2 == 0)
  .map((x) => x * x);
// [4, 16, 36, 64, 100]
```

### String Methods

```javascript
s = "Hello, World!";

// Length
s.len(); // 13

// Case
s.upper(); // "HELLO, WORLD!"
s.lower(); // "hello, world!"

// Trim
"  hello  ".trim(); // "hello"

// Split/Join
"a,b,c"
  .split(",") // ["a", "b", "c"]
  [("a", "b", "c")].join("-"); // "a-b-c"

// Search
s.indexOf("World"); // 7
s.includes("Hello"); // true
s.startsWith("Hello"); // true
s.endsWith("!"); // true

// Extract
s.slice(0, 5); // "Hello"
s.charAt(0); // "H"
s.substring(0, 5); // "Hello"

// Replace
s.replace("World", "Jeem"); // "Hello, Jeem!"
```

### Object Methods

```javascript
obj = {name: "Alice", age: 30, city: "Berlin"}

// Length
obj.len()                    // 3

// Keys and Values
obj.keys()                   // ["name", "age", "city"]
obj.values()                 // ["Alice", 30, "Berlin"]
obj.entries()                // [["name","Alice"], ["age",30], ...]

// Check Property
obj.hasOwnProperty("name")   // true
hasKey(obj, "name")          // true

// Iterate
for key in obj {
    print(key, ":", obj[key])
}
```

### Import/Export

```javascript
// math_utils.jm
func square(x) {
    return x * x
}

func cube(x) {
    return x * x * x
}

// main.jm
import "math_utils.jm" as math

print(math.square(5))  // 25
print(math.cube(3))    // 27
```

## Built-in Functions

### Type Functions

| Function      | Description        |
| ------------- | ------------------ |
| `typeof(x)`   | Get type as string |
| `isNumber(x)` | Check if number    |
| `isString(x)` | Check if string    |
| `isArray(x)`  | Check if array     |
| `isObject(x)` | Check if object    |
| `isFunc(x)`   | Check if function  |

### Conversion Functions

| Function   | Description                                         |
| ---------- | --------------------------------------------------- |
| `int(x)`   | Convert to integer                                  |
| `float(x)` | Convert to float                                    |
| `str(x)`   | Convert to string                                   |
| `chr(n)`   | Character from code point (e.g., `chr(65)` → `"A"`) |
| `ord(s)`   | Code point from character (e.g., `ord("A")` → `65`) |

### Math Functions

| Function              | Description             |
| --------------------- | ----------------------- |
| `abs(x)`              | Absolute value          |
| `floor(x)`            | Round down              |
| `ceil(x)`             | Round up                |
| `round(x)`            | Round to nearest        |
| `sqrt(x)`             | Square root             |
| `pow(x, y)`           | Power                   |
| `min(a, b, ...)`      | Minimum value           |
| `max(a, b, ...)`      | Maximum value           |
| `random()`            | Random float 0-1        |
| `randomInt(max)`      | Random int 0 to max-1   |
| `randomInt(min, max)` | Random int min to max-1 |

### String Functions

| Function               | Description          |
| ---------------------- | -------------------- |
| `split(s, sep)`        | Split to array       |
| `join(arr, sep)`       | Join array to string |
| `replace(s, old, new)` | Replace substring    |
| `indexOf(s, sub)`      | Find position        |
| `slice(s, start, end)` | Extract substring    |

### Array Functions

| Function                  | Description             |
| ------------------------- | ----------------------- |
| `push(arr, item)`         | Add to end              |
| `pop(arr)`                | Remove from end         |
| `slice(arr, start, end)`  | Extract portion         |
| `indexOf(arr, item)`      | Find position           |
| `reverse(arr)`            | Reverse in place        |
| `sort(arr)`               | Sort in place           |
| `range(n)`                | Generate [0..n-1]       |
| `range(start, end)`       | Generate [start..end-1] |
| `range(start, end, step)` | Generate with step      |

### Object Functions

| Function           | Description         |
| ------------------ | ------------------- |
| `keys(obj)`        | Get keys array      |
| `values(obj)`      | Get values array    |
| `hasKey(obj, key)` | Check if key exists |
| `delete(obj, key)` | Remove key          |
| `clone(obj)`       | Deep copy           |

### I/O Functions

| Function        | Description      |
| --------------- | ---------------- |
| `print(...)`    | Print to console |
| `input(prompt)` | Read user input  |

### File Functions

| Function                 | Description          |
| ------------------------ | -------------------- |
| `readFile(path)`         | Read file contents   |
| `writeFile(path, data)`  | Write to file        |
| `appendFile(path, data)` | Append to file       |
| `fileExists(path)`       | Check if file exists |
| `remove(path)`           | Delete file          |
| `mkdir(path)`            | Create directory     |
| `rename(old, new)`       | Rename file          |

### JSON Functions

| Function                     | Description       |
| ---------------------------- | ----------------- |
| `jsonParse(str)`             | Parse JSON string |
| `jsonStringify(obj)`         | Convert to JSON   |
| `jsonStringify(obj, indent)` | Pretty JSON       |

### Time Functions

| Function    | Description                 |
| ----------- | --------------------------- |
| `time()`    | Unix timestamp (seconds)    |
| `now()`     | Current time (milliseconds) |
| `sleep(ms)` | Pause execution             |

### Timer Functions

```javascript
// setTimeout - runs once after delay
id = setTimeout(() => {
  print("Runs after 1 second");
}, 1000);

// setInterval - runs repeatedly
id = setInterval(() => {
  print("Runs every 500ms");
}, 500);

// Clear timers
clearTimeout(id);
clearInterval(id);
```

Timers run automatically at end of script (like JavaScript).

### HTTP Client

```javascript
// GET Request
response = httpGet("http://api.example.com/data");
if (response.status == 200) {
  data = jsonParse(response.body);
  print(data);
}

// POST Request
body = jsonStringify({ name: "Alice", age: 30 });
response = httpPost("http://api.example.com/users", body, "application/json");
print(response.status);

// PUT Request (Update)
body = jsonStringify({ name: "Alice", age: 31 });
response = httpPut("http://api.example.com/users/1", body);
print(response.status);

// DELETE Request
response = httpDelete("http://api.example.com/users/1");
print(response.status);

// PATCH Request (Partial Update)
body = jsonStringify({ age: 32 });
response = httpPatch("http://api.example.com/users/1", body);
print(response.status);

// Generic HTTP Request
response = http("GET", "http://api.example.com/data");
response = http(
  "POST",
  "http://api.example.com/data",
  body,
  "application/json"
);
```

**HTTP Methods Summary:**

| Method | Client Function                 | Server Route                           |
| ------ | ------------------------------- | -------------------------------------- |
| GET    | `httpGet(url)`                  | `serverRoute("GET", path, handler)`    |
| POST   | `httpPost(url, body, type)`     | `serverRoute("POST", path, handler)`   |
| PUT    | `httpPut(url, body, type)`      | `serverRoute("PUT", path, handler)`    |
| DELETE | `httpDelete(url)`               | `serverRoute("DELETE", path, handler)` |
| PATCH  | `httpPatch(url, body, type)`    | `serverRoute("PATCH", path, handler)`  |
| Any    | `http(method, url, body, type)` | `serverRoute(method, path, handler)`   |

### HTTP Server

```javascript
// Create server
server = createServer(8080);

// Define routes
serverRoute("GET", "/", (req) => {
  return {
    status: 200,
    contentType: "text/html",
    body: "<h1>Hello, World!</h1>",
  };
});

serverRoute("GET", "/api/data", (req) => {
  return {
    status: 200,
    contentType: "application/json",
    body: jsonStringify({ message: "Hello!" }),
  };
});

serverRoute("POST", "/api/echo", (req) => {
  return {
    status: 200,
    contentType: "application/json",
    body: jsonStringify({ received: req.body }),
  };
});

// Start listening (blocking, but processes timers)
print("Server running on http://localhost:8080");
serverListen();
```

**Non-blocking server with serverHandle():**

```javascript
server = createServer(8080);

serverRoute("GET", "/", (req) => {
  return { status: 200, contentType: "text/html", body: "<h1>Hello!</h1>" };
});

// Handle requests manually (non-blocking)
print("Server started on port 8080");
while (true) {
  serverHandle(100); // Handle one request, 100ms timeout
  // Do other work here...
}
```

### Utility Functions

| Function                     | Description                            |
| ---------------------------- | -------------------------------------- |
| `assert(condition, message)` | Assert condition                       |
| `serverHandle(timeout)`      | Handle one HTTP request (non-blocking) |

### Global Constants

| Constant  | Value                  |
| --------- | ---------------------- |
| `PI`      | 3.14159265358979       |
| `E`       | 2.71828182845904       |
| `VERSION` | "3.0"                  |
| `ARGS`    | Command line arguments |

## Examples

### FizzBuzz

```javascript
for i in range(1, 101) {
    if (i % 15 == 0) {
        print("FizzBuzz")
    } else if (i % 3 == 0) {
        print("Fizz")
    } else if (i % 5 == 0) {
        print("Buzz")
    } else {
        print(i)
    }
}
```

### Read and Process CSV

```javascript
content = readFile("data.csv")
lines = split(content, "\n")

for line, i in lines {
    if (i == 0) continue  // Skip header
    cols = split(line, ",")
    print("Name:", cols[0], "Age:", cols[1])
}
```

### Simple REST API

```javascript
server = createServer(8080);

users = [
  { id: 1, name: "Alice" },
  { id: 2, name: "Bob" },
];

serverRoute("GET", "/users", (req) => {
  return {
    status: 200,
    contentType: "application/json",
    body: jsonStringify(users),
  };
});

serverRoute("POST", "/users", (req) => {
  user = jsonParse(req.body);
  user.id = users.len() + 1;
  push(users, user);
  return {
    status: 201,
    contentType: "application/json",
    body: jsonStringify(user),
  };
});

print("API running on http://localhost:8080");
serverListen();
```

### Timer Example

```javascript
print("Starting...");

// Run after 1 second
setTimeout(() => {
  print("1 second passed!");
}, 1000);

// Run every 500ms
count = 0;
id = setInterval(() => {
  count++;
  print("Tick", count);
  if (count >= 5) {
    clearInterval(id);
    print("Done!");
  }
}, 500);

print("Timers scheduled, waiting...");
// Timers run automatically
```

### Data Processing Pipeline

```javascript
people = [
  { name: "Alice", age: 30, dept: "Engineering" },
  { name: "Bob", age: 25, dept: "Marketing" },
  { name: "Charlie", age: 35, dept: "Engineering" },
  { name: "Diana", age: 28, dept: "Engineering" },
];

// Find senior engineers
seniorEngineers = people
  .filter((p) => p.dept == "Engineering")
  .filter((p) => p.age >= 30)
  .map((p) => p.name);

print("Senior Engineers:", seniorEngineers);
// ["Alice", "Charlie"]

// Calculate average age
totalAge = people.reduce((sum, p) => sum + p.age, 0);
avgAge = totalAge / people.len();
print("Average age:", avgAge);
```

## Testing

Run the test suite:

```bash
jeem test_runner.jm
```

Individual tests:

```bash
jeem test_basics.jm
jeem test_arrays.jm
jeem test_functions.jm
jeem test_classes.jm
jeem test_files.jm
jeem test_timers.jm
jeem test_http_server.jm
```

## License

MIT License

## Version

Jeem v3.0
