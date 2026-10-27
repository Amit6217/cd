# Intermediate Code Generation - Lab Assignment & Problem Statement

## Aim
Generate intermediate code representations for source program expressions.

---

## Objectives
1. **Generate TAC**: Translate high-level source expressions into linearized Three-Address Code (TAC) statements.
2. **Quadruples**: Construct the Quadruple table representation containing `(op, arg1, arg2, result)` with explicit temporaries.
3. **Triples**: Construct the Triple table representation containing `(op, arg1, arg2)` using positional index references instead of explicit temporary names.
4. **Indirect Triples**: Construct the Indirect Triple representation using a pointer array referencing the triple table to facilitate optimization and instruction reordering.

---

## Problem Statement
Generate intermediate code representations for arithmetic and assignment expressions, producing:
1. Three-Address Code (TAC) sequences
2. Quadruple Table
3. Triple Table
4. Indirect Triple Table

---

## Sub Tasks
1. **Generate TAC**
   - Parse assignment and arithmetic expressions with operator precedence (`*`, `/`, `+`, `-`, unary minus, parentheses).
   - Emit linearized instructions: $t_i = \text{arg}_1 \text{ op } \text{arg}_2$, and final assignment $\text{id} = t_n$.

2. **Generate Quadruples**
   - Construct table with fields: `[Index, Op, Arg1, Arg2, Result]`.
   - Store temporary variable names ($t_1, t_2, \dots$) explicitly in the `Result` field.

3. **Generate Triples**
   - Construct table with fields: `[Index, Op, Arg1, Arg2]`.
   - Replace explicit temporary names with references to preceding triple indices: `(0)`, `(1)`, etc.

4. **Generate Indirect Triples**
   - Separate the execution order array (`Statement List / Pointers`) from the triple definitions table.
   - Display pointer array mapping execution index to triple address.

---

## Expected Output
1. Input Source Expression
2. Three-Address Code (TAC) Sequence
3. Quadruple Table
4. Triple Table
5. Indirect Triple Table

---

## Result
Intermediate code generated successfully across all four representations (TAC, Quadruples, Triples, and Indirect Triples).
