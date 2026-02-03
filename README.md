================================================================================
End-to-End EDM Framework - Lab 6 Submission
================================================================================

1. Build Instructions

---

Prerequisites: GCC, G++, Flex, Bison, Make.

To build the entire system, run:
$ make clean
$ make

This will generate the executable `edm_shell`.

2. How to Start

---

Run the shell executable:
$ ./edm_shell

3. Command Reference

---

- submit <file.edm> : Register a new program (Returns PID).
- run <pid> : Execute the program to completion.
- debug <pid> : Attach debugger to the program (Starts in PAUSED state).
- memstat <pid> : Show current heap usage and leak report.
- gc <pid> : Force garbage collection.
- kill <pid> : Terminate a program and free its resources.
- quit : Exit the shell.

4. Example Workflows

---

[Scenario A: Running and Checking Memory]
mysh> submit test1.edm
PID = 1
mysh> run 1
mysh> memstat 1
(Shows leaked objects = 3, representing variables x, y, z)
mysh> gc 1
(Garbage collection runs, globals are preserved)

[Scenario B: Debugging a Loop]
mysh> submit test2.edm
PID = 2
mysh> debug 2
(dbg) break 3
(dbg) continue
(Breakpoint hit at Line 3)
(dbg) state
(Shows stack contents)
(dbg) step
(dbg) continue
(Breakpoint hit again - next iteration)
(dbg) quit

5. Files Description

---

- src/shell/ : Shell implementation (C++).
- src/compiler/ : Lexer (Flex) and Parser (Bison).
- src/core/ : AST, IR, and Compiler glue code.
- src/debugger/ : Virtual Machine and Garbage Collector.
- tests/ : Example .edm programs.

6. Notes for Evaluator

---

The system strictly enforces component interdependence. The Debugger relies
on Parser metadata. The VM relies on the Linker for control flow. Memory
analysis relies on VM execution state.
