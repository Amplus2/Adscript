# Adscript
A high-performance, [s-expressions](https://en.wikipedia.org/wiki/S-expression)
based programming language that is natively compiled.

Note: At least llvm version 11.0 is recommended for building adscipt.
## Syntax

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

## Arrays, Lists and Maps
| Example    | Elements     | Implementation                                   |
|------------|--------------|--------------------------------------------------|
| `(f 1 2)`  | Heterogenous | None (Lists, but only valid for function calls)  |
| `[1 "hi"]` | Heterogenous | Array (`void**`)                                 |
| `#[1 2]`   | Homogenous   | Array (`T*`, where 'T' is the type)              |

Singly-linked lists are currently **not** supported, debates may be had.
Maps are to be defined. (Probably `{1 2}`)

### pointer-index-call
Returns the value at a certain index of a pointer.
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
(if 1 42 10)
```

### ref
Creates a pointer to a reference.

```adscript
(ref <pointer>)
(ref ("ABC*" 3))
```

### deref
Dereferences a pointer.

```adscript
(deref <pointer>)
(deref ("ABC*" 3))
```

### heget
Is short for 'heterogenous array get' and returns the value of a
heterogenous array at a certain index of the specified type.
```adscript
(heget <type> <pointer> <index>)
(heget i8 "ABC*" 3)
```

### defn
Defines a function globally, as described above.

### let (not implemented yet)
Defines a "final variable"/"run time constant", works like `let` in Clojure.
```adscript
(let <identifier> <value>)
(let a 42)
```

### var
Defines a variable that can be changed later.
```adscript
(var <identifier> <value>)
(var a 42)
```

### set
Sets the value for a pointer.
```adscript
(set <pointer> <value>)
(set a 42)
```


### llvm-ir (not implemented yet)
Equivalent to the asm "function" in c with llvm IR.

### native-c (not implemented yet)
Equivalent to the asm "function" in c but with c code.

### All C standard library functions
As long as you have gcc on your system and provide header definitions, you
probably will be able to use it.
<!--TODO: It should work as long as ld and libc are present.-->

