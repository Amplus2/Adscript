# rough Adscript Spec
This specification is not very complete, but still documents Adscript.

## adn
_Adscript Data Notation_ is a modified version of
[EDN](https://github.com/edn-format/edn). It may become its own spec in the
future, but at the moment it is embedded here.

### Data types
| adn type  | implementation                  | example    |
|-----------|---------------------------------|------------|
| `id`      | none (resolved at compile time) | `f`        |
| `char`    | `unsigned int`                  | `\A`       |
| `int`     | `int64_t`                       | `42`       |
| `float`   | `double`                        | `13.37`    |
| `str`     | `char*`                         | `"hi"`     |
| `list`    | none (resolved at compile time) | `(f 1)`    |
| `hetvec`  | `void**`                        | `["hi" 2]` |
| `homovec` | `T*`                            | `#[1 2]`   |

Maps are to be defined. (Probably `{1 2}`)

<!--TODO: go into detail about those-->

## Adscript

### Functions
In Adscript there is another native data type: The function. All functions that
are not natively implemented are "first-class values".

Functions can be created using the `fn` function and called using a `list`:

Defining functions this way is not implemented yet.

```adscript
(fn [<parameters>] <return type> <body>)
```

For example:

```adscript
((fn [int i] int i) 1)
```

### Structs (not implemented yet)
By quoting a list you can create a struct data type.

```adscript
'(int a int b int c)
```

### Additional builtin functions

<!--TODO: a defn++ with c++ mangline-->

#### `def`, `const` (not implemented yet)
Defines a compile-time constant.

```adscript
(def <identifier> <value>)
(const <identifier> <value>)
```

#### `defn`
This defines non-anonymous functions.

```adscript
(defn <identifier> [<parameters>] <return type> <body>)
```

### `let` (not implemented yet)
Defines a "final variable"/"run time constant", works like `let` in Clojure.
```adscript
(let <identifier> <value>)
(let a 42)
```

### `var`
Defines a variable that can be changed later.
```adscript
(var <identifier> <value>)
(var a 42)
```

### `set`
Sets the value for a variable or array element.
```adscript
(set <pointer> <value>)

(set a 42)

(var b #[1 2 4 8 1])
(set (b 4) 16)
```

### `setptr`
Sets the value for a pointer.
```adscript
(setptr <pointer> <value>)

(var i 21)
(setptr (ref i) 42)
```

### `+`, `-`, `*`, `/`, `%`, `|`, `&`, `^`, `~`, `=`, `<`, `>`, `<=`, `>=`, `or`, `and`, `xor`, `not`
These functions are so obvious that they will be documented later.

### `if`
A conditional expression, exactly like in Clojure.

```adscript
(if <condition> <then> <else>)
(if 1 42 10)
```

### `ref`
<!-- This sentence makes absolutely no sense. (TODO: fix it) -->
Creates a pointer to a reference.

```adscript
(ref <value>)
(ref (#[1 2 3] 1))
```

### `deref`
Dereferences a pointer.

```adscript
(deref <pointer>)
(deref (ref ("ABC*" 3)))
```

<!--TODO: prttify this-->

### native-llvm (not implemented yet)
Equivalent to the asm "function" in c with llvm IR.

### native-c (not implemented yet)
Equivalent to the asm "function" in c but with c code.

### native-c++ (not implemented yet)
Equivalent to the asm "function" in c but with c++ code.


