# Intermediate Code Generation — Complete Theory, Architecture, and Implementation Guide

> **Course**: Compiler Design / Language Processors  
> **Topic**: Intermediate Code Generation (ICG) — TAC, Quadruples, Triples, and Indirect Triples  
> **Directory**: `intermediate code generation`

---

## Table of Contents
1. [Aim & Objectives](#1-aim--objectives)
2. [Theoretical Background of Intermediate Code](#2-theoretical-background-of-intermediate-code)
   - [Why Intermediate Representation?](#why-intermediate-representation)
   - [Three-Address Code (TAC)](#three-address-code-tac)
   - [Quadruples](#quadruples)
   - [Triples](#triples)
   - [Indirect Triples](#indirect-triples)
   - [Comparison of Quadruples, Triples, and Indirect Triples](#comparison-of-quadruples-triples-and-indirect-triples)
3. [Algorithm Design](#3-algorithm-design)
   - [Expression Tokenization & Unary Minus Resolution](#expression-tokenization--unary-minus-resolution)
   - [Infix to Postfix Conversion (Shunting-Yard)](#infix-to-postfix-conversion-shunting-yard)
   - [Synthesis of TAC, Quadruples, and Triples](#synthesis-of-tac-quadruples-and-triples)
4. [Project Organization](#4-project-organization)
5. [Code Walkthrough](#5-code-walkthrough)
6. [Compilation & How to Run](#6-compilation--how-to-run)
7. [Comprehensive Output Walkthrough](#7-comprehensive-output-walkthrough)
   - [Expression 1: `x = (a + b) * (c - d)`](#expression-1-x--a--b--c---d)
   - [Expression 2: `a = -b * (c + d)`](#expression-2-a---b--c--d)
   - [Expression 3: `w = a + b * c / d - e`](#expression-3-w--a--b--c--d---e)
8. [Result](#8-result)

---

## 1. Aim & Objectives

### Aim
To generate intermediate code representations for source program expressions.

### Objectives
1. **Generate TAC**: Translate high-level source expressions into linearized Three-Address Code (TAC) statements.
2. **Quadruples**: Construct the Quadruple table representation containing `(op, arg1, arg2, result)` with explicit temporaries.
3. **Triples**: Construct the Triple table representation containing `(op, arg1, arg2)` using positional index references instead of explicit temporary names.
4. **Indirect Triples**: Construct the Indirect Triple representation using a pointer array referencing the triple table to facilitate optimization and instruction reordering.

---

## 2. Theoretical Background of Intermediate Code

### Why Intermediate Representation?
In a compiler, intermediate representations bridge the front end (lexical analysis, syntax analysis, semantic analysis) and the back end (code generation, target machine optimization):
- **Machine Independence**: Re-targeting the compiler to a new architecture only requires rewriting the code generator, preserving the front end and intermediate optimizer.
- **Optimization**: Machine-independent optimizations (common subexpression elimination, constant folding, dead code elimination, loop invariant code motion) are performed directly on the intermediate code.

---

### Three-Address Code (TAC)
Three-Address Code is a linearized representation of an Abstract Syntax Tree where each instruction has at most **one operator** and at most **three address operands**:
$$x = y \text{ op } z$$
$$x = \text{op } y \quad (\text{unary operations})$$
$$x = y \quad (\text{copy / assignment})$$

Compiler temporaries ($t_1, t_2, t_3, \dots$) are introduced to hold the intermediate evaluation results of subexpressions.

---

### Quadruples
A **Quadruple** is a record structure with four distinct fields:
$$\text{(op, arg1, arg2, result)}$$
- `op`: The operator being executed.
- `arg1`, `arg2`: The two operand variables or constants (for unary operators, `arg2` is unused or `-`).
- `result`: The destination variable or temporary holding the outcome.

**Advantage**: Temporaries are explicit, making variable use-def chain analysis straightforward.  
**Disadvantage**: Uses extra space to store temporary variable names.

---

### Triples
A **Triple** avoids storing explicit temporary variable names by referring to preceding computation results by their **table position (index)**:
$$\text{(op, arg1, arg2)}$$
- `op`: The operator being executed.
- `arg1`, `arg2`: Either source operands or positional pointers `(0)`, `(1)` referencing previous triples.

**Advantage**: Saves space by eliminating temporary variable names.  
**Disadvantage**: Optimization issues! If instructions are moved or optimized away, all positional references `(i)` throughout the table must be rewritten.

---

### Indirect Triples
**Indirect Triples** solve the instruction-moving limitation of triples by introducing a separate **Statement / Pointer Array**:
1. An array of pointers to the triple table representing the sequential order of execution:
   `[0] -> (0)`, `[1] -> (1)`, etc.
2. The actual Triple Table containing `(op, arg1, arg2)`.

**Advantage for Optimization**: To reorder, delete, or insert instructions during code optimization, the compiler simply modifies the pointer array without moving or changing the triple table records!

---

### Comparison of Quadruples, Triples, and Indirect Triples

| Feature | Quadruples | Triples | Indirect Triples |
| :--- | :--- | :--- | :--- |
| **Record Fields** | `(op, arg1, arg2, result)` | `(op, arg1, arg2)` | Pointer array + `(op, arg1, arg2)` |
| **Temporary Variables** | Explicit ($t_1, t_2, \dots$) | None (Positional index `(i)`) | None (Positional index `(i)`) |
| **Memory Footprint** | Moderate | Smallest | Small + pointer array |
| **Code Motion / Reordering** | Easy (move quadruples) | Difficult (requires updating all index pointers) | Very Easy (just reorder pointer array) |

---

## 3. Algorithm Design

### Expression Tokenization & Unary Minus Resolution
1. Identifiers (`a`, `b`, `c`, `temp`) and constants are extracted.
2. Unary minus vs. binary minus is resolved:
   - A `-` symbol is **unary minus** (`uminus`) if it occurs at the start of the expression or immediately after an operator (`(`, `=`, `+`, `-`, `*`, `/`).
   - Otherwise, it is binary minus.

### Infix to Postfix Conversion (Shunting-Yard)
The standard Dijkstra Shunting-Yard algorithm transforms expressions using precedence rules:
- Precedence 3: Unary minus (`uminus`) — Right-associative
- Precedence 2: Multiplication (`*`), Division (`/`) — Left-associative
- Precedence 1: Addition (`+`), Subtraction (`-`) — Left-associative

### Synthesis of TAC, Quadruples, and Triples
During postfix stack evaluation:
- When encountering an operator:
  1. Pop operands from the value stack and triple stack.
  2. Generate a fresh temporary variable $t_i$.
  3. Emit TAC: $t_i = \text{arg}_1 \text{ op } \text{arg}_2$.
  4. Emit Quadruple: `(op, arg1, arg2, ti)`.
  5. Emit Triple: `(op, trip_arg1, trip_arg2)`.
  6. Append current index to the Indirect Triple pointer list.
  7. Push $t_i$ and `(index)` back onto the respective evaluation stacks.
- Final assignment statement links the destination identifier to the final temporary/triple result.

---

## 4. Project Organization

```
intermediate code generation/
├── QUESTION.md      # Aim, Objectives, Sub-tasks, Expected Output & Result
├── README.md        # Comprehensive Theory, Comparative Analysis & Walkthroughs
├── main.cpp         # Clean C++ implementation (Self-contained, < 350 lines)
└── icg.exe          # Pre-compiled Windows native executable
```

---

## 5. Code Walkthrough

In [`main.cpp`](file:///c:/Users/6217a/Desktop/compiler%20design/intermediate%20code%20generation/main.cpp):
1. **`Quadruple`**: Record with `index`, `op`, `arg1`, `arg2`, `result`.
2. **`Triple`**: Record with `index`, `op`, `arg1`, `arg2`.
3. **`IntermediateCodeGenerator`**:
   - `tokenize()`: Splits input into tokens and resolves `uminus`.
   - `infixToPostfix()`: Shunting-yard algorithm handling operator precedence and parentheses.
   - `generate()`: Evaluates postfix to synthesize TAC, Quadruples, Triples, and Indirect Triples simultaneously.
   - `printTAC()`, `printQuadruples()`, `printTriples()`, `printIndirectTriples()`: Tabular formatting.

---

## 6. Compilation & How to Run

### Step 1: Run Pre-Compiled Binary Directly
```powershell
cd "c:\Users\6217a\Desktop\compiler design\intermediate code generation"
.\icg.exe
```

### Step 2: Recompile with GCC / MinGW
```powershell
g++ -std=c++17 -O2 main.cpp -o icg.exe
.\icg.exe
```

---

## 7. Comprehensive Output Walkthrough

### Expression 1: `x = (a + b) * (c - d)`

#### [1] Three-Address Code (TAC)
```text
  (1)  t1 = a + b
  (2)  t2 = c - d
  (3)  t3 = t1 * t2
  (4)  x = t3
```

#### [2] Quadruple Table
| Index | Op | Arg1 | Arg2 | Result |
| :---: | :---: | :---: | :---: | :---: |
| **(0)** | `+` | `a` | `b` | `t1` |
| **(1)** | `-` | `c` | `d` | `t2` |
| **(2)** | `*` | `t1` | `t2` | `t3` |
| **(3)** | `=` | `t3` | `-` | `x` |

#### [3] Triple Table
| Index | Op | Arg1 | Arg2 |
| :---: | :---: | :---: | :---: |
| **(0)** | `+` | `a` | `b` |
| **(1)** | `-` | `c` | `d` |
| **(2)** | `*` | `(0)` | `(1)` |
| **(3)** | `=` | `x` | `(2)` |

#### [4] Indirect Triple Table
```text
  Statement Pointers Table:
  Statement   Triple Address  
  -----------------------------------
  [0]         (0)             
  [1]         (1)             
  [2]         (2)             
  [3]         (3)             

  Indirect Triples Table:
  ---------------------------------------------------
  Address   Op          Arg1            Arg2            
  ---------------------------------------------------
  (0)       +           a               b               
  (1)       -           c               d               
  (2)       *           (0)             (1)             
  (3)       =           x               (2)             
```

---

### Expression 2: `a = -b * (c + d)`

#### [1] Three-Address Code (TAC)
```text
  (1)  t1 = - b
  (2)  t2 = c + d
  (3)  t3 = t1 * t2
  (4)  a = t3
```

#### [2] Quadruple Table
| Index | Op | Arg1 | Arg2 | Result |
| :---: | :---: | :---: | :---: | :---: |
| **(0)** | `uminus` | `b` | `-` | `t1` |
| **(1)** | `+` | `c` | `d` | `t2` |
| **(2)** | `*` | `t1` | `t2` | `t3` |
| **(3)** | `=` | `t3` | `-` | `a` |

#### [3] Triple Table
| Index | Op | Arg1 | Arg2 |
| :---: | :---: | :---: | :---: |
| **(0)** | `uminus` | `b` | `-` |
| **(1)** | `+` | `c` | `d` |
| **(2)** | `*` | `(0)` | `(1)` |
| **(3)** | `=` | `a` | `(2)` |

#### [4] Indirect Triple Table
```text
  Statement Pointers Table:
  Statement   Triple Address  
  -----------------------------------
  [0]         (0)             
  [1]         (1)             
  [2]         (2)             
  [3]         (3)             

  Indirect Triples Table:
  ---------------------------------------------------
  Address   Op          Arg1            Arg2            
  ---------------------------------------------------
  (0)       uminus      b               -               
  (1)       +           c               d               
  (2)       *           (0)             (1)             
  (3)       =           a               (2)             
```

---

### Expression 3: `w = a + b * c / d - e`

#### [1] Three-Address Code (TAC)
```text
  (1)  t1 = b * c
  (2)  t2 = t1 / d
  (3)  t3 = a + t2
  (4)  t4 = t3 - e
  (5)  w = t4
```

#### [2] Quadruple Table
| Index | Op | Arg1 | Arg2 | Result |
| :---: | :---: | :---: | :---: | :---: |
| **(0)** | `*` | `b` | `c` | `t1` |
| **(1)** | `/` | `t1` | `d` | `t2` |
| **(2)** | `+` | `a` | `t2` | `t3` |
| **(3)** | `-` | `t3` | `e` | `t4` |
| **(4)** | `=` | `t4` | `-` | `w` |

#### [3] Triple Table
| Index | Op | Arg1 | Arg2 |
| :---: | :---: | :---: | :---: |
| **(0)** | `*` | `b` | `c` |
| **(1)** | `/` | `(0)` | `d` |
| **(2)** | `+` | `a` | `(1)` |
| **(3)** | `-` | `(2)` | `e` |
| **(4)** | `=` | `w` | `(3)` |

#### [4] Indirect Triple Table
```text
  Statement Pointers Table:
  Statement   Triple Address  
  -----------------------------------
  [0]         (0)             
  [1]         (1)             
  [2]         (2)             
  [3]         (3)             
  [4]         (4)             

  Indirect Triples Table:
  ---------------------------------------------------
  Address   Op          Arg1            Arg2            
  ---------------------------------------------------
  (0)       *           b               c               
  (1)       /           (0)             d               
  (2)       +           a               (1)             
  (3)       -           (2)             e               
  (4)       =           w               (3)             
```

---

## 8. Result

The **Intermediate Code Generation Suite** has been developed, verified, and demonstrated:
1. **TAC Generation**: Linearized instructions with temporary allocations and assignment statements.
2. **Quadruple Table**: Correct field mappings `(op, arg1, arg2, result)` with explicit temporaries.
3. **Triple Table**: Successfully eliminated temporaries using positional references `(0)`, `(1)`, etc.
4. **Indirect Triples**: Constructed the decoupled execution pointer array and triple definitions table.

Intermediate code generated successfully.
