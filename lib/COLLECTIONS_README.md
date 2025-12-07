# Collections - Jeem Data Structures Library

Advanced data structures for the Jeem programming language.

## Installation

Copy `collections.jm` to your project's modules folder.

## Usage

```javascript
import "modules/collections.jm" as col
```

## Stack (LIFO)

Last In, First Out data structure.

```javascript
s = col.Stack()

col.stackPush(s, "a")
col.stackPush(s, "b")
col.stackPush(s, "c")

col.stackPeek(s)      // "c" (view top without removing)
col.stackPop(s)       // "c" (remove and return top)
col.stackSize(s)      // 2
col.stackEmpty(s)     // false
col.stackClear(s)     // clear all items
col.stackToArray(s)   // convert to array (top first)
```

## Queue (FIFO)

First In, First Out data structure.

```javascript
q = col.Queue()

col.enqueue(q, "first")
col.enqueue(q, "second")
col.enqueue(q, "third")

col.queuePeek(q)      // "first" (view front)
col.dequeue(q)        // "first" (remove and return front)
col.queueSize(q)      // 2
col.queueEmpty(q)     // false
col.queueClear(q)     // clear all items
col.queueToArray(q)   // convert to array
```

## Deque (Double-Ended Queue)

Queue that allows insertion/removal at both ends.

```javascript
d = col.Deque()

col.dequePushFront(d, 1)  // add to front
col.dequePushBack(d, 2)   // add to back
col.dequePopFront(d)      // remove from front
col.dequePopBack(d)       // remove from back
col.dequePeekFront(d)     // view front
col.dequePeekBack(d)      // view back
col.dequeSize(d)
col.dequeEmpty(d)
```

## Set

Collection of unique values.

```javascript
s = col.Set()
s = col.SetFrom([1, 2, 3])  // create from array

col.setAdd(s, 4)
col.setRemove(s, 2)
col.setHas(s, 3)        // true
col.setSize(s)          // 3
col.setEmpty(s)         // false
col.setClear(s)
col.setToArray(s)       // convert to array
```

### Set Operations

```javascript
set1 = col.SetFrom([1, 2, 3])
set2 = col.SetFrom([2, 3, 4, 5])

col.setUnion(set1, set2)         // {1, 2, 3, 4, 5}
col.setIntersection(set1, set2)  // {2, 3}
col.setDifference(set1, set2)    // {1}
col.setSymmetricDiff(set1, set2) // {1, 4, 5}

col.setIsSubset(set1, set2)      // false
col.setIsSuperset(set1, set2)    // false
col.setEquals(set1, set2)        // false
```

## Map (Ordered)

Key-value pairs that maintain insertion order.

```javascript
m = col.Map()

col.mapSet(m, "name", "Alice")
col.mapSet(m, "age", 30)

col.mapGet(m, "name")     // "Alice"
col.mapHas(m, "age")      // true
col.mapDelete(m, "age")
col.mapSize(m)            // 1

col.mapKeys(m)            // ["name"]
col.mapValues(m)          // ["Alice"]
col.mapEntries(m)         // [["name", "Alice"]]
col.mapClear(m)
```

## LinkedList

Doubly-linked list for efficient insertions/deletions.

```javascript
list = col.LinkedList()
list = col.ListFrom([1, 2, 3])  // create from array

col.listAppend(list, 4)    // add to end
col.listPrepend(list, 0)   // add to front

col.listFirst(list)        // view first
col.listLast(list)         // view last
col.listGet(list, 2)       // get by index

col.listShift(list)        // remove from front
col.listPop(list)          // remove from back

col.listContains(list, 2)  // true
col.listSize(list)
col.listEmpty(list)
col.listToArray(list)
col.listReverse(list)      // reverse in place
```

## Priority Queue

Heap-based priority queue.

```javascript
// Min-heap (lowest priority first)
pq = col.PriorityQueue(false)

// Max-heap (highest priority first)
pq = col.PriorityQueue(true)

col.pqEnqueue(pq, "task1", 5)   // value, priority
col.pqEnqueue(pq, "task2", 1)
col.pqEnqueue(pq, "task3", 10)

col.pqPeek(pq)      // view highest priority
col.pqDequeue(pq)   // remove and return highest priority
col.pqSize(pq)
col.pqEmpty(pq)
```

### Example: Task Scheduler

```javascript
pq = col.PriorityQueue(false)  // min-heap

col.pqEnqueue(pq, "Critical bug fix", 1)
col.pqEnqueue(pq, "New feature", 5)
col.pqEnqueue(pq, "Documentation", 10)

while (!col.pqEmpty(pq)) {
    task = col.pqDequeue(pq)
    print("Processing:", task)
}
// Processing: Critical bug fix
// Processing: New feature
// Processing: Documentation
```

## Ring Buffer

Fixed-size circular buffer.

```javascript
rb = col.RingBuffer(3)  // capacity 3

col.rbWrite(rb, "a")
col.rbWrite(rb, "b")
col.rbWrite(rb, "c")
col.rbWrite(rb, "d")  // overwrites "a"

col.rbToArray(rb)     // ["b", "c", "d"]
col.rbRead(rb)        // "b"
col.rbPeek(rb)        // view next without removing
col.rbSize(rb)
col.rbEmpty(rb)
col.rbFull(rb)
```

## Trie (Prefix Tree)

Efficient string prefix search.

```javascript
trie = col.Trie()

col.trieInsert(trie, "hello", 1)
col.trieInsert(trie, "help", 2)
col.trieInsert(trie, "world", 3)

col.trieHas(trie, "hello")       // true
col.trieHas(trie, "hel")         // false (not complete word)
col.trieHasPrefix(trie, "hel")   // true
col.trieSearch(trie, "help")     // 2 (returns value)

col.trieWithPrefix(trie, "hel")  // ["hello", "help"]
col.trieWithPrefix(trie, "wo")   // ["world"]
```

### Example: Autocomplete

```javascript
trie = col.Trie()
words = ["apple", "application", "apply", "banana", "band"]
for word in words {
    col.trieInsert(trie, word, true)
}

// User types "app"
suggestions = col.trieWithPrefix(trie, "app")
// ["apple", "application", "apply"]
```

## Counter

Count occurrences of items.

```javascript
c = col.Counter()
c = col.CounterFrom(["a", "b", "a", "c", "a", "b"])

col.counterGet(c, "a")      // 3
col.counterGet(c, "d")      // 0 (not found)
col.counterAdd(c, "a")      // increment by 1
col.counterAdd(c, "d", 5)   // add with count

col.counterTotal(c)         // total of all counts
col.counterItems(c)         // all unique items
col.counterMostCommon(c, 2) // [["a", 4], ["b", 2]]
```

## DefaultDict

Dictionary with default values for missing keys.

```javascript
// Default value 0
dd = col.DefaultDict(0)
col.ddGet(dd, "missing")  // 0

col.ddSet(dd, "key", 10)
col.ddGet(dd, "key")      // 10
col.ddHas(dd, "key")      // true

// Default value as empty array
dd = col.DefaultDict([])
arr = col.ddGetOrCreate(dd, "items")
push(arr, "value")
```

### Example: Grouping

```javascript
dd = col.DefaultDict([])

data = [
    {name: "Alice", dept: "Engineering"},
    {name: "Bob", dept: "Sales"},
    {name: "Charlie", dept: "Engineering"}
]

for item in data {
    arr = col.ddGetOrCreate(dd, item.dept)
    push(arr, item.name)
}

col.ddGet(dd, "Engineering")  // ["Alice", "Charlie"]
col.ddGet(dd, "Sales")        // ["Bob"]
```

## Sorted List

List that maintains sorted order.

```javascript
// Ascending order
sl = col.SortedList(false)

// Descending order
sl = col.SortedList(true)

col.slInsert(sl, 5)
col.slInsert(sl, 2)
col.slInsert(sl, 8)
col.slInsert(sl, 1)

col.slToArray(sl)      // [1, 2, 5, 8] (ascending)
col.slMin(sl)          // 1
col.slMax(sl)          // 8
col.slGet(sl, 0)       // get by index
col.slContains(sl, 5)  // true
col.slRemove(sl, 5)    // remove first occurrence
col.slSize(sl)
```

## Type Checking

```javascript
col.isStack(obj)
col.isQueue(obj)
col.isSet(obj)
col.isMap(obj)
col.isLinkedList(obj)
col.isPriorityQueue(obj)
col.isTrie(obj)
col.isCounter(obj)
```

## Complete Example

```javascript
import "modules/collections.jm" as col

// Task management system
tasks = col.PriorityQueue(false)  // min-heap for priorities
completed = col.Set()
history = col.Queue()

// Add tasks
col.pqEnqueue(tasks, "Fix critical bug", 1)
col.pqEnqueue(tasks, "Review PR", 3)
col.pqEnqueue(tasks, "Write docs", 5)

// Process tasks
while (!col.pqEmpty(tasks)) {
    task = col.pqDequeue(tasks)
    print("Working on:", task)
    
    col.setAdd(completed, task)
    col.enqueue(history, task)
}

// Check completed
print("Completed:", col.setSize(completed), "tasks")
print("History:", col.queueToArray(history))
```
