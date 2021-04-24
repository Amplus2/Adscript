# Adscript
A programming language that will be used for Amplus2/Adscriptum.

## Syntax
### Primitive expressions
Integers, floating point values and identifiers are primitive expressions.

### Function calls
```adscript
(<identifier> <parameters>)
```

```adscript

```

### Functions
```adscript
([<parameters>]
    <body>)
```

```adscript
([int a int b]
    (ret (+ a b)))
```

## General
### Expressions
In Adscriptum, functions, function calls and primitive expressions
are expressions.

### Binary expressions
There probably won't be any binary expressions. Instead,
there will be built-in functions named after generally used
binary operators.

## Built-in functions

### +, -, *, /, %
These functions basically just do what their operator does.

### llvm-ir
Equivalent to the asm "function" in c.

### All c std library functions
As long as you have gcc on your system, you
probably will be able to use it.
