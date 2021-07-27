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
1. Tables and arrays with initialized items `[1, 2, 3]` `{foo: bar, hello: world}`

## Long Term Goals

1. Implementations in other languages including Rust, Go, and Java
1. Large standard library written in Hymn
