# Adscript
A high-performance, [s-expressions](https://en.wikipedia.org/wiki/S-expression)
based programming language that is natively compiled.

## Syntax
### Primitive expressions
Integers, floating point values and identifiers are primitive expressions.

### Function calls
```adscript
(<identifier> <parameters>)
```

```adscript
(+ 40 2)
(var a 42)
```

### Functions
```adscript
(<id> [<parameters>] <return type>
    <body>)
```

```adscript
(iadd [int a int b] int
    (- a b)     ;; this is useless
    (* a b)     ;; this is also useless
    (+ a b))    ;; this is returned
```

## General
### Expressions
In Adscript, functions, function calls and primitive expressions
are expressions.

## Built-in functions

### +, -, *, /, %
These functions basically just do what their operator does.

### llvm-ir
Equivalent to the asm "function" in c.

### All C standard library functions
As long as you have gcc on your system, you
probably will be able to use it.
<!--TODO: It should work as long as ld and libc are present.-->
