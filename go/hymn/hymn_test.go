package hymn

import (
	"os"
	"path/filepath"
	"runtime"
	"sort"
	"strings"
	"testing"
	"time"
)

var scripts = filepath.Join("..", "..", "test", "language")

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
			tests = append(tests, path)
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

func test(t *testing.T, file string) error {
	count++
	content, err := os.ReadFile(file)
	if err != nil {
		return err
	}
	source := string(content)
	runes := []rune(source)
	expected := parse(runes)
	if expected == "" {
		return nil
	}
	vm := NewVM()
	var result error
	err = InterpretScript(vm, file, source)
	if strings.HasPrefix(expected, "@exception") {
		if err == nil {
			result = err
		}
	} else {
	}
	if result != nil {
		fail++
	} else {
		success++
	}
	return result
}

func TestHymn(t *testing.T) {
	_, path, _, report := runtime.Caller(0)
	if !report {
		t.Fail()
	}
	err := os.Chdir(filepath.Dir(path))
	if err != nil {
		t.Error(err)
		t.Fail()
	}
	start := time.Now().UnixMilli()
	tests, err := find()
	if err != nil {
		t.Error(err)
		t.Fail()
	}
	sort.Strings(tests)
	for _, file := range tests {
		result := test(t, file)
		if result != nil {
			t.Error(result)
		}
	}
	end := time.Now().UnixMilli()
	t.Logf("\nSuccess: %d, Failed: %d, Count: %d, Time: %d ms\n\n", success, fail, count, end-start)
}
