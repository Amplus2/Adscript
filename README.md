# Adscript
A high-performance, [s-expressions](https://en.wikipedia.org/wiki/S-expression)
based programming language that is natively compiled.

## Syntax
### Lists, Arrays, Maps

| Example  | Elements     | Implementation        |
|----------|--------------|-----------------------|
| `(1 2)`  | Heterogenous | Singly-linked list    |
| `#(1 2)` | Homogenous   | Singly-linked list    |
| `[1 2]`  | Heterogenous | Vector/Growable array |
| `#[1 2]` | Homogenous   | Vector/Growable array |

If you want to use a heterogenous singly-linked list as a literal, you will have
to _quote_ it: `'(1 2 3)`

Maps are to be defined.

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
(defn [<keywords>] <id> [<parameters>] [<return type>]
    <body>)
```

```adscript
(defn :export iadd [int a int b] int
    (- a b)     ;; this is useless
    (* a b)     ;; this is also useless
    (+ a b))    ;; this is returned
```

## Built-in functions

### +, -, *, /, %
These functions basically just do what their operator does.

### defn
Defines a function globally, as described above.

### let
Defines a "final variable"/"run time constant", works like `let` in Clojure.

### var
Defines a variable that can be changed later.

### llvm-ir
Equivalent to the asm "function" in c.

### All C standard library functions
As long as you have gcc on your system, you
probably will be able to use it.
<!--TODO: It should work as long as ld and libc are present.-->
