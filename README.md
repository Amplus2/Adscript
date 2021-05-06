# Adscript
A high-performance, [s-expressions](https://en.wikipedia.org/wiki/S-expression)
based programming language that is natively compiled.

## Syntax
### Lists, Arrays, Maps

| Example    | Elements     | Implementation                                   |
|------------|--------------|--------------------------------------------------|
| `(f 1 2)`  | Heterogenous | None (Lists, but only valid for function calls)  |
| `[1 "hi"]` | Heterogenous | Array (`std::vector<void*>`)                     |
| `#[1 2]`   | Homogenous   | Array (`std::vector<T>`, where `T`` is the type) |

Maps are to be defined. (Probably `{1 2}`)
Singly-linked lists are currently **not** supported, debates may be had.

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
(defn [<parameters>] <id> <return type>
    <body>)
```

```adscript
(defn iadd [int a int b] int
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
Equivalent to the asm "function" in c with llvm IR.

### native-c
Equivalent to the asm "function" in c with c code.

### All C standard library functions
As long as you have gcc on your system, you
probably will be able to use it.
<!--TODO: It should work as long as ld and libc are present.-->
