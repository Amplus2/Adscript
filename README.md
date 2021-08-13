# Adscript
A high-performance, [s-expressions](https://en.wikipedia.org/wiki/S-expression)
based programming language that is natively compiled through llvm >= 12.

We have [a rough spec](SPEC.md).

# Usage

```sh
adscript [-ehlv] [-o <file>] [-t <target-triple>] <files>
```

- `-e`, `--executable`: generate an executable instead of an object file
- `-l`, `--llvm-ir`: emit llvm ir instead of native code
- `-o <file>`, `--output <file>`: specify an output file
- `-t <t>`, `--target-triple <t>`: specify a target triple to compile for (i. e. "i386-linux-elf")
- `-h`, `--help`: print a bit of help
- `-v`, `--version`: print information about your adscript version

# Installation
```sh
make install
```
