#!/bin/bash
SHELL=./mysh
PASS=0
FAIL=0
TMPDIR=$(mktemp -d)

run_test() {
    local desc="$1"
    local input="$2"
    local expected="$3"

    local actual
    actual=$(echo "$input" | $SHELL 2>&1 | sed 's/mysh> //g' | sed '/^$/d')

    if [ "$actual" = "$expected" ]; then
        echo "  PASS: $desc"
        PASS=$((PASS + 1))
    else
        echo "  FAIL: $desc"
        echo "    expected: '$expected'"
        echo "    got:      '$actual'"
        FAIL=$((FAIL + 1))
    fi
}

echo "=== basic commands ==="
run_test "echo" "echo hello" "hello"
run_test "multiple args" "echo one two three" "one two three"

echo ""
echo "=== pipes ==="
run_test "single pipe" "echo hello world | wc -w" "2"
run_test "double pipe" "printf 'a\nb\nc\n' | wc -l" "3"

echo ""
echo "=== output redirection ==="
run_test "write to file" "$(printf 'echo test123 > %s/out.txt\ncat %s/out.txt' "$TMPDIR" "$TMPDIR")" "test123"

printf '' > "$TMPDIR/app.txt"
run_test "append to file" "$(printf 'echo line1 > %s/app.txt\necho line2 >> %s/app.txt\ncat %s/app.txt' "$TMPDIR" "$TMPDIR" "$TMPDIR")" "line1
line2"

echo ""
echo "=== input redirection ==="
printf 'alpha\nbravo\ncharlie\n' > "$TMPDIR/in.txt"
run_test "read from file" "wc -l < $TMPDIR/in.txt" "3"
run_test "sort from file" "sort < $TMPDIR/in.txt | head -1" "alpha"

echo ""
echo "=== builtins ==="
run_test "cd to /tmp" "$(printf 'cd /tmp\npwd')" "/tmp"
run_test "cd no args" "$(printf 'cd\npwd')" "$HOME"

echo ""
echo "=== error handling ==="
run_test "bad command" "notarealcmd123" "notarealcmd123: command not found"
run_test "pipe syntax leading" "| ls" "mysh: syntax error near '|'"
run_test "pipe syntax trailing" "ls |" "mysh: syntax error near '|'"
run_test "missing redirect file" "echo hi >" "mysh: syntax error near '>'"
run_test "bad input file" "cat < /tmp/nonexistent_mysh_test_xyz" "/tmp/nonexistent_mysh_test_xyz: No such file or directory"
run_test "cd too many args" "cd /tmp /var" "cd: too many arguments"

echo ""
echo "=== history ==="
run_test "history records commands" "$(printf 'echo aaa\necho bbb\nhistory')" "aaa
bbb
  1  echo aaa
  2  echo bbb
  3  history"

echo ""
echo "=== tilde expansion ==="
run_test "bare tilde" "echo ~" "$HOME"
run_test "tilde slash" "echo ~/foo" "$HOME/foo"
run_test "no expand mid-word" "echo hello~" "hello~"

echo ""
echo "=== status tracking ==="
run_test "status after success" "$(printf 'echo ok\nstatus')" "ok
0"
run_test "status after failure" "$(printf 'notarealcmd123\nstatus')" "notarealcmd123: command not found
127"

echo ""
echo "=== overlong input ==="
LONGSTR=$(python3 -c "print('x' * 2000)")
run_test "rejects overlong line" "$LONGSTR" "mysh: input too long"

echo ""
echo "=== environment variables ==="
run_test "export and expand" "$(printf 'export MYTEST=hello42\necho $MYTEST')" "hello42"
run_test "unset removes var" "$(printf 'export MYTEST=hello42\nunset MYTEST\necho $MYTEST')" ""
run_test "expand unknown var" "echo \$NOSUCHVAR" ""
run_test "mixed text and var" "$(printf 'export FOO=bar\necho prefix_${FOO}_suffix')" 'prefix_bar_suffix'
echo ""
echo "=== glob expansion ==="
GLOBDIR="$TMPDIR/globtest"
mkdir -p "$GLOBDIR"
touch "$GLOBDIR/a.txt" "$GLOBDIR/b.txt" "$GLOBDIR/c.log"
run_test "glob *.txt" "ls $GLOBDIR/*.txt" "$GLOBDIR/a.txt
$GLOBDIR/b.txt"
run_test "glob no match stays literal" "echo /tmp/nonexistent_glob_mysh_test*" "/tmp/nonexistent_glob_mysh_test*"

echo ""
echo "=== help ==="
HELP_OUT=$(echo "help" | $SHELL 2>&1 | sed 's/mysh> //g' | head -1)
if [ "$HELP_OUT" = "mysh - mini unix shell" ]; then
    echo "  PASS: help shows header"
    PASS=$((PASS + 1))
else
    echo "  FAIL: help shows header"
    echo "    expected: 'mysh - mini unix shell'"
    echo "    got:      '$HELP_OUT'"
    FAIL=$((FAIL + 1))
fi

echo ""
echo "=== exit ==="
run_test "exit cleanly" "exit" ""

rm -rf "$TMPDIR"

echo ""
echo "===================="
echo "Results: $PASS passed, $FAIL failed"
[ $FAIL -eq 0 ] && exit 0 || exit 1
