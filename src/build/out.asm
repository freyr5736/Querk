global _start
_start:
    mov rax, 1
    push rax
    mov rax, 0
    push rax
    mov rax, 60
    mov rdi, 0
    syscall
