# DateTime - Jeem Date/Time Library

A comprehensive date and time library for the Jeem programming language.

## Installation

Copy `datetime.jm` to your project's modules folder.

## Usage

```javascript
import "modules/datetime.jm" as dt
```

## DateTime Creation

```javascript
// Full DateTime
d = dt.DateTime(2024, 12, 25, 10, 30, 45, 0)

// Date only (time defaults to midnight)
d = dt.Date(2024, 1, 15)

// Time only (date defaults to 1970-01-01)
t = dt.Time(14, 30, 0, 0)

// From Unix timestamp
d = dt.fromTimestamp(1704067200)
```

## Parsing

```javascript
// ISO 8601 format
d = dt.parseISO("2024-06-15T14:30:00")

// Custom format
d = dt.parse("25/12/2024", "%d/%m/%Y")
d = dt.parse("2024-01-15 10:30", "%Y-%m-%d %H:%M")

// Auto-detect common formats
d = dt.parseAuto("2024-03-20")      // YYYY-MM-DD
d = dt.parseAuto("15/07/2024")      // DD/MM/YYYY
d = dt.parseAuto("20.06.2024")      // DD.MM.YYYY
```

### Format Specifiers

| Specifier | Description | Example |
|-----------|-------------|---------|
| `%Y` | 4-digit year | 2024 |
| `%y` | 2-digit year | 24 |
| `%m` | Month (01-12) | 06 |
| `%d` | Day (01-31) | 15 |
| `%H` | Hour 24h (00-23) | 14 |
| `%I` | Hour 12h (01-12) | 02 |
| `%M` | Minute (00-59) | 30 |
| `%S` | Second (00-59) | 45 |
| `%p` | AM/PM | PM |
| `%B` | Full month name | January |
| `%b` | Short month name | Jan |
| `%A` | Full weekday name | Monday |
| `%a` | Short weekday name | Mon |
| `%j` | Day of year | 186 |
| `%W` | Week of year | 27 |

## Formatting

```javascript
d = dt.DateTime(2024, 7, 4, 15, 30, 0, 0)

dt.toISO(d)           // "2024-07-04T15:30:00"
dt.toISODate(d)       // "2024-07-04"
dt.toISOTime(d)       // "15:30:00"
dt.toHuman(d)         // "Thursday, July 4, 2024"

// Custom format
dt.format(d, "%Y-%m-%d %H:%M")   // "2024-07-04 15:30"
dt.format(d, "%d.%m.%Y")         // "04.07.2024"
dt.format(d, "%B %d, %Y")        // "July 04, 2024"
dt.format(d, "%I:%M %p")         // "03:30 PM"
```

## Date Components

```javascript
d = dt.DateTime(2024, 7, 4, 15, 30, 0, 0)

dt.dayOfWeek(d)     // 4 (0=Sunday, 6=Saturday)
dt.dayOfYear(d)     // 186
dt.weekOfYear(d)    // 27
dt.quarter(d)       // 3

dt.isWeekend(d)     // false
dt.isWeekday(d)     // true
dt.isLeapYear(2024) // true
dt.daysInMonth(2024, 2)  // 29
dt.daysInYear(2024)      // 366
```

## Date Arithmetic

```javascript
base = dt.DateTime(2024, 1, 15, 12, 0, 0, 0)

// Add
dt.addYears(base, 1)     // 2025-01-15
dt.addMonths(base, 2)    // 2024-03-15
dt.addDays(base, 10)     // 2024-01-25
dt.addHours(base, 5)     // 2024-01-15T17:00:00
dt.addMinutes(base, 45)  // 2024-01-15T12:45:00
dt.addSeconds(base, 30)  // 2024-01-15T12:00:30

// Subtract
dt.subYears(base, 1)
dt.subMonths(base, 1)
dt.subDays(base, 5)
dt.subHours(base, 2)
dt.subMinutes(base, 15)
dt.subSeconds(base, 10)
```

## Comparison

```javascript
d1 = dt.DateTime(2024, 1, 15, 10, 0, 0, 0)
d2 = dt.DateTime(2024, 1, 20, 10, 0, 0, 0)

dt.compare(d1, d2)      // -1 (d1 < d2), 0 (equal), 1 (d1 > d2)
dt.isEqual(d1, d2)      // false
dt.isBefore(d1, d2)     // true
dt.isAfter(d1, d2)      // false

dt.isSameDay(d1, d2)    // false
dt.isSameMonth(d1, d2)  // true
dt.isSameYear(d1, d2)   // true

dt.isBetween(mid, d1, d2)  // true if mid is between d1 and d2
```

## Differences

```javascript
start = dt.DateTime(2024, 1, 1, 0, 0, 0, 0)
end = dt.DateTime(2024, 1, 15, 12, 30, 0, 0)

dt.diffSeconds(end, start)  // 1255800
dt.diffMinutes(end, start)  // 20930
dt.diffHours(end, start)    // 348.5
dt.diffDays(end, start)     // 14.52
dt.diffWeeks(end, start)    // 2.07
dt.diffMonths(end, start)   // 0
dt.diffYears(end, start)    // 0

// Detailed difference
dt.diff(end, start)
// {years: 0, months: 0, days: 14, hours: 12, minutes: 30, seconds: 0}
```

## Start/End of Period

```javascript
d = dt.DateTime(2024, 6, 15, 14, 30, 0, 0)

dt.startOfDay(d)      // 2024-06-15T00:00:00
dt.endOfDay(d)        // 2024-06-15T23:59:59

dt.startOfMonth(d)    // 2024-06-01T00:00:00
dt.endOfMonth(d)      // 2024-06-30T23:59:59

dt.startOfYear(d)     // 2024-01-01T00:00:00
dt.endOfYear(d)       // 2024-12-31T23:59:59

dt.startOfWeek(d)     // Start of week (Sunday)
dt.endOfWeek(d)       // End of week (Saturday)

dt.startOfQuarter(d)  // 2024-04-01T00:00:00
dt.endOfQuarter(d)    // 2024-06-30T23:59:59
```

## Duration

```javascript
// Create duration
dur = dt.Duration(2, 5, 30, 15)  // 2 days, 5 hours, 30 min, 15 sec

// Format
dt.formatDuration(dur)      // "2d 5h 30m 15s"

// Convert to/from seconds
dt.durationToSeconds(dur)   // 192615
dt.durationFromSeconds(90061)  // {days: 1, hours: 1, minutes: 1, seconds: 1}

// Add duration to date
dt.addDuration(base, dur)
```

## Relative Time

```javascript
now = dt.DateTime(2024, 6, 15, 12, 0, 0, 0)
past = dt.subHours(now, 5)
future = dt.addDays(now, 3)

dt.toRelative(past, now)    // "5 hours ago"
dt.toRelative(future, now)  // "in 3 days"
```

## Calendar

```javascript
// Generate calendar array for month
cal = dt.calendar(2024, 12)
// Returns array of weeks, each week is array of days (null for empty)

// Print formatted calendar
dt.printCalendar(2024, 12)
//     December 2024
//  Sun Mon Tue Wed Thu Fri Sat
//   1   2   3   4   5   6   7 
//   8   9  10  11  12  13  14 
//  ...
```

## Timestamp Conversion

```javascript
// DateTime to Unix timestamp
ts = dt.toTimestamp(d)

// Unix timestamp to DateTime
d = dt.fromTimestamp(1704067200)
```

## Constants

```javascript
dt.MONTHS()       // ["January", "February", ...]
dt.MONTHS_SHORT() // ["Jan", "Feb", ...]
dt.DAYS()         // ["Sunday", "Monday", ...]
dt.DAYS_SHORT()   // ["Sun", "Mon", ...]
dt.DAYS_IN_MONTH() // [31, 28, 31, 30, ...]
```

## Validation

```javascript
dt.isValid(d)  // true if valid date/time values
```

## Utility

```javascript
dt.clone(d)     // Create copy of DateTime
dt.dtStr(d)     // Same as toISO()
dt.printDT(d)   // Print human-readable date and time
```
