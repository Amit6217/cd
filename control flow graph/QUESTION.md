# Control Flow Graph (CFG) Generation - Lab Assignment & Problem Statement

## Aim
Generate a Control Flow Graph (CFG) from Three-Address Code (TAC).

---

## Objectives
1. **Identify Leaders**: Determine all leader statements in a sequence of Three-Address Code instructions using formal leader identification rules.
2. **Construct Basic Blocks**: Partition the instruction stream into maximal sequences of consecutive statements (Basic Blocks) where control enters solely at the leader and leaves at the final instruction.
3. **Generate CFG**: Construct directed edges between basic blocks reflecting sequential fall-throughs, conditional branches, and unconditional jumps.
4. **Display Graph**: Present the control flow graph using structural adjacency representations, predecessor/successor mappings, and visual ASCII graph diagrams.

---

## Problem Statement
Given an intermediate Three-Address Code program, construct its Control Flow Graph by:
1. Detecting all leader instructions according to formal compiler rules.
2. Partitioning instructions into distinct Basic Blocks ($B_1, B_2, \dots, B_n$).
3. Creating directed control flow edges $(B_i \to B_j)$ representing all possible runtime transfer paths.
4. Displaying the resulting control flow graph along with its predecessor/successor relationships and structural representation.

---

## Sub Tasks
1. **Identify Leaders**
   - Rule 1: The first statement in the TAC sequence is a leader.
   - Rule 2: Any statement that is the target of a conditional or unconditional `goto` is a leader.
   - Rule 3: Any statement that immediately follows a conditional or unconditional `goto` is a leader.

2. **Construct Basic Blocks**
   - For each leader, its basic block includes that leader and all subsequent instructions up to (but not including) the next leader, or the end of the program.
   - Assign block labels: $B_1, B_2, \dots, B_k$.

3. **Generate CFG**
   - Add directed edge $B_i \to B_j$ if:
     - $B_i$ ends with a conditional or unconditional jump to the leader of $B_j$.
     - $B_i$ does not end with an unconditional jump, and $B_j$ immediately follows $B_i$ in sequential order (fall-through).

4. **Display Graph**
   - Display statements with leader flags and identification reasons.
   - Display basic blocks with their constituent instructions.
   - Display directed edges with branch labels (True / False / Unconditional / Fall-through).
   - Display Predecessor and Successor sets for each block.
   - Display an ASCII control flow diagram and DOT Graphviz format.

---

## Expected Output
1. Leader Identification Table (Line #, Statement, Is Leader, Reason).
2. Basic Blocks List ($B_1, B_2, \dots$).
3. Control Flow Graph Edges & Predecessor/Successor Tables.
4. ASCII Graph Representation.

---

## Result
CFG generated successfully.
