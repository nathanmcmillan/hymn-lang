package main

import (
	"bufio"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
)

func main() {
	if len(os.Args) != 2 {
		fmt.Println("path?")
		return
	}
	pwd := os.Args[1]
	sprites := spriteBundle(pwd)
	animations := animationBundle(pwd)
	tiles := tileBundle(pwd)
	resources := resourceBundle(pwd)

	var data strings.Builder
	data.WriteString("resources{")
	data.WriteString(resources)
	data.WriteString("},sprites{")
	data.WriteString(sprites)
	data.WriteString("},animations{")
	data.WriteString(animations)
	data.WriteString("},tiles{")
	data.WriteString(tiles)
	data.WriteString("},")

	bundle, err := os.Create(filepath.Join(pwd, "public", "wad"))
	if err != nil {
		panic(err)
	}
	defer bundle.Close()
	_, err = bundle.WriteString(data.String())
	if err != nil {
		panic(err)
	}
}
