# Virtual Machine
*Fully-Functional, Turing-Complete, Stack-Based Virtual Machine written in pure C.*

## Features:
* Assembler (Python)
* Virtual Machine (C)

## Instructions:
***The Virtual Machine uses different bytecode instructions, as usual.***

* `push <value>`    -> Pushes a value onto the stack
* `add`             -> Adds the top two values on the stack
* `sub`             -> Subtracts the top two values on the stack
* `mul`             -> Multiplies the top two values on the stack
* `div`             -> Divides the top two values on the stack (Remainder is also pushed)
* `dup`             -> Duplicates the value on the top of the stack
* `swap`            -> Swaps the top two values on the stack
* `rot`             -> Rotates the top three values on the stack
* `drop`            -> Drops the top value on the stack
* `print`           -> Prints the value at the top of the stack
* `printc`          -> Prints the character represented by the top of the stack
* `dump`            -> Dumps the stack (Prints every value from top to bottom)
* `mset <addr>`     -> Sets `mem[addr]` to the value at the top of the stack
* `mget <addr>`     -> Pushes `mem[addr]` to the stack
* `cmp`             -> Compares the top two items on the stack
* `jmp <addr>`      -> Unconditional jump to `addr`
* `jc <addr>`       -> Jumps to `addr` if the previous arithmetic operation overflowed
* `jnc <addr>`      -> Jumps to `addr` if the previous arithmetic operation did not overflow
* `je <addr>`       -> Jumps to `addr` if compared numbers were equal
* `jne <addr>`      -> Jumps to `addr` if compared numbers were not equal
* `jl <addr>`       -> Jumps to `addr` if the first number was less than the second in `cmp`
* `jle <addr>`      -> Jumps to `addr` if the first number was less than or equal to the second in `cmp`
* `jg <addr>`       -> Jumps to `addr` if the first number was greater than the second in `cmp`
* `jge <addr>`      -> Jumps to `addr` if the first number was greater than or equal to the second in `cmp`
* `setptr`          -> Sets `mem[stk[sp]]` to `stk[sp - 1]`
* `getptr`          -> Pushes `mem[stk[sp]]` to the stack
