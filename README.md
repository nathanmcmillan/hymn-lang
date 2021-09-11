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

1. More constants than 256
1. Intern integer and boolean constants
1. A failed malloc, calloc, etc should return a compile or runtime error if possible

| Implementation | Status          |
| -------------- | --------------- |
| C              | Complete        |
| JavaScript     | Almost complete |
| Rust           | Started         |

## Long Term Goals

1. Implementations in other languages including Rust, Go, and Java
1. Large standard library written in Hymn
