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

1. Expression `try` and `catch` blocks with string exceptions
1. Handle memory leaks during compile and runtime exceptions
1. More constants than 256
1. Intern non-string constants
1. Compile errors should be wrapped inside runtime errors so that they can be caught

## Long Term Goals

1. Implementations in other languages including Rust, Go, and Java
1. Large standard library written in Hymn
