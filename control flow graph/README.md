# Control Flow Graph (CFG) Generation — Complete Theory, Architecture, and Implementation Guide

> **Course**: Compiler Design / Language Processors  
> **Topic**: Code Optimization & Control Flow Analysis — Leader Identification, Basic Blocks Partitioning, and CFG Construction  
> **Directory**: `control flow graph`

---

## Table of Contents
1. [Aim & Objectives](#1-aim--objectives)
2. [Theoretical Background of Control Flow Analysis](#2-theoretical-background-of-control-flow-analysis)
   - [What is a Basic Block?](#what-is-a-basic-block)
   - [What is a Control Flow Graph (CFG)?](#what-is-a-control-flow-graph-cfg)
   - [Why are CFGs Critical in Compilers?](#why-are-cfgs-critical-in-compilers)
3. [Core Algorithms](#3-core-algorithms)
   - [Sub-Task 1: The Three Rules of Leader Identification](#sub-task-1-the-three-rules-of-leader-identification)
   - [Sub-Task 2: Partitioning Instructions into Basic Blocks](#sub-task-2-partitioning-instructions-into-basic-blocks)
   - [Sub-Task 3: Constructing Directed Control Flow Edges](#sub-task-3-constructing-directed-control-flow-edges)
   - [Sub-Task 4: Displaying the Graph (Adjacency, Predecessors/Successors, ASCII & DOT)](#sub-task-4-displaying-the-graph)
4. [Project Organization](#4-project-organization)
5. [Code Walkthrough](#5-code-walkthrough)
6. [Compilation & How to Run](#6-compilation--how-to-run)
7. [Comprehensive Output Walkthrough](#7-comprehensive-output-walkthrough)
   - [Program 1: Loop with Conditional Branch (Sum of Squares)](#program-1-loop-with-conditional-branch-sum-of-squares)
   - [Program 2: If-Else Branching Structure](#program-2-if-else-branching-structure)
8. [Result](#8-result)

---

## 1. Aim & Objectives

### Aim
To generate a **Control Flow Graph (CFG)** from intermediate Three-Address Code (TAC).

### Objectives
1. **Identify Leaders**: Determine all leader statements in a sequence of Three-Address Code instructions using formal leader identification rules.
2. **Construct Basic Blocks**: Partition the instruction stream into maximal sequences of consecutive statements (Basic Blocks) where control enters solely at the leader and leaves at the final instruction.
3. **Generate CFG**: Construct directed edges between basic blocks reflecting sequential fall-throughs, conditional branches, and unconditional jumps.
4. **Display Graph**: Present the control flow graph using structural adjacency representations, predecessor/successor mappings, and visual ASCII graph diagrams.

---

## 2. Theoretical Background of Control Flow Analysis

### What is a Basic Block?
A **Basic Block** is a sequence of consecutive Three-Address Code statements in which:
1. Flow of control enters **only** at the beginning of the block (the leader statement).
2. Flow of control leaves the block **only** at the end of the block (at the final statement).
3. Once the first statement is executed, all subsequent statements in the block are executed sequentially without halting or branching (except possibly at the last statement).

### What is a Control Flow Graph (CFG)?
A **Control Flow Graph (CFG)** is a directed graph $G = (V, E)$ where:
- The vertices $V$ represent the **Basic Blocks**.
- The directed edges $E$ represent possible **transfers of control** between basic blocks at runtime.
- Two special distinguished nodes are included or implied:
  - **ENTRY**: Point where control enters the graph.
  - **EXIT**: Point where control terminates or leaves the procedure.

### Why are CFGs Critical in Compilers?
CFGs serve as the primary intermediate representation for:
- **Data-Flow Analysis**: Available expressions, reaching definitions, live variable analysis.
- **Loop Optimizations**: Loop invariant code motion, induction variable elimination, loop unrolling.
- **Dead Code Elimination**: Detecting unreachable basic blocks and removing useless computations.

---

## 3. Core Algorithms

### Sub-Task 1: The Three Rules of Leader Identification
Given a numbered sequence of Three-Address Code statements $1, 2, \dots, n$:

1. **Rule 1 (First Statement)**:
   The first statement of the program (Line 1) is always a leader.
2. **Rule 2 (Target of Jump)**:
   Any statement that is the target of a conditional or unconditional `goto` is a leader.
3. **Rule 3 (Successor of Jump)**:
   Any statement that immediately follows a conditional or unconditional `goto` is a leader.

---

### Sub-Task 2: Partitioning Instructions into Basic Blocks
After all leaders are identified:
1. For each leader instruction:
   - Its basic block starts at the leader.
   - It contains all subsequent instructions up to, but not including, the next leader (or until the end of the instruction sequence).
2. The basic blocks are sequentially indexed as $B_1, B_2, \dots, B_k$.
3. Every instruction in the program belongs to exactly one basic block.

---

### Sub-Task 3: Constructing Directed Control Flow Edges
For each basic block $B_i$ ending with instruction $L$:
1. **Conditional Jump** (`if condition goto target`):
   - Add a **True Branch edge**: $B_i \to B_{\text{target}}$ (where $B_{\text{target}}$ is the block containing the target line).
   - Add a **False Fall-through edge**: $B_i \to B_{i+1}$ (where $B_{i+1}$ is the next sequential block, if it exists).
2. **Unconditional Jump** (`goto target`):
   - Add an **Unconditional Jump edge**: $B_i \to B_{\text{target}}$.
   - No fall-through edge is added because control cannot pass to $B_{i+1}$ sequentially.
3. **Halt / Return** (`halt` or `return`):
   - No outgoing edges to subsequent blocks are added.
4. **Normal Sequential Instruction**:
   - If $B_i$ does not end with a jump or halt, add a **Fall-through edge**: $B_i \to B_{i+1}$.

---

### Sub-Task 4: Displaying the Graph
- **Leader Table**: Line #, Statement, Leader Flag (`YES`/`No`), and Reason.
- **Basic Blocks Partition**: Statements grouped within each block $B_i$.
- **Adjacency Table**: Directed edges $(B_i \to B_j)$ with edge classifications.
- **Predecessor & Successor Sets**: $\text{Pred}(B_i)$ and $\text{Succ}(B_i)$ for dataflow analysis.
- **ASCII Graph & Graphviz DOT**: Human-readable terminal diagrams and standard DOT format for external renderers.

---

## 4. Project Organization

```
control flow graph/
├── QUESTION.md      # Aim, Objectives, Sub-tasks, Expected Output & Result
├── README.md        # Comprehensive Theory, Algorithms, Walkthroughs & Diagrams
├── main.cpp         # Clean C++ implementation (Self-contained, 381 lines)
└── cfg.exe          # Pre-compiled Windows native executable
```

---

## 5. Code Walkthrough

In [`main.cpp`](file:///c:/Users/6217a/Desktop/compiler%20design/control%20flow%20graph/main.cpp):
1. **`Instruction`**: Stores line number, text, leader flag, reason, and jump targets.
2. **`BasicBlock`**: Stores block ID, start line, end line, instructions list, predecessors, and successors.
3. **`ControlFlowGraph`**:
   - `parseInstructions()`: Scans code and identifies goto targets.
   - `identifyLeaders()`: Applies Rules 1, 2, and 3.
   - `constructBasicBlocks()`: Groups statements into blocks.
   - `generateCFG()`: Builds directed edges, predecessors, and successors.
   - `displayLeaders()`, `displayBasicBlocks()`, `displayCFGStructure()`, `displayASCIIGraph()`, `displayDOTGraph()`: Outputs complete graph analysis.

---

## 6. Compilation & How to Run

### Step 1: Run Pre-Compiled Binary Directly
```powershell
cd "c:\Users\6217a\Desktop\compiler design\control flow graph"
.\cfg.exe
```

### Step 2: Recompile with GCC / MinGW
```powershell
g++ -std=c++17 -O2 main.cpp -o cfg.exe
.\cfg.exe
```

---

## 7. Comprehensive Output Walkthrough

### Program 1: Loop with Conditional Branch (Sum of Squares)

#### Three-Address Code Input
```text
1:  i = 1
2:  sum = 0
3:  t1 = i * i
4:  sum = sum + t1
5:  i = i + 1
6:  if i <= 10 goto 3
7:  write sum
8:  halt
```

#### [1] Leader Identification Table
| Line | Three-Address Code | Is Leader? | Reason |
| :---: | :--- | :---: | :--- |
| **1** | `i = 1` | **YES** | Rule 1: First statement of program |
| **2** | `sum = 0` | No | - |
| **3** | `t1 = i * i` | **YES** | Rule 2: Target of goto at line 6 |
| **4** | `sum = sum + t1` | No | - |
| **5** | `i = i + 1` | No | - |
| **6** | `if i <= 10 goto 3` | No | - |
| **7** | `write sum` | **YES** | Rule 3: Follows goto at line 6 |
| **8** | `halt` | No | - |

#### [2] Basic Blocks Partition
```text
  +--- [ Block B1 ] (Lines 1 to 2) ---
  |  (1)  i = 1
  |  (2)  sum = 0
  +-------------------------------------------------

  +--- [ Block B2 ] (Lines 3 to 6) ---
  |  (3)  t1 = i * i
  |  (4)  sum = sum + t1
  |  (5)  i = i + 1
  |  (6)  if i <= 10 goto 3
  +-------------------------------------------------

  +--- [ Block B3 ] (Lines 7 to 8) ---
  |  (7)  write sum
  |  (8)  halt
  +-------------------------------------------------
```

#### [3] CFG Edges & Adjacency
| Source | Target | Edge Type | Description |
| :---: | :---: | :--- | :--- |
| **B1** | **B2** | Fall-through | Control transfers from B1 -> B2 |
| **B2** | **B2** | Branch (True) | Loop back edge: control transfers from B2 -> B2 |
| **B2** | **B3** | Fall-through (False) | Loop exit edge: control transfers from B2 -> B3 |

#### [4] Predecessor & Successor Sets
| Block | Predecessors | Successors |
| :---: | :--- | :--- |
| **B1** | `{ }` | `{ B2 }` |
| **B2** | `{ B1, B2 }` | `{ B2, B3 }` |
| **B3** | `{ B2 }` | `{ }` |

#### [5] ASCII Control Flow Graph
```text
  [ENTRY]
     |
     v
  +-----------------------------+
  |          BLOCK B1           |
  |  1: i = 1                   |
  |  2: sum = 0                 |
  +-----------------------------+
     | (to B2: Fall-through)
     v
  +-----------------------------+
  |          BLOCK B2           |
  |  3: t1 = i * i              |
  |  4: sum = sum + t1          |
  |  5: i = i + 1               |
  |  6: if i <= 10 goto 3       |
  +-----------------------------+
     |--- Loop back to B2 (Branch (True))
     | (to B3: Fall-through (False))
     v
  +-----------------------------+
  |          BLOCK B3           |
  |  7: write sum               |
  |  8: halt                    |
  +-----------------------------+
     |
     v
  [EXIT]
```

---

### Program 2: If-Else Branching Structure

#### Three-Address Code Input
```text
1:  read x
2:  if x > 0 goto 5
3:  y = -x
4:  goto 6
5:  y = x
6:  write y
7:  halt
```

#### [1] Leader Identification Table
| Line | Three-Address Code | Is Leader? | Reason |
| :---: | :--- | :---: | :--- |
| **1** | `read x` | **YES** | Rule 1: First statement of program |
| **2** | `if x > 0 goto 5` | No | - |
| **3** | `y = -x` | **YES** | Rule 3: Follows goto at line 2 |
| **4** | `goto 6` | No | - |
| **5** | `y = x` | **YES** | Rule 2: Target of line 2 & Rule 3: Follows goto at line 4 |
| **6** | `write y` | **YES** | Rule 2: Target of goto at line 4 |
| **7** | `halt` | No | - |

#### [2] Basic Blocks Partition
- **Block B1** (Lines 1 to 2): `read x`, `if x > 0 goto 5`
- **Block B2** (Lines 3 to 4): `y = -x`, `goto 6`
- **Block B3** (Line 5): `y = x`
- **Block B4** (Lines 6 to 7): `write y`, `halt`

#### [3] CFG Edges
- **B1 $\to$ B3**: Branch (True)
- **B1 $\to$ B2**: Fall-through (False)
- **B2 $\to$ B4**: Unconditional Jump
- **B3 $\to$ B4**: Fall-through

#### [4] Predecessors & Successors
| Block | Predecessors | Successors |
| :---: | :--- | :--- |
| **B1** | `{ }` | `{ B3, B2 }` |
| **B2** | `{ B1 }` | `{ B4 }` |
| **B3** | `{ B1 }` | `{ B4 }` |
| **B4** | `{ B2, B3 }` | `{ }` |

---

## 8. Result

The **Control Flow Graph (CFG) Generator** has been developed, tested, and verified:
1. **Identify Leaders**: Successfully classified leaders according to Rules 1, 2, and 3 with diagnostic justification.
2. **Construct Basic Blocks**: Partitioned instruction streams into maximal non-branching sequences ($B_1, B_2, \dots$).
3. **Generate CFG**: Correctly computed directed edges, branch types (True, False, Unconditional, Fall-through), and Predecessor/Successor sets.
4. **Display Graph**: Rendered tabular adjacency, ASCII visual diagrams, and Graphviz DOT representations.

CFG generated successfully.
