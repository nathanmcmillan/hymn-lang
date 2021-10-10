grep -cve '^\s*$' src/*.c src/*.h

echo ""

grep -v '^\s*$' src/*.c src/*.h | wc -l
