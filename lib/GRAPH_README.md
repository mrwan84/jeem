# Graph - Jeem Graph Library

Graph data structure with BFS, DFS, Dijkstra, and more.

## Installation

Copy `graph.jm` to your project's lib folder.

## Usage

```javascript
import "lib/graph.jm" as graph
```

## Graph Creation

```javascript
// Undirected graph
g = graph.Graph()

// Directed graph
g = graph.DiGraph()

// Weighted graph
g = graph.WeightedGraph()

// Weighted directed graph
g = graph.WeightedDiGraph()

// From adjacency list
g = graph.fromAdjacencyList({
    "A": ["B", "C"],
    "B": ["A", "D"],
    "C": ["A", "D"],
    "D": ["B", "C"]
})

// From edge list
g = graph.fromEdgeList([
    ["A", "B", 5],   // [from, to, weight]
    ["B", "C", 3],
    ["A", "C", 10]
], true)  // true = directed
```

## Node Operations

```javascript
// Add nodes
graph.addNode(g, "A", null)
graph.addNode(g, "B", {name: "Node B"})  // With data

// Remove node
graph.removeNode(g, "A")

// Get node
node = graph.getNode(g, "A")

// Check if node exists
graph.hasNode(g, "A")  // true/false

// Get all nodes
nodes = graph.getNodes(g)  // ["A", "B", "C"]

// Node count
graph.nodeCount(g)

// Node data
graph.setNodeData(g, "A", {visited: true})
data = graph.getNodeData(g, "A")
```

## Edge Operations

```javascript
// Add edges
graph.addEdge(g, "A", "B")        // Unweighted
graph.addEdge(g, "A", "B", 5)     // Weighted

// Remove edge
graph.removeEdge(g, "A", "B")

// Check if edge exists
graph.hasEdge(g, "A", "B")

// Get/set weight
w = graph.getWeight(g, "A", "B")
graph.setWeight(g, "A", "B", 10)

// Get all edges
edges = graph.getEdges(g)  // [{from, to, weight}, ...]

// Edge count
graph.edgeCount(g)

// Get neighbors
neighbors = graph.neighbors(g, "A")  // ["B", "C"]

// Degree (number of edges)
graph.degree(g, "A")      // Total degree
graph.inDegree(g, "A")    // Incoming (directed)
graph.outDegree(g, "A")   // Outgoing (directed)
```

## Graph Traversal

### Breadth-First Search (BFS)

```javascript
// Visit all nodes starting from "A"
visited = graph.bfs(g, "A", null)
// ["A", "B", "C", "D"]

// With callback
graph.bfs(g, "A", (node, data) => {
    print("Visiting:", node)
})
```

### Depth-First Search (DFS)

```javascript
// Recursive DFS
visited = graph.dfs(g, "A", null)

// Iterative DFS
visited = graph.dfsIterative(g, "A", null)

// With callback
graph.dfs(g, "A", (node, data) => {
    print("Visiting:", node)
})
```

## Shortest Path

### Dijkstra's Algorithm

```javascript
// Find shortest paths from source
result = graph.dijkstra(g, "A", null)
// result.distances: {"A": 0, "B": 5, "C": 8, ...}
// result.previous: {"B": "A", "C": "B", ...}

// Find shortest path to specific target
path = graph.shortestPath(g, "A", "D")
// ["A", "B", "D"]

// Get shortest distance
dist = graph.shortestDistance(g, "A", "D")
// 7
```

### Bellman-Ford Algorithm

Works with negative edge weights.

```javascript
result = graph.bellmanFord(g, "A")
// result.distances
// result.previous
// result.hasNegativeCycle
```

## Connectivity

```javascript
// Check if graph is connected
graph.isConnected(g)  // true/false

// Find connected components
components = graph.connectedComponents(g)
// [["A", "B"], ["C", "D", "E"]]

// Check if path exists
graph.pathExists(g, "A", "D")  // true/false
```

## Cycle Detection

```javascript
// Check for cycles
graph.hasCycle(g)  // true/false
```

## Topological Sort

For directed acyclic graphs (DAGs).

```javascript
// Returns null if graph has cycles
sorted = graph.topologicalSort(g)
// ["A", "B", "C", "D"]
```

## Minimum Spanning Tree

### Kruskal's Algorithm

```javascript
mst = graph.kruskal(g)
// {
//   edges: [{from, to, weight}, ...],
//   weight: 15  // Total weight
// }
```

## Visualization

### DOT Format (Graphviz)

```javascript
dot = graph.toDot(g, "MyGraph")
// digraph MyGraph {
//     A;
//     B;
//     A -> B [label="5"];
// }
```

### Print Graph Info

```javascript
graph.printGraph(g)
// Graph Info:
//   Type: Directed, Weighted
//   Nodes: 5
//   Edges: 7
//   Connected: true
//   Has Cycle: false
```

## Examples

### Social Network

```javascript
import "lib/graph.jm" as graph

g = graph.Graph()

// Add people
graph.addNode(g, "Alice", {age: 30})
graph.addNode(g, "Bob", {age: 25})
graph.addNode(g, "Charlie", {age: 35})

// Add friendships
graph.addEdge(g, "Alice", "Bob")
graph.addEdge(g, "Bob", "Charlie")

// Find friends of friends
friends = graph.neighbors(g, "Alice")
for f in friends {
    fof = graph.neighbors(g, f)
    print(f, "knows:", fof)
}
```

### Road Network

```javascript
import "lib/graph.jm" as graph

g = graph.WeightedGraph()

// Cities and distances
graph.addEdge(g, "Berlin", "Munich", 585)
graph.addEdge(g, "Berlin", "Hamburg", 289)
graph.addEdge(g, "Munich", "Frankfurt", 392)
graph.addEdge(g, "Hamburg", "Frankfurt", 492)

// Find shortest route
path = graph.shortestPath(g, "Berlin", "Frankfurt")
dist = graph.shortestDistance(g, "Berlin", "Frankfurt")

print("Route:", path)
print("Distance:", dist, "km")
```

### Task Dependencies

```javascript
import "lib/graph.jm" as graph

g = graph.DiGraph()

// Tasks and dependencies
graph.addEdge(g, "design", "implement")
graph.addEdge(g, "implement", "test")
graph.addEdge(g, "test", "deploy")
graph.addEdge(g, "design", "document")
graph.addEdge(g, "document", "deploy")

// Get execution order
order = graph.topologicalSort(g)
print("Task order:", order)
// ["design", "implement", "document", "test", "deploy"]
```

### Network Analysis

```javascript
import "lib/graph.jm" as graph

g = graph.Graph()

// Build network
edges = [["A","B"],["B","C"],["C","D"],["D","A"],["E","F"]]
for e in edges {
    graph.addEdge(g, e[0], e[1])
}

// Analyze
print("Connected:", graph.isConnected(g))
print("Components:", graph.connectedComponents(g))
print("Has cycle:", graph.hasCycle(g))
```
