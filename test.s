memory 
    "Hello, World!\n"
end

push 0
mset 100

label print_lp
    mget 100
    getptr

    dup
    push 0
    cmp

    je print_done

    mget 100
    push 1
    add
    mset 100

    printc
    jmp print_lp

label print_done


