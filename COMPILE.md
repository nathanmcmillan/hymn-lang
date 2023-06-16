# Compiling

## Linux + GCC

### Debug

```
$ gcc src/*.c -std=c11 -Wall -Wextra -Werror -pedantic -Wpadded -Wundef -Wpointer-arith -Wunreachable-code -Wuninitialized -Winit-self -Wmissing-include-dirs -Wswitch-default -Wunused -Wunused-parameter -Wunused-variable -Wunused-value -Wshadow -Wconversion -Wcast-qual -Wcast-align -Wwrite-strings -Wlogical-op -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Winline -Wvla -Woverlength-strings -Wstrict-overflow=5 -g -o hymn -lm -ldl -rdynamic
```

### Testing

```
$ gcc test/*.c src/*.c -std=c11 -Wall -Wextra -Werror -pedantic -Wno-unused-function -g -DHYMN_TESTING -Isrc -o hymntest -lm -ldl -rdynamic
```

### Benchmark

```
gcc test/*.c src/*.c -std=c11 -O3 -s -DNDEBUG -DHYMN_NO_CLI -DHYMN_NO_TEST -DHYMN_BENCHMARK -Isrc -o hymnbenchmark -lm -ldl
```

### Release

```
$ gcc src/*.c -std=c11 -O3 -s -DNDEBUG -o hymn -lm -ldl
```

### Formatter

```
$ gcc format/*.c -std=c11 -Wall -Wextra -Werror -pedantic -Wno-unused-function -g -o hymnfmt
```

### Dynamic Library

1. Compile the dynamic library C file into an object file. The will create `dynamic.o`

```
$ gcc -Isrc -fpic -c test/dynamic/dynamic.c
```

2. Compile the object file into a shared object file. This will create `dynamic.so`

```
$ gcc -shared -lc -o dynamic.so dynamic.o
```

3. Load `dynamic.so` and use in Hymn script

```
> ./hymn
> use "dynamic"
```

## Windows + MSVC

### Debug

```
> cl src/*.c /W4 /WX /wd4996 /link /out:HYMN.exe
```

### Testing

```
> cl test/*.c src/*.c /Isrc /W4 /WX /wd4996 -DHYMN_TESTING /link /out:HYMNTEST.exe
```

### Release

```
> cl src/*.c /O2 /DNDEBUG /link /out:HYMN.exe
```

### Formatter

```
> cl format/*.c /W4 /WX /wd4996 /link /out:HYMNFMT.exe
```

### Dynamic Library

1. Compile Hymn as a library. This will create `hymn.dll`, `hymn.lib` and `hymn.exp`

```
> cl src/hymn.c /W4 /WX /wd4996 /LD
```

2. Compile the dynamic library C file against `hymn.lib`. This will create `dynamic.dll`, `dynamic.lib`, and `dynamic.exp`

```
> cl /LD /Isrc hymn.lib test\\dynamic\\dynamic.c
```

3. Load `dynamic.dll` and use in Hymn script

```
> HYMN.exe
> use "dynamic"
```

## Windows + Clang

### Debug

```
> clang src/*.c -Isrc -std=c11 -Wall -Wextra -Weverything -Werror -pedantic -Wduplicate-decl-specifier -Wdeprecated-register -Wempty-body -Wnull-dereference -Wparentheses -Wno-deprecated-declarations -Wno-gnu-zero-variadic-macro-arguments -Wno-language-extension-token -Wno-float-equal -Wno-bad-function-cast -Wno-switch-enum -Wno-covered-switch-default -Wno-format-nonliteral -o HYMN.exe
```

### Testing

```
> clang test/*.c src/*.c -Isrc -std=c11 -Wall -Wextra -Werror -pedantic -Wno-unused-function -Wno-deprecated-declarations -Wno-gnu-zero-variadic-macro-arguments -Wno-language-extension-token -DHYMN_TESTING -o HYMNTEST.exe
```

### Release

```
> clang -O2 src/*.c -Isrc -std=c11 -Wno-deprecated-declarations -o HYMN.exe
```

### Formatter

```
> clang format/*.c -std=c11 -Wall -Wextra -Werror -pedantic -Wno-unused-function -Wno-deprecated-declarations -Wno-gnu-zero-variadic-macro-arguments -Wno-language-extension-token -o HYMNFMT.exe
```

## Node

### Testing

```
$ node js/test.js
```

# Profiling on Linux

```
$ gcc src/*.c -O0 -pg -o profile -lm
$ ./profile [FILE].hm
$ gprof profile gmon.out | less
```

```
$ valgrind --tool=callgrind ./hymn [FILE].hm
$ callgrind_annotate --auto=yes callgrind.out.* | less
```

# Memory Leaks on Linux

```
$ valgrind --leak-check=full --show-leak-kinds=all -s ./hymn [FILE].hm
```
