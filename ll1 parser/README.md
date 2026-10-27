# LL(1) Parser — Complete Theory, Architecture, and Implementation Guide

> **Course**: Compiler Design / Language Processors  
> **Topic**: Top-Down Predictive Parsing — LL(1) Grammar Transformation, Table Construction, and Stack Parsing Simulation  
> **Directory**: `ll1 parser`

---

## Table of Contents
1. [Aim & Objectives](#1-aim--objectives)
2. [Theoretical Background of LL(1) Parsing](#2-theoretical-background-of-ll1-parsing)
   - [What is an LL(1) Parser?](#what-is-an-ll1-parser)
   - [Necessary & Sufficient Conditions for LL(1)](#necessary--sufficient-conditions-for-ll1)
   - [Why Left Recursion Must Be Eliminated](#why-left-recursion-must-be-eliminated)
   - [Why Left Factoring is Necessary](#why-left-factoring-is-necessary)
3. [Core Algorithms](#3-core-algorithms)
   - [Sub-Task 1: Elimination of Left Recursion](#sub-task-1-elimination-of-left-recursion)
   - [Sub-Task 2: Left Factoring](#sub-task-2-left-factoring)
   - [Sub-Task 3: Computation of FIRST and FOLLOW Sets](#sub-task-3-computation-of-first-and-follow-sets)
   - [Sub-Task 4: LL(1) Parsing Table Construction](#sub-task-4-ll1-parsing-table-construction)
   - [Sub-Task 5: Stack-Based Predictive Parsing Simulation](#sub-task-5-stack-based-predictive-parsing-simulation)
4. [Project Architecture & File Organization](#4-project-architecture--file-organization)
5. [Code Walkthrough](#5-code-walkthrough)
   - [Grammar Representation](#grammar-representation)
   - [Left Recursion Removal Engine](#left-recursion-removal-engine)
   - [Left Factoring Engine](#left-factoring-engine)
   - [FIRST and FOLLOW Computation Engine](#first-and-follow-computation-engine)
   - [Table Generator & Conflict Detector](#table-generator--conflict-detector)
   - [Pushdown Stack Simulation Engine](#pushdown-stack-simulation-engine)
6. [Compilation & How to Run](#6-compilation--how-to-run)
7. [Comprehensive Output Walkthrough](#7-comprehensive-output-walkthrough)
   - [Phase 1: Arithmetic Grammar Transformation](#phase-1-arithmetic-grammar-transformation)
   - [Phase 2: FIRST and FOLLOW Sets](#phase-2-first-and-follow-sets)
   - [Phase 3: LL(1) Parsing Table](#phase-3-ll1-parsing-table)
   - [Phase 4: Step-by-Step Parse Trace for `id + id * id` (ACCEPT)](#phase-4-step-by-step-parse-trace-for-id--id--id-accept)
   - [Phase 5: Step-by-Step Parse Trace for `( id + id ) * id` (ACCEPT)](#phase-5-step-by-step-parse-trace-for--id--id---id-accept)
   - [Phase 6: Parse Trace for Syntax Error `id + * id` (REJECT)](#phase-6-parse-trace-for-syntax-error-id---id-reject)
   - [Phase 7: Dangling-Else Grammar (Left Factoring & Ambiguity Detection)](#phase-7-dangling-else-grammar-left-factoring--ambiguity-detection)
8. [Result](#8-result)

---

## 1. Aim & Objectives

### Aim
To develop a complete, fully functional **LL(1) Parser** for Context-Free Grammars (CFGs).

### Objectives
1. **Remove Left Recursion**: Eliminate direct and indirect left recursion from arbitrary Context-Free Grammars to prevent non-terminating loops during top-down predictive parsing.
2. **Left Factoring**: Factor out common prefixes among alternative productions of each non-terminal so that the parser can deterministically choose the correct production using a single lookahead token.
3. **Construct Parsing Table**: Compute the `FIRST` and `FOLLOW` sets for all grammar symbols and build the 2-dimensional predictive parsing table $M[A, a]$. Detect and report any grammar conflicts (First/First or First/Follow).
4. **Parse Input**: Implement a stack-based pushdown automaton that simulates parsing of tokenized input strings, recording each transition in a detailed trace table and returning a final **Accept** or **Reject** decision.

---

## 2. Theoretical Background of LL(1) Parsing

### What is an LL(1) Parser?
An **LL(1)** parser is a non-backtracking, top-down predictive parser that processes an input string from left to right to construct a leftmost derivation using **one** token of lookahead:
- **First L**: Scans input **L**eft-to-right.
- **Second L**: Constructs a **L**eftmost derivation.
- **(1)**: Uses **1** token of lookahead at each step to deterministically choose which production to apply.

### Necessary & Sufficient Conditions for LL(1)
A Context-Free Grammar $G = (V_N, V_T, P, S)$ is LL(1) if and only if for every pair of distinct productions $A \to \alpha$ and $A \to \beta$ ($A \in V_N$, $\alpha \ne \beta$):
1. $\text{FIRST}(\alpha) \cap \text{FIRST}(\beta) = \emptyset$ (Disjoint FIRST sets: lookahead determines rule uniquely).
2. At most one of $\alpha$ or $\beta$ can derive the empty string $\epsilon$.
3. If $\beta \Rightarrow^* \epsilon$, then $\text{FIRST}(\alpha) \cap \text{FOLLOW}(A) = \emptyset$.

### Why Left Recursion Must Be Eliminated
A production is **left recursive** if it has the form $A \to A\alpha$.
In a recursive descent or top-down predictive parser, when attempting to expand $A$, the parser would repeatedly call or look up $A$ without consuming any input terminal. This causes an **infinite loop** (or infinite stack growth):

$$A \Rightarrow A\alpha \Rightarrow A\alpha\alpha \Rightarrow A\alpha\alpha\alpha \dots$$

Therefore, all direct and indirect left recursions must be removed before constructing an LL(1) parser.

### Why Left Factoring is Necessary
When two productions for a non-terminal share a common prefix:
$$A \to \alpha \beta_1 \mid \alpha \beta_2$$
and the next input token belongs to $\text{FIRST}(\alpha)$, the parser cannot determine whether to expand to $\alpha \beta_1$ or $\alpha \beta_2$ with only 1 token of lookahead.
**Left factoring** defers the decision until the common prefix $\alpha$ has been seen, rewriting the rules as:
$$A \to \alpha A'$$
$$A' \to \beta_1 \mid \beta_2$$

---

## 3. Core Algorithms

### Sub-Task 1: Elimination of Left Recursion

#### (A) Immediate (Direct) Left Recursion
Given a non-terminal $A$ with productions grouped as:
$$A \to A\alpha_1 \mid A\alpha_2 \mid \dots \mid A\alpha_m \mid \beta_1 \mid \beta_2 \mid \dots \mid \beta_k$$
where no $\beta_i$ begins with $A$.
We replace these productions with:
$$A \to \beta_1 A' \mid \beta_2 A' \mid \dots \mid \beta_k A'$$
$$A' \to \alpha_1 A' \mid \alpha_2 A' \mid \dots \mid \alpha_m A' \mid \epsilon$$
*(where $\epsilon$ is represented as `#`)*.

#### (B) General (Indirect) Left Recursion
Given ordered non-terminals $A_1, A_2, \dots, A_n$:
```text
For i = 1 to n:
    For j = 1 to i - 1:
        Replace each production Ai -> Aj gamma by:
            Ai -> delta_1 gamma | delta_2 gamma | ...
            where Aj -> delta_1 | delta_2 | ... are current productions of Aj
    Eliminate immediate left recursion on Ai productions.
```

---

### Sub-Task 2: Left Factoring
Given a non-terminal $A$ with productions $A \to \gamma_1 \mid \gamma_2 \mid \dots \mid \gamma_n$:
```text
Repeat until no common prefixes exist:
    1. Find the longest common prefix alpha (length >= 1) among two or more alternatives of A.
    2. If found for alternatives alpha beta_1, alpha beta_2, ..., alpha beta_p:
         Create new non-terminal A'
         Replace the factored productions with: A -> alpha A'
         Add productions for A': A' -> beta_1 | beta_2 | ... | beta_p
         (If any beta_i is empty, it becomes epsilon '#')
         Retain all remaining non-matching alternatives of A.
```

---

### Sub-Task 3: Computation of FIRST and FOLLOW Sets

#### Computation of FIRST(X)
For every grammar symbol $X \in (V_N \cup V_T \cup \{\epsilon\})$:
1. If $X$ is a terminal, $\text{FIRST}(X) = \{X\}$.
2. If $X = \epsilon$, $\text{FIRST}(\epsilon) = \{\epsilon\}$.
3. If $X \in V_N$:
   - For every production $X \to Y_1 Y_2 \dots Y_k$:
     - If $k = 0$ or production is $X \to \epsilon$, add $\epsilon$ to $\text{FIRST}(X)$.
     - Otherwise, for $i = 1$ to $k$:
       - Add all elements of $(\text{FIRST}(Y_i) \setminus \{\epsilon\})$ to $\text{FIRST}(X)$.
       - If $\epsilon \notin \text{FIRST}(Y_i)$, break.
       - If $\epsilon \in \text{FIRST}(Y_i)$ for all $1 \le i \le k$, add $\epsilon$ to $\text{FIRST}(X)$.
4. Repeat iteratively until no FIRST set changes.

#### Computation of FIRST of a Sequence $\alpha = X_1 X_2 \dots X_n$
- $\text{FIRST}(\alpha) = \emptyset$
- For each $X_i$ in sequence:
  - $\text{FIRST}(\alpha) = \text{FIRST}(\alpha) \cup (\text{FIRST}(X_i) \setminus \{\epsilon\})$
  - If $\epsilon \notin \text{FIRST}(X_i)$, return $\text{FIRST}(\alpha)$.
- Add $\epsilon$ to $\text{FIRST}(\alpha)$ (since all symbols can derive $\epsilon$).
- Return $\text{FIRST}(\alpha)$.

#### Computation of FOLLOW(A)
For every non-terminal $A \in V_N$:
1. If $A = S$ (the start symbol), add `$` (end-of-input marker) to $\text{FOLLOW}(S)$.
2. For each production $B \to \alpha A \beta$:
   - Add all elements of $(\text{FIRST}(\beta) \setminus \{\epsilon\})$ to $\text{FOLLOW}(A)$.
3. For each production $B \to \alpha A$ or $B \to \alpha A \beta$ where $\epsilon \in \text{FIRST}(\beta)$:
   - Add all elements of $\text{FOLLOW}(B)$ to $\text{FOLLOW}(A)$.
4. Repeat iteratively until no FOLLOW set changes.

---

### Sub-Task 4: LL(1) Parsing Table Construction

Let $M[A, a]$ be a table indexed by non-terminal $A \in V_N$ and terminal $a \in (V_T \cup \{\$\})$.
```text
For each production A -> alpha in grammar:
    1. For each terminal a in FIRST(alpha) (where a != epsilon):
           Add (A -> alpha) to M[A, a]
    2. If epsilon in FIRST(alpha):
           For each terminal b in FOLLOW(A) (including $):
               Add (A -> alpha) to M[A, b]
```

#### Conflict Detection
- If any cell $M[A, a]$ contains $> 1$ production rule, the grammar has a **First/First** or **First/Follow conflict** and is **NOT LL(1)**.
- If every cell contains at most 1 production rule, the grammar is **strictly LL(1)**.

---

### Sub-Task 5: Stack-Based Predictive Parsing Simulation

Given tokenized input $w = t_1 t_2 \dots t_n$:
1. Append end-marker `$` to input buffer: $w' = [t_1, t_2, \dots, t_n, \$]$.
2. Initialize pushdown stack: push `$`, push start symbol $S$. (Top of stack is $S$, bottom is `$`.)
3. Let input pointer $ip = 0$.

```text
While stack is not empty:
    Let X = top of stack
    Let a = input_buffer[ip]

    Case 1: (X == '$' and a == '$')
        SUCCESS! String is ACCEPTED. Halt.

    Case 2: (X == a)
        Terminal Match!
        Pop X from stack.
        Increment ip (advance input).
        Action: "Match terminal 'a'"

    Case 3: (X is Non-Terminal)
        Look up M[X, a].
        If M[X, a] is empty:
            SYNTAX ERROR! String is REJECTED.
            Report: "No table entry for M[X, a]". Halt.
        Else:
            Let production be X -> Y1 Y2 ... Yk.
            Pop X from stack.
            If production is NOT X -> epsilon:
                Push Yk, Yk-1, ..., Y1 onto stack (reverse order).
            Action: "Output X -> Y1 Y2 ... Yk"

    Case 4: (X is Terminal and X != a)
        SYNTAX ERROR! String is REJECTED.
        Report: "Mismatched terminal: Expected X, got a". Halt.
```

---

## 4. Project Architecture & File Organization

The `ll1 parser` directory is organized as follows:

```
ll1 parser/
├── QUESTION.md                  # Complete Aim, Objectives, Sub-tasks, and Result definition
├── README.md                    # In-depth theory, algorithms, architecture, and output guide
├── main.cpp                     # Full-featured C++ implementation (Zero external dependencies)
├── ll1_parser.exe               # Pre-compiled high-performance Windows executable
└── sample_inputs/
    ├── grammar1_arithmetic.txt  # Classic arithmetic grammar requiring Left Recursion Removal
    ├── grammar2_dangling_else.txt # Conditional grammar requiring Left Factoring
    ├── grammar3_ll1_direct.txt  # Pre-transformed canonical LL(1) grammar
    └── test_strings.txt         # Collection of valid and invalid test input strings
```

---

## 5. Code Walkthrough

### Grammar Representation
In [`main.cpp`](file:///c:/Users/6217a/Desktop/compiler%20design/ll1%20parser/main.cpp):
- `Production`: Encapsulates left-hand side (`lhs`) and right-hand side token vector (`rhs`).
- `Grammar`: Stores `start_symbol`, ordered vector of `non_terminals`, terminal sets, and map of productions `rules: map<string, vector<vector<string>>>`.
- Epsilon is normalized internally to `EPSILON = "#"`.

### Left Recursion Removal Engine
- `Grammar::removeLeftRecursion()`:
  - Loops through ordered non-terminals $A_i$.
  - Replaces earlier non-terminals $A_j$ ($j < i$) to handle indirect recursion.
  - Partitions rules into recursive $\alpha$ and non-recursive $\beta$.
  - Generates new non-terminal $A_i'$ and assigns transformed rules.

### Left Factoring Engine
- `Grammar::leftFactor()`:
  - Iteratively finds the longest common prefix of length $\ge 1$ across pairs of productions for each non-terminal.
  - Replaces matching productions with $A \to \alpha A'$ and generates $A' \to \beta_1 \mid \beta_2 \dots$.
  - Repeats until no non-terminal has two productions starting with the same symbol.

### FIRST and FOLLOW Computation Engine
- `FirstFollow`:
  - `computeFirst()`: Fixed-point iterative relaxation. Propagates terminals and handles nullable symbols.
  - `firstOfString()`: Evaluates FIRST for arbitrary symbol sequences $\alpha = X_1 X_2 \dots X_k$.
  - `computeFollow()`: Fixed-point algorithm propagating FOLLOW symbols across production bodies.

### Table Generator & Conflict Detector
- `ParsingTable`:
  - Populates 2D table `M[NonTerminal, Terminal]`.
  - Automatically identifies conflicts: checks if any cell has `.size() > 1`.
  - Distinguishes valid LL(1) grammars from non-LL(1) grammars (e.g. dangling-else ambiguity).

### Pushdown Stack Simulation Engine
- `LL1Parser::parse()`:
  - Maintains stack `[$, S]` and input pointer.
  - Records every single transition into `vector<ParseTraceStep>` containing:
    - Step number
    - Stack contents (formatted bottom-to-top)
    - Remaining input buffer
    - Action performed
  - Detects syntax errors, empty table lookups, and terminal mismatches with diagnostics.

---

## 6. Compilation & How to Run

### Step 1: Compile the Code
Using GCC / MinGW:
```powershell
g++ -O2 main.cpp -o ll1_parser.exe
```

### Step 2: Run the Parser

- **Run Full Demonstration (Default)**:
  ```powershell
  .\ll1_parser.exe
  ```
  *(or `.\ll1_parser.exe --demo`)*

- **Parse from a Grammar File with an Input String**:
  ```powershell
  .\ll1_parser.exe --file sample_inputs/grammar1_arithmetic.txt "id + id * id"
  ```

- **Interactive Mode** (enter rules and test strings directly in the terminal):
  ```powershell
  .\ll1_parser.exe --interactive
  ```

---

## 7. Comprehensive Output Walkthrough

Here is the exact verified execution trace produced by the parser:

### Phase 1: Arithmetic Grammar Transformation

#### Original Grammar (Contains Direct Left Recursion)
```text
  E      -> E + T | T
  T      -> T * F | F
  F      -> ( E ) | id
```

#### Step 1: After Removing Left Recursion
```text
  E      -> T E'
  T      -> F T'
  F      -> ( E ) | id
  E'     -> + T E' | #
  T'     -> * F T' | #
```
*(where `#` denotes $\epsilon$)*.

#### Step 2: After Left Factoring
```text
  E      -> T E'
  T      -> F T'
  F      -> ( E ) | id
  E'     -> + T E' | #
  T'     -> * F T' | #
```
*(Grammar has no common prefixes, already factored)*.

---

### Phase 2: FIRST and FOLLOW Sets

| Non-Terminal | FIRST | FOLLOW |
| :--- | :--- | :--- |
| **E** | `{ (, id }` | `{ $, ) }` |
| **T** | `{ (, id }` | `{ $, ), + }` |
| **F** | `{ (, id }` | `{ $, ), *, + }` |
| **E'** | `{ #, + }` | `{ $, ) }` |
| **T'** | `{ #, * }` | `{ $, ), + }` |

---

### Phase 3: LL(1) Parsing Table

| NT \ Term | `(` | `)` | `*` | `+` | `id` | `$` |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **E** | `E -> T E'` | - | - | - | `E -> T E'` | - |
| **T** | `T -> F T'` | - | - | - | `T -> F T'` | - |
| **F** | `F -> ( E )` | - | - | - | `F -> id` | - |
| **E'** | - | `E' -> #` | - | `E' -> + T E'` | - | `E' -> #` |
| **T'** | - | `T' -> #` | `T' -> * F T'` | `T' -> #` | - | `T' -> #` |

> **Status**: Grammar is strictly LL(1). No conflicts detected.

---

### Phase 4: Step-by-Step Parse Trace for `id + id * id` (ACCEPT)

```text
Step  Stack (Bottom->Top)       Input Buffer            Action                            
------------------------------------------------------------------------------------------
1     $ E                       id + id * id $          Output E -> T E'                  
2     $ E' T                    id + id * id $          Output T -> F T'                  
3     $ E' T' F                 id + id * id $          Output F -> id                    
4     $ E' T' id                id + id * id $          Match terminal 'id'               
5     $ E' T'                   + id * id $             Output T' -> #                    
6     $ E'                      + id * id $             Output E' -> + T E'               
7     $ E' T +                  + id * id $             Match terminal '+'                
8     $ E' T                    id * id $               Output T -> F T'                  
9     $ E' T' F                 id * id $               Output F -> id                    
10    $ E' T' id                id * id $               Match terminal 'id'               
11    $ E' T'                   * id $                  Output T' -> * F T'               
12    $ E' T' F *               * id $                  Match terminal '*'                
13    $ E' T' F                 id $                    Output F -> id                    
14    $ E' T' id                id $                    Match terminal 'id'               
15    $ E' T'                   $                       Output T' -> #                    
16    $ E'                      $                       Output E' -> #                    
17    $                         $                       ACCEPT (End of input reached successfully)
------------------------------------------------------------------------------------------
RESULT: >>> ACCEPT <<<
Status Message: String successfully parsed and ACCEPTED.
```

---

### Phase 5: Step-by-Step Parse Trace for `( id + id ) * id` (ACCEPT)

```text
Step  Stack (Bottom->Top)       Input Buffer            Action                            
------------------------------------------------------------------------------------------
1     $ E                       ( id + id ) * id $      Output E -> T E'                  
2     $ E' T                    ( id + id ) * id $      Output T -> F T'                  
3     $ E' T' F                 ( id + id ) * id $      Output F -> ( E )                 
4     $ E' T' ) E (             ( id + id ) * id $      Match terminal '('                
5     $ E' T' ) E               id + id ) * id $        Output E -> T E'                  
6     $ E' T' ) E' T            id + id ) * id $        Output T -> F T'                  
7     $ E' T' ) E' T' F         id + id ) * id $        Output F -> id                    
8     $ E' T' ) E' T' id        id + id ) * id $        Match terminal 'id'               
9     $ E' T' ) E' T'           + id ) * id $           Output T' -> #                    
10    $ E' T' ) E'              + id ) * id $           Output E' -> + T E'               
11    $ E' T' ) E' T +          + id ) * id $           Match terminal '+'                
12    $ E' T' ) E' T            id ) * id $             Output T -> F T'                  
13    $ E' T' ) E' T' F         id ) * id $             Output F -> id                    
14    $ E' T' ) E' T' id        id ) * id $             Match terminal 'id'               
15    $ E' T' ) E' T'           ) * id $                Output T' -> #                    
16    $ E' T' ) E'              ) * id $                Output E' -> #                    
17    $ E' T' )                 ) * id $                Match terminal ')'                
18    $ E' T'                   * id $                  Output T' -> * F T'               
19    $ E' T' F *               * id $                  Match terminal '*'                
20    $ E' T' F                 id $                    Output F -> id                    
21    $ E' T' id                id $                    Match terminal 'id'               
22    $ E' T'                   $                       Output T' -> #                    
23    $ E'                      $                       Output E' -> #                    
24    $                         $                       ACCEPT (End of input reached successfully)
------------------------------------------------------------------------------------------
RESULT: >>> ACCEPT <<<
Status Message: String successfully parsed and ACCEPTED.
```

---

### Phase 6: Parse Trace for Syntax Error `id + * id` (REJECT)

```text
Step  Stack (Bottom->Top)       Input Buffer            Action                            
------------------------------------------------------------------------------------------
1     $ E                       id + * id $             Output E -> T E'                  
2     $ E' T                    id + * id $             Output T -> F T'                  
3     $ E' T' F                 id + * id $             Output F -> id                    
4     $ E' T' id                id + * id $             Match terminal 'id'               
5     $ E' T'                   + * id $                Output T' -> #                    
6     $ E'                      + * id $                Output E' -> + T E'               
7     $ E' T +                  + * id $                Match terminal '+'                
8     $ E' T                    * id $                  ERROR: No entry in M[T, *]        
------------------------------------------------------------------------------------------
RESULT: >>> REJECT <<<
Status Message: Syntax Error: Unexpected token '*' for non-terminal 'T'.
```

---

### Phase 7: Dangling-Else Grammar (Left Factoring & Ambiguity Detection)

#### Original Grammar
```text
  S      -> i E t S | i E t S e S | a
  E      -> b
```

#### After Left Factoring
```text
  S      -> i E t S S' | a
  E      -> b
  S'     -> # | e S
```

#### FIRST and FOLLOW Sets
| Non-Terminal | FIRST | FOLLOW |
| :--- | :--- | :--- |
| **S** | `{ a, i }` | `{ $, e }` |
| **E** | `{ b }` | `{ t }` |
| **S'** | `{ #, e }` | `{ $, e }` |

#### Generated Parsing Table with Ambiguity Detection
```text
NT \ Term   a               b               e               i               t               $               
------------------------------------------------------------------------------------------------------------
S           S -> a          -               -               S -> i E t S S' -               -               
E           -               E -> b          -               -               -               -               
S'          -               -               S' -> # [!]     -               -               S' -> #         

>> Status: Grammar is NOT LL(1). Conflicts detected (Classic Dangling-Else Ambiguity):
   Conflict at M[S', e]: S' -> # vs S' -> e S
```

---

## 8. Result

The **LL(1) Parser** has been completely developed, tested, and verified:
1. **Left Recursion Removal**: Successfully implemented and verified on direct and indirect recursion.
2. **Left Factoring**: Successfully implemented and verified on shared prefix grammar productions.
3. **Parsing Table Construction**: Computed accurate `FIRST` and `FOLLOW` sets, constructed the $M[A, a]$ parsing table, and performed automated conflict detection.
4. **Predictive Parsing Simulation**: Accurately simulated pushdown stack transitions on valid and invalid input strings, generating complete trace tables and determining **`ACCEPT`** or **`REJECT`** results with diagnostic error reporting.

The parser satisfies all lab and theoretical requirements and works successfully.
