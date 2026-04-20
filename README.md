# mini-unix-shell

A minimal Unix shell written in C using POSIX system calls.

## Features

- Interactive prompt (`mysh> `)
- Command parsing with whitespace tokenization
- Pipelines (`ls | grep .c | wc -l`)
- I/O redirection (`<`, `>`, `>>`)
- Background execution (`sleep 5 &`)
- Built-in commands (`cd`, `exit`, `history`, `status`)
- Tilde expansion (`~` → `$HOME`)
- Command history (last 100 commands)
- Exit code tracking (`status` prints last exit code)
- Signal handling (Ctrl+C / Ctrl+Z safe)
- Automatic zombie reaping via `SIGCHLD`
- Overlong input detection

## Build

```bash
make
```

## Test

```bash
make test
```

## Usage

```
$ ./mysh
mysh> echo hello world
hello world
mysh> ls | wc -l
12
mysh> echo output > file.txt
mysh> cat < file.txt
output
mysh> sleep 5 &
[bg] 12345
mysh> cd ~/projects
mysh> notarealcmd
notarealcmd: command not found
mysh> status
127
mysh> history
  1  echo hello world
  2  ls | wc -l
  ...
mysh> exit
```

## Project Structure

| File | Purpose |
|------|---------|
| `main.c` | Shell loop, signal setup, input handling |
| `parse.c` / `parse.h` | Tokenization, tilde expansion, pipeline struct |
| `execute.c` / `execute.h` | Pipeline execution with fork/pipe/dup2 |
| `builtins.c` / `builtins.h` | Built-in commands (cd, history, status) |
| `history.c` / `history.h` | Command history storage |
| `test.sh` | Automated test suite (24 tests) |
