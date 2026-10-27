# LR(1) Parser — Complete Theory, Architecture, and Implementation Guide

> **Course**: Compiler Design / Language Processors  
> **Topic**: Bottom-Up Shift-Reduce Parsing — LR(1) Canonical Collection, ACTION & GOTO Table Construction, and Stack Parsing Simulation  
> **Directory**: `lr1 parser`

---

## Table of Contents
1. [Aim & Objectives](#1-aim--objectives)
2. [Theoretical Foundations of LR(1) Parsing](#2-theoretical-foundations-of-lr1-parsing)
   - [What is an LR(1) Parser?](#what-is-an-lr1-parser)
   - [LR(1) Items vs LR(0) Items](#lr1-items-vs-lr0-items)
   - [Why LR(1) Resolves Conflicts](#why-lr1-resolves-conflicts)
3. [Core Algorithms](#3-core-algorithms)
   - [Sub-Task 1: Formulation of LR(1) Items & CLOSURE](#sub-task-1-formulation-of-lr1-items--closure)
   - [Sub-Task 2: GOTO Function & Canonical Collection](#sub-task-2-goto-function--canonical-collection)
   - [Sub-Task 3: ACTION and GOTO Parsing Table Construction](#sub-task-3-action-and-goto-parsing-table-construction)
   - [Sub-Task 4: Shift-Reduce Parsing Simulation](#sub-task-4-shift-reduce-parsing-simulation)
4. [Project Organization](#4-project-organization)
5. [Code Walkthrough](#5-code-walkthrough)
6. [Compilation & How to Run](#6-compilation--how-to-run)
7. [Comprehensive Output Walkthrough](#7-comprehensive-output-walkthrough)
   - [Grammar Productions](#grammar-productions)
   - [Canonical Collection of LR(1) Item Sets](#canonical-collection-of-lr1-item-sets)
   - [LR(1) Parsing Table (ACTION & GOTO)](#lr1-parsing-table-action--goto)
   - [Parse Trace 1: `c d c d` (ACCEPT)](#parse-trace-1-c-d-c-d-accept)
   - [Parse Trace 2: `d d` (ACCEPT)](#parse-trace-2-d-d-accept)
   - [Parse Trace 3: `c d c` (REJECT — Syntax Error)](#parse-trace-3-c-d-c-reject--syntax-error)
8. [Result](#8-result)

---

## 1. Aim & Objectives

### Aim
To develop a complete, fully functional **LR(1) Parser** for Context-Free Grammars.

### Objectives
1. **Construct LR Items**: Represent LR(1) items $[A \to \alpha \cdot \beta, \, a]$ combining core dotted productions with a 1-symbol terminal lookahead.
2. **Canonical Collection**: Compute the complete set of LR(1) states using iterative `CLOSURE` and `GOTO` operations over grammar symbols.
3. **ACTION/GOTO Tables**: Construct the parsing action table (`shift`, `reduce`, `accept`, `error`) and non-terminal transition `GOTO` table, checking for any grammar conflicts.
4. **Parse Input**: Implement a shift-reduce pushdown automaton that simulates parsing of tokenized input strings, tracing each step with stack configurations and returning an **Accept** or **Reject** result.

---

## 2. Theoretical Foundations of LR(1) Parsing

### What is an LR(1) Parser?
An **LR(1)** parser is a bottom-up, non-backtracking, deterministic shift-reduce parser:
- **L**: Scans the input from **L**eft-to-right.
- **R**: Constructs a **R**ightmost derivation in reverse.
- **(1)**: Uses **1** token of lookahead to determine parsing actions.

LR(1) parsers are the most powerful deterministic bottom-up parsers in common use; they recognize virtually all context-free syntactic constructs in modern programming languages.

### LR(1) Items vs LR(0) Items
- An **LR(0) item** is simply a production with a dot indicating how much has been parsed:
  $$[A \to \alpha \cdot \beta]$$
  In LR(0) and SLR(1) parsing, a reduction $A \to \alpha \cdot$ applies blindly to all follow symbols, often creating false shift/reduce or reduce/reduce conflicts.
- An **LR(1) item** pairs the dotted production with an explicit **lookahead terminal**:
  $$[A \to \alpha \cdot \beta, \quad a]$$
  where $a \in (V_T \cup \{\$\})$. The lookahead $a$ specifies that reduction to $A$ is valid **only** if the next input terminal is exactly $a$.

### Why LR(1) Resolves Conflicts
By tracking the precise lookahead context in which a non-terminal was introduced:
- A reduce action is placed in $\text{ACTION}[i, a]$ **only** for the lookaheads $a$ that can legally follow that particular reduction in that specific state.
- This eliminates spurious shift/reduce and reduce/reduce conflicts present in SLR(1) parsers.

---

## 3. Core Algorithms

### Sub-Task 1: Formulation of LR(1) Items & CLOSURE
Given an augmented grammar with start production $S' \to S$:

$$\text{CLOSURE}(I):$$
1. Initialize $J = I$.
2. Repeat until no more items can be added to $J$:
   - For every item $[A \to \alpha \cdot B \beta, \, a] \in J$:
     - For every production $B \to \gamma$:
       - For every terminal $b \in \text{FIRST}(\beta a)$:
         - Add $[B \to \cdot \gamma, \, b]$ to $J$.
3. Return $J$.

---

### Sub-Task 2: GOTO Function & Canonical Collection

$$\text{GOTO}(I, X):$$
For any grammar symbol $X \in (V_N \cup V_T)$:
$$\text{GOTO}(I, X) = \text{CLOSURE}(\{ [A \to \alpha X \cdot \beta, \, a] \mid [A \to \alpha \cdot X \beta, \, a] \in I \})$$

#### Canonical Collection Generation:
1. $I_0 = \text{CLOSURE}(\{[S' \to \cdot S, \, \$]\})$.
2. Set $C = \{I_0\}$.
3. Repeat until no new item sets are added to $C$:
   - For each item set $I_i \in C$:
     - For each grammar symbol $X$:
       - Let $J = \text{GOTO}(I_i, X)$.
       - If $J \ne \emptyset$ and $J \notin C$:
         - Add $J$ as a new state to $C$.

---

### Sub-Task 3: ACTION and GOTO Parsing Table Construction

Let $C = \{I_0, I_1, \dots, I_n\}$ be the canonical collection. For each state $i$ (corresponding to $I_i$):
1. **Shift**: If $[A \to \alpha \cdot a \beta, \, b] \in I_i$ and $\text{GOTO}(I_i, a) = I_j$ (where $a$ is a terminal):
   $$\text{ACTION}[i, a] = \text{"s"}j$$
2. **Reduce**: If $[A \to \alpha \cdot, \, a] \in I_i$ and $A \ne S'$:
   Let production $A \to \alpha$ be production rule index $k$:
   $$\text{ACTION}[i, a] = \text{"r"}k$$
3. **Accept**: If $[S' \to S \cdot, \, \$] \in I_i$:
   $$\text{ACTION}[i, \$] = \text{"acc"}$$
4. **GOTO**: If $\text{GOTO}(I_i, A) = I_j$ for non-terminal $A \in V_N$:
   $$\text{GOTO}[i, A] = j$$

---

### Sub-Task 4: Shift-Reduce Parsing Simulation

The parser maintains a pushdown stack containing alternating states and grammar symbols:
- **Initial Stack**: States: `[0]`, Symbols: `[$]`
- **Input Buffer**: Tokens followed by `$`

```text
While true:
    Let s = top state on stack
    Let a = current input token
    Look up ACTION[s, a]

    Case 1: "acc"
        ACCEPT! Parsing successfully completed. Return true.

    Case 2: "sj" (Shift to state j)
        Push symbol a onto symbol stack.
        Push state j onto state stack.
        Advance input pointer.

    Case 3: "rk" (Reduce by rule k: A -> alpha)
        Pop |alpha| elements from state stack.
        Pop |alpha| elements from symbol stack.
        Let topState = state at top of state stack.
        Let nextState = GOTO[topState, A].
        Push symbol A onto symbol stack.
        Push nextState onto state stack.

    Case 4: Empty / No entry
        SYNTAX ERROR! Reject input string. Return false.
```

---

## 4. Project Organization

```
lr1 parser/
├── QUESTION.md      # Aim, Objectives, Sub-tasks, Expected Output & Result
├── README.md        # Comprehensive Theory, Algorithms, Tables & Parse Traces
├── main.cpp         # Clean, self-contained C++ implementation (under 400 lines)
└── lr1_parser.exe   # Pre-compiled Windows native binary
```

---

## 5. Code Walkthrough

In [`main.cpp`](file:///c:/Users/6217a/Desktop/compiler%20design/lr1%20parser/main.cpp):
1. **`Production`**: Stores production id, left-hand side (`lhs`), and right-hand side token vector (`rhs`).
2. **`Item`**: Encapsulates dotted production (`prodId`, `dot`) and lookahead string (`lookahead`).
3. **`LR1Parser`**:
   - `computeFirst()`: Computes `FIRST` sets for all terminals and non-terminals.
   - `closure()`: Expands items according to the LR(1) closure definition.
   - `goTo()`: Advances the dot over symbol $X$ and takes the closure.
   - `generateCanonicalCollection()`: Discovers all reachable states $I_0, I_1, \dots, I_{n-1}$.
   - `buildParsingTables()`: Fills `actionTable` (`s`, `r`, `acc`) and `gotoTable`.
   - `parseString()`: Simulates the pushdown automaton and prints step-by-step trace tables.

---

## 6. Compilation & How to Run

### Step 1: Run Pre-Compiled Binary Directly
```powershell
cd "c:\Users\6217a\Desktop\compiler design\lr1 parser"
.\lr1_parser.exe
```

### Step 2: Recompile with GCC / MinGW
```powershell
g++ -std=c++17 -O2 main.cpp -o lr1_parser.exe
.\lr1_parser.exe
```

---

## 7. Comprehensive Output Walkthrough

The textbook canonical grammar used for demonstration is:
$$S \to C \, C$$
$$C \to c \, C \mid d$$

### Grammar Productions
```text
  (0) S' -> S
  (1) S -> C C
  (2) C -> c C
  (3) C -> d
```

---

### Canonical Collection of LR(1) Item Sets

```text
I0:
    [S' -> . S , $]
    [S -> . C C , $]
    [C -> . c C , c]
    [C -> . c C , d]
    [C -> . d , c]
    [C -> . d , d]

I1:
    [C -> . c C , c]
    [C -> . c C , d]
    [C -> c . C , c]
    [C -> c . C , d]
    [C -> . d , c]
    [C -> . d , d]

I2:
    [C -> d . , c]
    [C -> d . , d]

I3:
    [S -> C . C , $]
    [C -> . c C , $]
    [C -> . d , $]

I4:
    [S' -> S . , $]

I5:
    [C -> c C . , c]
    [C -> c C . , d]

I6:
    [C -> . c C , $]
    [C -> c . C , $]
    [C -> . d , $]

I7:
    [C -> d . , $]

I8:
    [S -> C C . , $]

I9:
    [C -> c C . , $]
```

---

### LR(1) Parsing Table (ACTION & GOTO)

| State | `c` | `d` | `$` | `C` | `S` |
| :---: | :---: | :---: | :---: | :---: | :---: |
| **0** | s1 | s2 | - | 3 | 4 |
| **1** | s1 | s2 | - | 5 | - |
| **2** | r3 | r3 | - | - | - |
| **3** | s6 | s7 | - | 8 | - |
| **4** | - | - | **acc** | - | - |
| **5** | r2 | r2 | - | - | - |
| **6** | s6 | s7 | - | 9 | - |
| **7** | - | - | r3 | - | - |
| **8** | - | - | r1 | - | - |
| **9** | - | - | r2 | - | - |

---

### Parse Trace 1: `c d c d` (ACCEPT)

```text
Step  State Stack         Symbol Stack      Input Buffer        Action
----------------------------------------------------------------------------
1     0                   $                 c d c d $           Shift to State 1
2     0 1                 $ c               d c d $             Shift to State 2
3     0 1 2               $ c d             c d $               Reduce by C -> d
4     0 1 5               $ c C             c d $               Reduce by C -> c C
5     0 3                 $ C               c d $               Shift to State 6
6     0 3 6               $ C c             d $                 Shift to State 7
7     0 3 6 7             $ C c d           $                   Reduce by C -> d
8     0 3 6 9             $ C c C           $                   Reduce by C -> c C
9     0 3 8               $ C C             $                   Reduce by S -> C C
10    0 4                 $ S               $                   ACCEPT
----------------------------------------------------------------------------
>> Result: ACCEPTED (String successfully parsed)
```

---

### Parse Trace 2: `d d` (ACCEPT)

```text
Step  State Stack         Symbol Stack      Input Buffer        Action
----------------------------------------------------------------------------
1     0                   $                 d d $               Shift to State 2
2     0 2                 $ d               d $                 Reduce by C -> d
3     0 3                 $ C               d $                 Shift to State 7
4     0 3 7               $ C d             $                   Reduce by C -> d
5     0 3 8               $ C C             $                   Reduce by S -> C C
6     0 4                 $ S               $                   ACCEPT
----------------------------------------------------------------------------
>> Result: ACCEPTED (String successfully parsed)
```

---

### Parse Trace 3: `c d c` (REJECT — Syntax Error)

```text
Step  State Stack         Symbol Stack      Input Buffer        Action
----------------------------------------------------------------------------
1     0                   $                 c d c $             Shift to State 1
2     0 1                 $ c               d c $               Shift to State 2
3     0 1 2               $ c d             c $                 Reduce by C -> d
4     0 1 5               $ c C             c $                 Reduce by C -> c C
5     0 3                 $ C               c $                 Shift to State 6
6     0 3 6               $ C c             $                   ERROR (No action for [6, $])
----------------------------------------------------------------------------
>> Result: REJECTED (Syntax Error)
```

---

## 8. Result

The **LR(1) Parser** has been completely developed, tested, and verified:
1. **Construct LR Items**: Augmented grammar initialized, and `CLOSURE` operator successfully propagates lookaheads using `FIRST` calculations.
2. **Canonical Collection**: Successfully generated the 10 distinct canonical LR(1) item sets ($I_0$ to $I_9$).
3. **ACTION & GOTO Tables**: Successfully built conflict-free shift, reduce, accept, and non-terminal transition tables.
4. **Shift-Reduce Parsing**: Demonstrated step-by-step stack simulation on both valid (`c d c d`, `d d`) and invalid (`c d c`) inputs, concluding with deterministic **`ACCEPT`** or **`REJECT`** results.

The LR(1) parser works successfully.
