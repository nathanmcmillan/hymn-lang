# Hymn Scripting Language

A small byte-code interpreted language with a focus on simplicity. [Visit the website!](https://hymn-lang.org)

```
# import additional scripts
use "math"

# tables hold key value pairs
function new_node(value) {
  return { value: value, next: none }
}

function node_add(list, value) {
  let node = list
  while true {
    if node.next == none { break }
    node = node.next
  }
  node.next = new_node(value)
}

# objects are passed by reference
let list = new_node("hello")
node_add(list, "world")

# print statements will show all nested values in an object
echo list
```

# Why use Hymn

1. You want reference counting memory management. Memory is released deterministically, as soon as possible. There are no random garbage collection pauses in Hymn.
1. You feel programming paradigms like classes and interfaces add unnecessary complexity. Hymn only has strings, arrays, and tables.
1. You don't need namespaces. The Hymn `use` statement imports all variables and functions globally.
1. You want a scripting language with C like conventions: Brackets indicate scope, indices start at 0, and the not equals operator is `!=`
1. You're weary keeping up with evolving programming languages. Hymn is small and will stay small. There will not be significant changes to the core language and built-in functions.

# Development

## Principles

1. Portable C11 code that passes with `-Wall` `-Wextra` `-Werror` `-pedantic` flags
1. Small implementation under 10,000 total lines of C code
1. Reference counting memory management
1. Easy to learn syntax
1. No closures
1. No namespaces
1. No classes
1. No breaking changes after version 1.0

## Status

| Implementation | Release       |
| -------------- | ------------- |
| C              | Version 0.6.3 |
| JavaScript     | Version 0.6.3 |
| Go             | Incomplete    |
| Rust           | Incomplete    |

# Performance

AMD Ryzen 5 1600 6 core 3.2 GHz - Windows 10 using Ubuntu WSL

| Test    | Factors | Fib   | List  | Loop  | Objects | Primes | Tail  |
| ------- | ------- | ----- | ----- | ----- | ------- | ------ | ----- |
| Hymn    | 1.64s   | 1.55s | 2.29s | 0.97s | 3.70s   | 0.90s  | 3.05s |
| Lua     | 1.14s   | 1.16s | 1.98s | 0.72s | 4.66s   | 0.81s  | 1.81s |
| Python  | 3.98s   | 2.27s | 2.63s | 1.28s | 4.68s   | 2.59s  | -     |
| Node    | 0.69s   | 0.10s | 0.68s | 0.45s | 1.77s   | 0.19s  | -     |
| Lua JIT | 0.08s   | 0.11s | 0.56s | 0.28s | 1.20s   | 0.06s  | 0.03s |
