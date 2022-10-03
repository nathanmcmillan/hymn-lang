package main

import (
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"time"
)

var scripts = filepath.Join("test", "language")

var (
	success = 0
	fail    = 0
	count   = 0
)

func find() ([]string, error) {
	tests := []string{}
	err := filepath.Walk(scripts, func(path string, file os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if !file.IsDir() {
			tests = append(tests, filepath.Join(scripts, file.Name()))
		}
		return nil
	})
	if err != nil {
		return nil, err
	}
	return tests, nil
}

func parse(source []rune) string {
	expected := ""
	size := len(source)
	for pos := 0; pos < size; pos++ {
		c := source[pos]
		if c == '#' && pos+1 < size {
			if source[pos+1] == ' ' {
				pos += 2
				for pos < size {
					c = source[pos]
					expected += string(c)
					if c == '\n' {
						break
					}
					pos++
				}
			} else if source[pos+1] == '\n' {
				pos++
				expected += "\n"
				continue
			}
		}
		break
	}
	return strings.Trim(expected, " ")
}

func testFile(file string) string {
	count++
	content, err := os.ReadFile(file)
	if err != nil {
		return err.Error()
	}
	source := []rune(string(content))
	expected := parse(source)
	if expected == "" {
		return ""
	}
	result := ""
	if result != "" {
		fail++
	} else {
		success++
	}
	return result
}

func main() {
	path, err := os.Executable()
	if err != nil {
		panic(err)
	}
	err = os.Chdir(filepath.Dir(path))
	if err != nil {
		panic(err)
	}
	fmt.Println()
	start := time.Now().UnixMilli()
	tests, err := find()
	if err != nil {
		panic(err)
	}
	sort.Strings(tests)
	for _, test := range tests {
		fmt.Println(test)
		result := testFile(test)
		if result != "" {
			fmt.Println(result)
		}
	}
	end := time.Now().UnixMilli()
	fmt.Printf("\nSuccess: %d, Failed: %d, Count: %d, Time: %d ms\n\n", success, fail, count, end-start)
}
