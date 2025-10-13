# IFJ25 Compiler

## Setup

```bash
./dev.sh build
```

## Commands

```bash
./dev.sh start       # Start dev container
./dev.sh compile     # Build the compiler
./dev.sh test        # Run tests
./dev.sh clean       # Clean build artifacts
./dev.sh stop        # Stop containers
```

## Inside Container

```bash
make clean all              # Compile
./compiler source.ifj       # Run compiler
valgrind ./compiler test.ifj # Check memory leaks
```

## Project Structure

```
src/        # Source code
tests/      # Test files
examples/   # Example programs
documents/  # Documentation
```
