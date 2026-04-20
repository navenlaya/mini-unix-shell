# mini-unix-shell

A minimal Unix shell written in C using POSIX system calls.

## Features

- Interactive prompt (`mysh> `)
- Command parsing with whitespace tokenization
- Pipelines (`ls | grep .c | wc -l`)
- I/O redirection (`<`, `>`, `>>`)
- Background execution (`sleep 5 &`)
- Built-in commands (`cd`, `exit`)
- Signal handling (Ctrl+C / Ctrl+Z ignored in shell)
- Automatic zombie reaping via `SIGCHLD`

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
mysh> cd /tmp
mysh> pwd
/tmp
mysh> exit
```

## Project Structure

| File | Purpose |
|------|---------|
| `main.c` | Shell loop, signal setup |
| `parse.c` / `parse.h` | Input tokenization into pipeline struct |
| `execute.c` / `execute.h` | Pipeline execution with fork/pipe/dup2 |
| `builtins.c` / `builtins.h` | Built-in commands (cd) |
| `test.sh` | Automated test suite |
