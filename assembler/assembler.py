#!/bin/python3
######################
#   Assembler
######################

import os
import sys
import array
import pathlib

from enum import IntEnum, auto

class Inst(IntEnum):
    INST_PUSH = auto()

    INST_PLUS = auto()
    INST_SUB  = auto()
    INST_MUL  = auto()
    INST_DIV  = auto()

    INST_DUPL = auto()
    INST_SWAP = auto()
    INST_ROT  = auto()
    INST_DROP = auto()

    INST_PRINT_NUM = auto()
    INST_PRINT_CHAR = auto()

    INST_DUMP = auto()
    
    INST_MSET_ABS = auto()
    INST_MGET_ABS = auto()

    INST_CMP = auto()

    INST_JMP_ABS = auto()
    INST_JC = auto()
    INST_JNC = auto()
    INST_JE = auto()
    INST_JNE = auto()
    INST_JL = auto()
    INST_JLE = auto()
    INST_JG = auto()
    INST_JGE = auto()

    INST_SET_PTR = auto()
    INST_GET_PTR = auto()

    INST_CALL = auto()
    INST_RETURN = auto()
    INST_EXIT = auto()

def read_file(filename: str) -> str:
    contents = ""

    with open(filename, "r") as f:
        lines = f.readlines()

        for i, v in enumerate(lines):
            lines[i] = v.split(";")[0]

    contents = "\n".join(lines + ['\n'])

    return contents

def write_file(filename: str, program) -> None:
    new_path = str(pathlib.Path(filename).with_suffix(""))

    print(sys.byteorder)

    tmp = array.array("I")
    tmp.fromlist(program)

    if sys.byteorder == "big":
        tmp.byteswap()

    program = tmp.tolist()
    program = [0x565343] + program

    with open(new_path, "wb") as f:
        arr = array.array("I")
        arr.fromlist(program)

        if sys.byteorder == "big":
            arr.byteswap()

        arr.tofile(f)

class Lexer:
    def __init__(self, input_str: str) -> None:
        self.input_str = input_str

        self.letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
        self.digits  = "0123456789"

    def get_next_char(self):
        c = self.input_str[0]

        self.input_str = self.input_str[1:]
        return c

    def at(self):
        return self.input_str[0]

    def push_left(self, char: str):
        self.input_str = char + self.input_str

    def is_not_empty(self):
        return len(self.input_str) > 0

    def make_word(self, start_char):
        res = start_char
        tok_type = "Word"

        c = start_char

        while self.is_not_empty() and (c in self.letters or c in self.digits or c == "_"):
            c = self.get_next_char()

            res += c

            c = self.at()

        return (tok_type, res)

    def make_number(self, start_char):
        res = start_char
        tok_type = "Number"

        c = start_char

        while self.is_not_empty() and c in self.digits:
            c = self.get_next_char()

            res += c

            c = self.at()

        return (tok_type, res)

    def make_char(self):
        res = ""

        tok_type = "Number"

        c = ""

        while self.is_not_empty() and not (c in ["'"]):
            c = self.get_next_char()

            res += c

        res = res.replace("\\n", "\n").replace("\\t", "\t").replace("\\r", "\r")

        return (tok_type, ord(res[:-1]))

    def make_str(self):
        res = ""

        tok_type = "String"

        c = ""

        while self.is_not_empty() and not (c in ["\""]):
            c = self.get_next_char()
            res += c

        res = res.replace("\\n", "\n").replace("\\t", "\t").replace("\\r", "\r")
        return (tok_type, res[:-1])

    def get_token(self):
        next_char = self.get_next_char()

        while self.is_not_empty() and next_char in [" ", "\n", "\t"]:
            next_char = self.get_next_char()

        if next_char in self.letters:
            return self.make_word(next_char)

        if next_char in self.digits:
            return self.make_number(next_char)

        if next_char == "'":
            return self.make_char()

        if next_char == "\"":
            return self.make_str()

        return ("EOF", "")

    def lex(self):
        tokens = []

        print("Tokenizing input...")

        while self.is_not_empty():
            tokens.append(self.get_token())

        return tokens

class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.debug = False

        self.labels = {}
        self.index = 0

        self.memory = []

        self.inst_map = {
            "push": Inst.INST_PUSH,

            "add":  Inst.INST_PLUS,
            "sub":  Inst.INST_SUB,
            "mul":  Inst.INST_MUL,
            "div":  Inst.INST_DIV,

            "dup":  Inst.INST_DUPL,
            "swap": Inst.INST_SWAP,
            "rot":  Inst.INST_ROT,
            "drop": Inst.INST_DROP,

            "print": Inst.INST_PRINT_NUM,
            "printc": Inst.INST_PRINT_CHAR,

            "dump": Inst.INST_DUMP,

            "mset": Inst.INST_MSET_ABS,
            "mget": Inst.INST_MGET_ABS,

            "cmp": Inst.INST_CMP,

            "jmp": Inst.INST_JMP_ABS,
            "jc": Inst.INST_JC,
            "jnc": Inst.INST_JNC,
            "je": Inst.INST_JE,
            "jne": Inst.INST_JNE,
            "jl": Inst.INST_JL,
            "jle": Inst.INST_JLE,
            "jg": Inst.INST_JG,
            "jge": Inst.INST_JGE,

            "setptr": Inst.INST_SET_PTR,
            "getptr": Inst.INST_GET_PTR,

            "call": Inst.INST_CALL,
            "ret": Inst.INST_RETURN,
            "exit": Inst.INST_EXIT,
        }

        self.op_count = {
            Inst.INST_PUSH: 1,

            Inst.INST_PLUS: 0,
            Inst.INST_SUB:  0,
            Inst.INST_MUL:  0,
            Inst.INST_DIV:  0,

            Inst.INST_DUPL: 0,
            Inst.INST_SWAP: 0,
            Inst.INST_ROT:  0,
            Inst.INST_DROP: 0,

            Inst.INST_PRINT_NUM: 0,
            Inst.INST_PRINT_CHAR: 0,

            Inst.INST_DUMP: 0,

            Inst.INST_MSET_ABS: 1,
            Inst.INST_MGET_ABS: 1,

            Inst.INST_CMP: 0,

            Inst.INST_JMP_ABS: 1,
            Inst.INST_JC: 1,
            Inst.INST_JNC: 1,
            Inst.INST_JE: 1,
            Inst.INST_JNE: 1,
            Inst.INST_JL: 1,
            Inst.INST_JLE: 1,
            Inst.INST_JG: 1,
            Inst.INST_JGE: 1,

            Inst.INST_SET_PTR: 0,
            Inst.INST_GET_PTR: 0,

            Inst.INST_CALL: 1,
            Inst.INST_RETURN: 0,
            Inst.INST_EXIT: 0 
        }

    def is_not_empty(self):
        return len(self.tokens) > 0

    def get_next_token(self):
        tok = self.tokens[0]
        self.tokens = self.tokens[1:]

        return tok

    def at(self):
        return self.tokens[0]

    def parse_next(self):
        (tok_type, tok_value) = self.get_next_token()

        if tok_type == "Word":
            if tok_value in self.inst_map:
                mapped = self.inst_map[tok_value]

                if self.op_count[mapped] == 1:
                    self.index += 1
                    n = self.parse_next()
                    if self.debug: print(mapped, n)
                    return [int(mapped)-1] + n

                else:
                    self.index += 1
                    if self.debug: print(mapped)
                    return [int(mapped)-1, 0]

            elif tok_value in self.labels:
                return [ self.labels[tok_value] ]

            elif tok_value == "memory":
                (ntype, nval) = self.at()

                if ntype == "Word":
                    self.labels[nval] = len(self.memory)
                    self.get_next_token()

                    (ntype, nval) = self.at()
                
                while nval != "end":
                    self.memory += self.parse_next()
                    (ntype, nval) = self.at()
                
                return []

            else:
                return []

        if tok_type == "Number":
            return [ int(tok_value) ]

        if tok_type == "String":
            return [ ord(c) for c in tok_value ] + [0]

        if tok_type == "EOF":
            return []

    def get_labels(self):
        i = 0
        while i < len(self.tokens):
            (tok_type, tok_value) = self.tokens[i]

            if tok_type == "Word":
                if tok_value in self.inst_map:
                    self.index += 1

                elif tok_value == "label":
                    (name_type, name_value) = self.tokens[i+1]

                    self.tokens.pop(i)
                    self.tokens.pop(i)

                    self.labels[name_value] = self.index
                    # print(self.labels)
                    continue

            i += 1

    def parse(self):
        program = []
        print("Parsing (Pass 1)...")
        self.get_labels()
        # print(self.tokens)

        print("Parsing (Pass 2)...")
        while self.is_not_empty():
            program += self.parse_next()

        num_instructions = int(len(program) / 2)

        program += self.memory

        program = [num_instructions] + program

        return program

if len(sys.argv) < 2:
    print("Usage: assembler <filename>")
    exit(1)

lexer = Lexer(read_file(sys.argv[1]))
tokens = lexer.lex()

parser = Parser(tokens)
program = parser.parse()

print("Writing Output...")
write_file(sys.argv[1], program)
