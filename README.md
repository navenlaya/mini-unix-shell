# mini-unix-shell

A minimal Unix shell written in C using POSIX system calls. Supports pipelines, I/O redirection, background execution, environment variables, glob expansion, and built-in commands.

## Features

- Pipelines — `ls | grep .c | wc -l`
- I/O redirection — `<`, `>`, `>>`
- Background execution — `sleep 5 &`
- Environment variables — `export`, `unset`, `$VAR`, `${VAR}`
- Glob expansion — `ls *.c`, `rm /tmp/*.log`
- Tilde expansion — `cd ~/projects`
- Built-in commands — `cd`, `exit`, `history`, `status`, `export`, `unset`, `help`
- Command history — last 100 commands
- Exit code tracking — `status` prints the last return code
- Signal handling — Ctrl+C / Ctrl+Z don't kill the shell
- Zombie prevention — background children reaped via `SIGCHLD`

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
mysh> ls *.c | wc -l
5
mysh> echo output > file.txt
mysh> cat < file.txt
output
mysh> sleep 5 &
[bg] 12345
mysh> export NAME=mysh
mysh> echo $NAME
mysh
mysh> cd ~/projects
mysh> status
0
mysh> history
  1  echo hello world
  2  ls *.c | wc -l
  ...
mysh> help
mysh> exit
```

## Project Structure

| File | Purpose |
|------|---------|
| `main.c` | Shell loop, signal setup, input handling |
| `parse.c` / `parse.h` | Tokenization, tilde/variable expansion |
| `execute.c` / `execute.h` | Pipeline execution, glob expansion, fork/pipe/dup2 |
| `builtins.c` / `builtins.h` | Built-in commands |
| `history.c` / `history.h` | Command history storage |
| `test.sh` | Automated test suite (31 tests) |
