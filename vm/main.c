#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

// Macros used by the VM
#define VM_STACK_CAPACITY 1024
#define VM_MEMORY_CAPACITY 16777216

// Typedefs
typedef int64_t word;

// Virtual Machine structure
struct _vm_t {
    word stack[VM_STACK_CAPACITY];
    size_t stack_size;

    word memory[VM_MEMORY_CAPACITY];

    size_t instruction_ptr;
    int equal, less, greater, carry;
};

typedef struct _vm_t vm_t;

// Instruction opcode
enum _instruction_type_t {
    INST_PUSH,

    INST_PLUS,
    INST_MINUS,
    INST_MUL,
    INST_DIV,

    INST_DUPL,
    INST_SWAP,
    INST_ROT,

    INST_DROP,

    INST_PRINT_NUM,
    INST_PRINT_CHAR,

    INST_DUMP,

    INST_MSET_ABS,
    INST_MGET_ABS,

    INST_CMP,

    INST_JMP,
    INST_JC,
    INST_JNC,
    INST_JEQ,
    INST_JNE,
    INST_JL,
    INST_JLE,
    INST_JG,
    INST_JGE,

    INST_SET_PTR,
    INST_GET_PTR,
};

typedef enum _instruction_type_t instruction_type_t;

// Instructions
struct _instruction_t {
    instruction_type_t type;
    word operand;
};

typedef struct _instruction_t instruction_t;

enum _vm_error_status_type_t {
    VM_OK,
    VM_STACK_OVERFLOW,
    VM_STACK_UNDERFLOW,
    VM_ILLEGAL_INSTRUCTION,
    VM_ILLEGAL_MEMORY_ACCESS,
};

typedef enum _vm_error_status_type_t vm_error_status_type_t;

// Global VM
vm_t vm = {0};

// Push value onto vm stack
void vm_push(vm_t* vm, word value) 
{
    vm->stack[vm->stack_size] = value;
    vm->stack_size++;
}

// Pop value from vm stack
word vm_pop(vm_t* vm) 
{
    vm->stack_size--;
    word res = vm->stack[vm->stack_size];
    return res;
}

// Get value at top of vm stack
word vm_peek(vm_t* vm) {
    return vm->stack[vm->stack_size-1];
}

// Rotates the top three values on stack
void vm_rot_stack(vm_t* vm) {
    word a = vm_pop(vm);
    word b = vm_pop(vm);
    word c = vm_pop(vm);

    vm_push(vm, a);
    vm_push(vm, b);
    vm_push(vm, c);
}

// Print stack contents
void vm_dump_stack(FILE* stream, vm_t* vm) 
{
    fprintf(stream, "Stack:\n");

    if(vm->stack_size > 0) {
        for(size_t i = vm->stack_size; i > 0; --i) {
            fprintf(stream, "    %ld\n", vm->stack[i-1]);
        }
    } else {
        fprintf(stream, "    [empty]\n");
    }
}

// Executes an instruction
vm_error_status_type_t vm_exec(vm_t* vm, instruction_t inst) 
{
    word a, b;
    vm->instruction_ptr++;

    switch(inst.type) {
        case INST_PUSH:
            if(vm->stack_size >= VM_STACK_CAPACITY - 1) {
                return VM_STACK_OVERFLOW;
            }

            vm_push(vm, inst.operand);
            break;

        case INST_PLUS:
            if(vm->stack_size < 2) {
                return VM_STACK_UNDERFLOW;
            }

            b = vm_pop(vm);
            a = vm_pop(vm);

            // printf("%d + %d\n", a, b);

            if(a + b < a) {
                vm->carry = 1;
            } else {
                vm->carry = 0;
            }

            vm_push(vm, a + b);

            break;

        case INST_MINUS:
            if(vm->stack_size < 2) {
                return VM_STACK_UNDERFLOW;
            }

            b = vm_pop(vm);
            a = vm_pop(vm);

            if(a - b > a) {
                vm->carry = 1;
            } else {
                vm->carry = 0;
            }

            vm_push(vm, a - b);
            break;

        case INST_MUL:
            if(vm->stack_size < 2) {
                return VM_STACK_UNDERFLOW;
            }

            b = vm_pop(vm);
            a = vm_pop(vm);
            
            if(a * b < a) {
                vm->carry = 1;
            } else {
                vm->carry = 0;
            }

            vm_push(vm, a * b);
            break;

        case INST_DIV:
            if(vm->stack_size < 2) {
                return VM_STACK_UNDERFLOW;
            }

            b = vm_pop(vm);
            a = vm_pop(vm);

            vm_push(vm, a % b);
            vm_push(vm, a / b);
            break;

        case INST_DUPL:
            if(vm->stack_size >= VM_STACK_CAPACITY - 1) {
                return VM_STACK_OVERFLOW;
            }

            vm_push(vm, vm_peek(vm));
            break;

        case INST_SWAP:
            if(vm->stack_size < 2) {
                return VM_STACK_UNDERFLOW;
            }

            a = vm_pop(vm);
            b = vm_pop(vm);

            vm_push(vm, a);
            vm_push(vm, b);
            break;

        case INST_ROT:
            if(vm->stack_size < 3) {
                return VM_STACK_UNDERFLOW;
            }

            vm_rot_stack(vm);
            break;

        case INST_DROP:
            if(vm->stack_size < 1) {
                return VM_STACK_UNDERFLOW;
            }

            vm_pop(vm);
            break;
    
        case INST_PRINT_NUM:
            if(vm->stack_size < 1) {
                return VM_STACK_UNDERFLOW;
            }

            printf("%ld\n", vm_pop(vm));

            break;

        case INST_PRINT_CHAR:
            if(vm->stack_size < 1) {
                return VM_STACK_UNDERFLOW;
            }

            a = vm_pop(vm);

            putc((int)a, stdout);
            break;

        case INST_DUMP:
            vm_dump_stack(stdout, vm);
            break;

        case INST_MSET_ABS:
            if(vm->stack_size < 1) {
                return VM_STACK_UNDERFLOW;
            }

            if(inst.operand < 0 || inst.operand >= VM_MEMORY_CAPACITY) {
                return VM_ILLEGAL_MEMORY_ACCESS;
            }

            a = vm_pop(vm);
            vm->memory[inst.operand] = a;

            break;

        case INST_MGET_ABS:
            if(vm->stack_size >= VM_STACK_CAPACITY - 1) {
                return VM_STACK_OVERFLOW;
            }

            if(inst.operand < 0 || inst.operand >= VM_MEMORY_CAPACITY) {
                return VM_ILLEGAL_MEMORY_ACCESS;
            }

            vm_push(vm, vm->memory[inst.operand]);
            break;

        case INST_CMP:
            if(vm->stack_size < 2) {
                return VM_STACK_UNDERFLOW;
            }

            b = vm_pop(vm);
            a = vm_pop(vm);

            if(a == b) {
                vm->equal = 1;
            } else {
                vm->equal = 0;
            }

            if(a < b) {
                vm->less = 1;
            } else {
                vm->less = 0;
            }

            if(a > b) {
                vm->greater = 1;
            } else {
                vm->greater = 0;
            }

            break;

        case INST_JMP:
            vm->instruction_ptr = inst.operand;

            break;

        case INST_JC:
            if(vm->carry) vm->instruction_ptr = inst.operand;
            break;

        case INST_JNC:
            if(!vm->carry) vm->instruction_ptr = inst.operand;
            break;

        case INST_JEQ:
            if(vm->equal) vm->instruction_ptr = inst.operand;
            break;

        case INST_JNE:
            if(!vm->equal) vm->instruction_ptr = inst.operand;
            break;

        case INST_JL:
            if(vm->less) vm->instruction_ptr = inst.operand;
            break;

        case INST_JLE:
            if(vm->less || vm->equal) vm->instruction_ptr = inst.operand;
            break;
        
        case INST_JG:
            if(vm->greater) vm->instruction_ptr = inst.operand;
            break;

        case INST_JGE:
            if(vm->greater || vm->equal) vm->instruction_ptr = inst.operand;
            break;

        case INST_SET_PTR:
            if(vm->stack_size < 2) {
                return VM_STACK_UNDERFLOW;
            }

            b = vm_pop(vm);
            a = vm_pop(vm);

            vm->memory[b] = a;
            break;

        case INST_GET_PTR:
            if(vm->stack_size < 1) {
                return VM_STACK_UNDERFLOW;
            }

            a = vm_pop(vm);
            vm_push(vm, vm->memory[a]);
            break;

        default:
            return VM_ILLEGAL_INSTRUCTION;
    }

    return VM_OK;
}

struct _program_t {
    uint32_t num_instructions;
    instruction_t* instructions;
};

typedef struct _program_t program_t;

program_t vm_read_file(const char* path, vm_t* vm) 
{
    FILE* file = fopen(path, "rb");

    if(!file) {
        fprintf(stderr, "Error: Could not read file '%s'.\n", path);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    program_t res;
    fread(&res.num_instructions, sizeof(int32_t), 1, file);

    res.instructions = (instruction_t*)malloc(16 * res.num_instructions);

    for(int i = 0; i < res.num_instructions; ++i) {
        instruction_t inst;

        fread(&inst.type, sizeof(int32_t), 1, file);
        fread(&inst.operand, sizeof(int32_t), 1, file);

        res.instructions[i] = inst;
    }

    size_t cur = ftell(file);

    size_t bytes = size - cur;
    bytes /= sizeof(int32_t);

    for(int i = 0; i < bytes; ++i) {
        fread(&vm->memory[i], sizeof(int32_t), 1, file);
    }

    fclose(file);

    return res;
}

void vm_exec_program(vm_t* vm, program_t program) 
{
    vm_error_status_type_t status;

    while(vm->instruction_ptr < program.num_instructions) {
        status = vm_exec(vm, program.instructions[vm->instruction_ptr]);

        if(status == VM_STACK_OVERFLOW) {
            printf("Error: Stack overflow.\n");
            vm_dump_stack(stderr, vm);
            return;
        } else if(status == VM_STACK_UNDERFLOW) {
            printf("Error: Stack underflow.\n");
            vm_dump_stack(stderr, vm);
            return;
        } else if(status == VM_ILLEGAL_INSTRUCTION) {
            printf("Error: Illegal instruction.\n");
            vm_dump_stack(stderr, vm);
            return;
        }
    }
}

int main(int argc, char** argv) 
{
    if(argc < 2) {
        fprintf(stderr, "Error: not enough arguments.\n");
        exit(EXIT_FAILURE);
    }

    program_t program = vm_read_file(argv[1], &vm);

    vm_exec_program(&vm, program);

    return 0;
}
