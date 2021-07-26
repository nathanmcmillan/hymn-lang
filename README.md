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

1. Global `__dir` string to the present working directory
1. Global `__paths` array used as import path priority-based resolver
1. Global `__imports` table that holds current used files. deleting from here will allow reloading a file
1. Expression `try` and `catch` blocks with string exceptions
1. Cache string intern hash
1. Initialize tables

## Long Term Goals

1. Implementations in other languages including Rust, Go, and Java
1. Large standard library written in Hymn
