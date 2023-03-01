SYS_OPEN EQU 5
SYS_WRITE EQU 4
SYS_READ EQU 3
STDIN    EQU 0
STDOUT    EQU 1
SYS_EXIT  EQU 1
S_IRWXU  EQU 0x700
O_RDONLY EQU 0x000
O_WRONLY EQU 0x001
extern printf
global main

section .bss
    inputFile resb 100;in.txt
    outputFile resb 100;out.txt
    encryptionNumber resb 100; 12345
    bufferIn         resb 100
    char_output      resb  1
section .data
     msg     db  'in', 10,  0
     msg_with_string db 'the string is: %s', 10, 0
     msg_with_char db 'the char is: %c',10, 0

     msg_with_number db 'the length of the number is: %d', 10, 0
    ; len1 EQU $ - msg1
    debugg   dd      0;;;;;;;;;;;;;;;;;; -D flag
    encrypt  dd      0;;;;;;;;;;;;;;;;;;;  +e flag can be 1
    input    dd      0 ;;;;;;;;;;;;;;; -i flag by default is STDIN 
    input_bool dd    0 
    output   dd      1;;;;;;;;;;;;;;; -o flag by default is STOUT
    output_bool dd   0
    i        dd      1;                     starting index to read arguments
    argc     dd      0;                     argc
    length_of_encryption    dd  0;           +e 12345 => length_of_encryption = 5
    input_file_index        dd  0
    encryptionNumberIndex   dd  0
    numberOfCharsRead       dd  0
    

section .text
main:
    push ebp
    mov ebp, esp
    mov eax,[ebp + 8] 
    mov dword[argc], eax;     setting argc
    mov eax, dword[ebp + 12];    eax is the pointer to argv
initiate_flags:
    mov ebx, dword[i]
    cmp ebx, dword[argc];  if(i == argc)=> endLoop
    je end_initiate_flags

    mov ecx, dword[eax + ebx*4]; ecx = argv[i]

    ;1;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; flags checking   ;;;;;;;;;;;;;;;;;;;;;;;;;
    
    cmp word[ecx], "+e"         ;if(argv[i][0] == "+" and argv[i][1] == "e")
    jne not_plus_e
    mov dword[encrypt], 1

    mov edx, ecx;           edx->argv[i]
    add edx, 2;             edx->argv[i]+2
    mov dword[encryptionNumber], edx;encryptionNumber -> [1,2,3,4,5]
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;compute length
    pushad;                 save register states
    push edx
    call compute_length_of_encryption
    add esp, 4

    popad;                  restore register states
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;end compute length
;2;
not_plus_e:
    cmp word[ecx], "-i"         ;if(argv[i][0] == "-" and argv[i][1] == "i")
    jne not_minus_i
    mov dword[input_bool], 1
    mov edx, ecx
    add edx, 2
    mov dword[inputFile], edx;presists hellow.txt
    pushad
    ;input flag is STDID
    push dword[inputFile]
    push msg_with_string;read call
    call printf
    add esp, 8
    popad 
;3;
not_minus_i:
    cmp word[ecx], "-o"         ;if(argv[i][0] == "-" and argv[i][1] == "o")
    jne not_minus_o

    mov dword[output_bool], 1
    mov edx, ecx
    add edx, 2
    mov dword[outputFile], edx;presists out.txt
    pushad
    ;input flag is STDID
    push dword[outputFile]
    push msg_with_string;read call
    call printf
    add esp, 8
    popad 
not_minus_o:
    inc dword[i]
    jmp initiate_flags


end_initiate_flags:
;users input
    ;if input flag == 0
    cmp dword[input_bool], 0
    je input_flag_is_off

    ;open input file which we got from -iinput.txt
    ;push eax;persist the pointer of argv
    pushad
    push S_IRWXU;file permissions
    push O_RDONLY;read only
    push dword[inputFile];input file name
    push SYS_OPEN
    call system_call
    add esp, 16
    mov dword[input], eax
    popad
    ;pop eax;restore argv pointer

input_flag_is_off:
    cmp dword[output_bool], 0
    je output_flag_is_off

    ;open output file which we got from -ooutput.txt
    ;push eax;persist the pointer of argv
    pushad
    push S_IRWXU;file permissions
    push O_WRONLY;write only
    push dword[outputFile];output file name
    push SYS_OPEN
    call system_call
    add esp, 16
    mov dword[output], eax
    popad
    ;pop eax;restore argv pointer
;;;;;;;;;;;;;;;read into buffer;;;;;;;;;;
output_flag_is_off:

read_from_user_input:   
    pushad
    ;input flag is STDID
    push 100;edx = number of bytes to read
    push bufferIn;ecx = where to store the data
    push dword[input];STDIN
    push SYS_READ;read call
    call system_call
    add esp, 16
    mov dword[numberOfCharsRead], eax

    pushad
    push eax
    push msg_with_number
    call printf
    add esp, 8
    popad
    

while_not_EOF:
    ;TODO read the buffer bytes until facing -1 == EOF
    cmp dword[numberOfCharsRead], 0
    je EOF

    mov edx, dword[input_file_index];edx = fileIndex
    xor ecx, ecx
    mov cl, byte[bufferIn + edx];cl = fileIn[index]

   

    cmp cl, 10
    je new_line
    
    ;do counter until \n
    cmp dword[encrypt], 0
    je not_encrypted
    mov edx, dword[encryptionNumber]
    add edx, dword[encryptionNumberIndex]
    xor eax, eax
    mov al, byte[edx];al = encryptionNumber[encryptionNumberIndex]
    sub al, 48; al-'0'
    add cl, al; cl(currentChar) += (encryptionNumber[encryptionNumberIndex]-'0')
    inc dword[encryptionNumberIndex];encryptionNumberIndex++

    mov edx, dword[encryptionNumberIndex]
    ;if(encryptionNumberSize == encryptionNumberIndex)
    cmp dword[length_of_encryption], edx
    jne reset_encryptionNumberIndex_not_needed
    mov dword[encryptionNumberIndex], 0;reset index
    ;if lower case
    jmp reset_encryptionNumberIndex_not_needed
    not_encrypted:

    cmp cl, 97;cl<'a'
    jb not_lower
    cmp cl, 122;cl>'z'
    ja not_lower
    sub cl, 32;to lower
    
    jmp reset_encryptionNumberIndex_not_needed

new_line:
    mov byte[char_output], 10
    pushad
    push 1;1 char
    push dword char_output
    push dword[output]
    push SYS_WRITE;SYS_WRITE
    call system_call
    add esp, 16
    popad

    
    mov dword[encryptionNumberIndex], 0
    cmp dword[input_bool], 0
    je not_input
    inc dword[input_file_index]
    dec dword[numberOfCharsRead]
    jmp while_not_EOF

    not_input:
    mov dword[input_file_index], 0
    jmp read_from_user_input

not_lower:
reset_encryptionNumberIndex_not_needed:

;;;;;;;;;;;;print cl


    mov byte[char_output], cl
    pushad
    push 1;1 char
    push dword char_output
    push dword[output]
    push SYS_WRITE;SYS_WRITE
    call system_call
    add esp, 16
    popad

    dec dword[numberOfCharsRead]
    inc dword[input_file_index]
    jmp while_not_EOF

    
EOF:
    xor eax, eax
    leave
    ret


compute_length_of_encryption:
    push ebp
    mov ebp, esp
    mov eax, dword[ebp + 8]; eax-> argv[i]+2 example eax->[1, 2, 3, 4, 5]
    mov ebx, 0;             ebx = counter = 0
compute_length_of_encryption_loop:
    xor ecx, ecx
    mov cl, byte[eax+ebx]; cl = eax[i]
    cmp cl, 0
    je end_of_compute_length_of_encryption_loop; if(eax[i] == 0)break;
    inc ebx;              counter++
    jmp compute_length_of_encryption_loop

end_of_compute_length_of_encryption_loop:
    mov dword[length_of_encryption], ebx; length_of_encryption = counter = ebx
    leave
    ret

      
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
