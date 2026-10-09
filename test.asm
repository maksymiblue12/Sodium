.intel_syntax noprefix

.global _start
_start:
    mov rax, 15
    push rax

    mov rax, 69
    push rax

    push qword ptr [rsp+0]

    mov rax, 60
    pop rdi
    syscall
    add rsp,8
    mov rax, 5
    push rax

    push qword ptr [rsp+8]

    pop rax
    pop rbx
    add rax, rbx
    push rax

    push qword ptr [rsp+0]

    mov rax, 60
    pop rdi
    syscall
    mov rax, 60
    mov rdi, 0
    syscall
