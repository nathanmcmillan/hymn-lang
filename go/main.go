package main

import (
	"fmt"
	"hymn-lang/go/hymn"
	"os"
)

func main() {
	args := os.Args

	if len(args) <= 2 {
		fmt.Println("Usage: hymn [-b] [-c] FILE")
		fmt.Println("Interprets a Hymn script FILE.")
		fmt.Println()
		fmt.Println("  -b  Print compiled byte code")
		fmt.Println("  -c  Run FILE as source code")
	}

	vm := hymn.NewVM()

	var err error

	if len(args) >= 4 {
		if args[2] == "-b" {
			if len(args) >= 5 {
				if args[3] == "-c" {
					source := args[4]
					err = hymn.DebugInterpret(vm, source)
				} else {
					fmt.Println("Unknown second argument:", args[3])
				}
			} else {
				script := args[3]
				content, err := os.ReadFile(script)
				if err == nil {
					source := string(content)
					err = hymn.DebugInterpretScript(vm, script, source)
				}
			}
		} else if args[2] == "-c" {
			source := args[2]
			err = hymn.Interpret(vm, source)
		} else {
			fmt.Println("Unknown argument:", args[2])
		}
	} else {
		script := args[2]
		content, err := os.ReadFile(script)
		if err == nil {
			source := string(content)
			err = hymn.InterpretScript(vm, script, source)
		}
	}

	if err != nil {
		fmt.Println(err)
	}
}
