# Hymn Scripting Language

A small byte-code interpreted language with a focus on simplicty. [Visit the website!](https://hymn-lang.org)

## Goals

1. Portable C11 code that passes with `-Wall` `-Wextra` `-Werror` `-pedantic` flags
1. Small implementation under 4,000 semicolons
1. Reference counting memory management
1. No closures
1. No namespaces
1. No classes
1. No new langauage features
1. Long term compatibility
1. Emphasis on writing simple and readable scripts
1. Emphasis on one way to do things right

## Development

1. Handle memory leaks during compile and runtime exceptions
1. More constants than 256
1. Intern non-string constants
1. Compile errors should be wrapped inside runtime errors so that they can be caught (when they're in an import)
1. A failed malloc, calloc, etc should return a compile or runtime error if possible

#### Exceptions

Compile errors are thrown as syntax errors so they can be caught at runtime during imports.

```
/absolute/path/to/file.hm

Expected 2 arguments for `index` function.

7|  index(0)
         ^^^

Hint: The `index` function expects an array or table for the 1st argument, and an integer or string for the 2nd argument.

/absolute/path/to/file.hm <function foo> line 28
/absolute/path/to/file.hm <function bar> line 31

```

## Long Term Goals

1. Implementations in other languages including Rust, Go, and Java
1. Large standard library written in Hymn
