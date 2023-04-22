memory str1 
    "String #1\n"
end

memory str2
    "String #2\n"
end

label main
    push str1
    call puts

    push str2
    call puts

    exit

label puts
    mset 100
    jmp puts_lp
label puts_lp
    mget 100
    getptr

    dup
    push 0
    cmp

    je puts_done

    mget 100
    push 1
    add
    mset 100

    printc
    jmp puts_lp

label puts_done
    ret

