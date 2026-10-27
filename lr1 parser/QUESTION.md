# LR(1) Parser - Lab Assignment & Problem Statement

## Aim
Develop an **LR(1) Parser** for Context-Free Grammars.

---

## Objectives
1. **Construct LR Items**: Formulate LR(1) items $[A \to \alpha \cdot \beta, \, a]$ combining core dotted productions with a 1-symbol terminal lookahead.
2. **Canonical Collection**: Generate the complete set of LR(1) states using iterative `CLOSURE` and `GOTO` operations over grammar symbols.
3. **ACTION/GOTO Tables**: Construct the parsing action table (`shift`, `reduce`, `accept`, `error`) and non-terminal transition `GOTO` table, checking for any grammar conflicts.
4. **Parse Input**: Implement a shift-reduce pushdown automaton that simulates parsing of tokenized input strings, tracing each step with stack configurations and returning an **Accept** or **Reject** result.

---

## Problem Statement
Implement an **LR(1) Parser** that augments a Context-Free Grammar, computes `FIRST` sets, generates the canonical collection of LR(1) item sets via `CLOSURE` and `GOTO` functions, builds the deterministic `ACTION` and `GOTO` parsing tables, and performs shift-reduce parsing on target input strings with a step-by-step verification trace.

---

## Sub Tasks
1. **Construct LR Items**
   - Add augmented start production: $S' \to S$.
   - Define item structure: $[A \to \alpha \cdot \beta, \, a]$.
   - Implement `CLOSURE(I)`:
     $$\text{For each } [A \to \alpha \cdot B \beta, \, a] \in I, \text{ add } [B \to \cdot \gamma, \, b] \text{ for each } B \to \gamma \text{ and } b \in \text{FIRST}(\beta a)$$

2. **Generate Canonical Collection**
   - Compute initial state: $I_0 = \text{CLOSURE}(\{[S' \to \cdot S, \, \$]\})$.
   - Define $\text{GOTO}(I, X)$ for all grammar symbols $X \in (V_N \cup V_T)$.
   - Generate all canonical states until no new states can be formed.

3. **Build Parsing Tables**
   - **Shift Action**: If $[A \to \alpha \cdot a \beta, \, b] \in I_i$ and $\text{GOTO}(I_i, a) = I_j$, set $\text{ACTION}[i, a] = \text{"s"}j$.
   - **Reduce Action**: If $[A \to \alpha \cdot, \, a] \in I_i$ and $A \ne S'$, set $\text{ACTION}[i, a] = \text{"r"}k$ (where $k$ is rule index).
   - **Accept Action**: If $[S' \to S \cdot, \, \$] \in I_i$, set $\text{ACTION}[i, \$] = \text{"acc"}$.
   - **GOTO Table**: If $\text{GOTO}(I_i, A) = I_j$ for $A \in V_N$, set $\text{GOTO}[i, A] = j$.

4. **Parse String**
   - Maintain state-symbol pushdown stack: initially `[0]`.
   - Read token from input buffer appended with `$`.
   - Execute shift, reduce, or accept according to $\text{ACTION}[\text{top\_state}, \text{current\_token}]$.
   - Record and display full step-by-step trace: `(Step, State Stack, Symbol Stack, Input Buffer, Action)`.

---

## Expected Output
1. Augmented Grammar and Production Indexing.
2. Complete Canonical Collection of LR(1) Item Sets ($I_0, I_1, \dots, I_n$).
3. Formatted `ACTION` and `GOTO` Parsing Tables.
4. Step-by-step parsing trace table:
   - **Step Number**
   - **Stack Contents** (States / Symbols)
   - **Input Buffer**
   - **Action Taken** (Shift / Reduce / Accept / Error)
5. Parsing result: **`ACCEPT`** or **`REJECT`**.

---

## Result
The LR(1) parser works successfully, constructing canonical LR(1) item sets, generating deterministic ACTION and GOTO tables, and parsing target strings with step-by-step verification.
