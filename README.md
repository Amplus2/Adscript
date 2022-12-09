# Adscript

A high-performance, [s-expressions](https://en.wikipedia.org/wiki/S-expression)
based programming language that is natively compiled through llvm 12/13.

We have [a rough spec](SPEC.md).

## Installation

```sh
make install -j`nproc`
```

If you're using a modern system and have a newer llvm version installed, you
need to also install llvm 12 or 13 and play some `PATH` chess, something like
these for example:

```sh
# arm64 macos
brew install llvm@13
PATH=/opt/homebrew/opt/llvm@13/bin:$PATH make install -j`nproc`

# arch/manjaro/... linux
pacman -S llvm13
PATH=/usr/lib/llvm13/bin:$PATH make install -j`nproc`

# debian/ubuntu/... linux
apt install llvm-12 llvm-12-dev
PATH=/usr/lib/llvm-12/bin:$PATH make install -j`nproc`
```

The only alternative to this is sending us a PR updating the code to llvm 14/...

## Usage

```sh
adscript [-ehlv] [-o <file>] [-t <target-triple>] <files>
```

- `-e`, `--executable`: generate an executable instead of an object file
- `-l`, `--llvm-ir`: emit llvm ir instead of native code
- `-o <file>`, `--output <file>`: specify an output file
- `-t <t>`, `--target-triple <t>`: specify a target triple to compile for (i.e.
  `i386-linux-elf`)
- `-h`, `--help`: print a bit of help
- `-v`, `--version`: print information about your adscript version
