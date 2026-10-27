# LL(1) Parser - Lab Assignment & Problem Statement

## Aim
Develop an **LL(1) Parser** for Context-Free Grammars.

---

## Objectives
1. **Remove Left Recursion**: Eliminate direct and indirect left recursion from given context-free grammar productions to avoid infinite loops in predictive top-down parsing.
2. **Left Factoring**: Transform grammars containing common prefixes among alternatives of a non-terminal to make predictive lookahead decisions unambiguous.
3. **Construct Parsing Table**: Compute `FIRST` and `FOLLOW` sets for all grammar symbols and non-terminals, and systematically build the $M[A, a]$ LL(1) parsing table.
4. **Parse Input**: Implement a stack-based predictive parser that processes input token streams using the generated parsing table, tracing every step and providing clear **Accept / Reject** validation.

---

## Problem Statement
Construct an **LL(1) Parser** that accepts a Context-Free Grammar (CFG), validates and transforms it into LL(1) form by eliminating left recursion and applying left factoring, calculates `FIRST` and `FOLLOW` sets, produces the LL(1) predictive parsing table (detecting any LL(1) grammar conflicts), and simulates parsing of target input strings using an explicit pushdown stack mechanism.

---

## Sub Tasks
1. **Remove Left Recursion**
   - Detect direct left recursion of the form $A \to A\alpha \mid \beta$.
   - Transform productions by introducing new non-terminals:
     $$A \to \beta A'$$
     $$A' \to \alpha A' \mid \epsilon$$
   - Detect and expand indirect left recursions systematically across ordered non-terminals.

2. **Left Factoring**
   - Identify shared prefixes among productions of the same non-terminal: $A \to \alpha\beta_1 \mid \alpha\beta_2 \mid \gamma$.
   - Extract common prefix $\alpha$ and introduce new non-terminals:
     $$A \to \alpha A' \mid \gamma$$
     $$A' \to \beta_1 \mid \beta_2$$

3. **Generate Parsing Table**
   - Compute `FIRST(X)` for all grammar symbols $X \in (V_N \cup V_T)$.
   - Compute `FOLLOW(A)` for all non-terminals $A \in V_N$.
   - For every production $A \to \alpha$:
     - For each terminal $a \in \text{FIRST}(\alpha)$, add $A \to \alpha$ to $M[A, a]$.
     - If $\epsilon \in \text{FIRST}(\alpha)$, add $A \to \alpha$ to $M[A, b]$ for each terminal/marker $b \in \text{FOLLOW}(A)$.
   - Verify if any cell contains multiple entries (LL(1) conflict detection).

4. **Parse String**
   - Initialize stack with bottom marker `$` and start symbol `S`.
   - Read token from input buffer appended with `$`.
   - Iteratively execute:
     - Match terminal at top of stack with current input token.
     - Or expand top non-terminal using table lookup $M[Top, Token]$.
     - Or flag syntax error if cell is empty or mismatch occurs.
   - Record and display full step-by-step trace: `(Step, Stack, Input Buffer, Action)`.

---

## Expected Output
1. Original Grammar vs. Transformed Grammar (after left recursion elimination and left factoring).
2. Complete `FIRST` and `FOLLOW` sets for all non-terminals.
3. Neatly formatted 2D LL(1) Parsing Table $M[NonTerminal, Terminal]$.
4. Step-by-step parsing trace table:
   - **Step Number**
   - **Stack Contents** (bottom to top)
   - **Remaining Input Buffer**
   - **Action Taken** (Production applied / Terminal matched / Error)
5. Final parsing result: **`ACCEPT`** or **`REJECT`** with diagnostic reason.

---

## Result
The LL(1) parser works successfully, performing grammar transformation, table construction, and predictive parsing with full step-by-step verification.
