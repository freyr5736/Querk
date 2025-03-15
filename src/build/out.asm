global _start
_start:
    mov rax, 2
    push rax
    mov rax, 7
    push rax
    pop rax
    pop rbx
    cqo
    idiv rbx
    push rdx
    mov rax, 10
    push rax
    push QWORD [rsp + 8]
    mov rax, 60
    pop rdi
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
