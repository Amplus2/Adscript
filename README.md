# Adscript
A high-performance, [s-expressions](https://en.wikipedia.org/wiki/S-expression)
based programming language that is natively compiled.

## Syntax

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
(defn <id> [<parameters>] <return type>
    <body>)
```

```adscript
(defn iadd [int a int b] int
    (- a b)     ;; this is useless
    (* a b)     ;; this is also useless
    (+ a b))    ;; this is returned
```

## Arrays
| Example    | Elements     | Implementation                                   |
|------------|--------------|--------------------------------------------------|
| `(f 1 2)`  | Heterogenous | None (Lists, but only valid for function calls)  |
| `[1 "hi"]` | Heterogenous | Array (`void**`)                     |
| `#[1 2]`   | Homogenous   | Array (`T*`, where 'T' is the type) |

### pointer-index-call
Returns the pointer to a value at a certain index of a pointer.
```adscript
(<pointer> <index>)
```

## Built-in functions

### +, -, *, /, %, |, &, =, <, >, <=, >=, or, and, xor, not
These functions basically just do what their operator does.

### if
Creates a conditional expression.

```adscript
(if <condition> <true-value> <false-value>)
```

### deref
Dereferences a pointer.

```adscript
(deref <pointer>)
```

### heget
Is short for 'heterogenous array get' and returns the value of a
heterogenous array at a certain index of the specified type.
```adscript
(heget <type> <pointer> <index>)
```

### defn
Defines a function globally, as described above.

### let (not implemented yet)
Defines a "final variable"/"run time constant", works like `let` in Clojure.
```adscript
(let <identifier> <value>)
```

### var
Defines a variable that can be changed later.
```adscript
(var <identifier> <value>)
```

### set
Sets the value for a pointer.
```adscript
(set <pointer> <value>)
```


### llvm-ir (not implemented yet)
Equivalent to the asm "function" in c with llvm IR.

### native-c (not implemented yet)
Equivalent to the asm "function" in c but with c code.

### All C standard library functions
As long as you have gcc on your system and provide header definitions, you
probably will be able to use it.
<!--TODO: It should work as long as ld and libc are present.-->

