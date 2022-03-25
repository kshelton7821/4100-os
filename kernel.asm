global k_print
global dispatch
global lidtr
global go
extern Running
extern dequeue
extern enqueue
extern queue


k_print:
push ebp
mov ebp, esp
pushf
push eax
push ebx
push ecx
push esi
push edi
;Complete Push of used Registers

;Start to calculate offset
mov eax, 0xB8000
mov ebx, [ebp + 16] ;load row into b
imul ebx, 80 ;mult row by 80
add ebx, [ebp + 20] ;add column to result
imul ebx, 2 ;mult result by 2
add eax, ebx ; add base to result and store in a
mov edi, eax ;move offset from aex to edi
mov ecx, [ebp + 12] ;load length of string into ecx
mov esi, [ebp + 8] ;load address of string into esi

_loop:
cmp edi, 0xB8F9F ; cmp edi to 0xB8F9E
jl _good ; if less than than 0xB8F9E jmp to _good
mov edi, 0xB8000 ; if greater than 0xB8F9E load edi with 0xB8000
_good:
cmp ecx, 0 ; cmp length to 0
je _printdone ; if done jmp to done sequence
movsb ; move string byte
mov BYTE [edi], 31 ; move color code to memory specified by edi
inc edi ; increment edi to next character
dec ecx
jmp _loop
_printdone:
;Restore registers to default state
pop edi
pop esi
pop ecx
pop ebx
pop eax
popf
pop ebp
ret


lidtr:
push ebp ;store bptr
mov ebp, esp ;store stptr
pushf
push eax

mov eax, [ebp+8] ;move itdr to aex
lidt [eax]

;restore registers
pop eax
popf
pop ebp
ret

dispatch:
pushad
push ds
push es
push fs
push gs

mov eax, [Running]
mov [eax], esp

push eax
push queue
call enqueue
push queue
call dequeue
mov esp, [eax]

pop gs
pop fs
pop es
pop ds
popad
iret

go:
;dequeue pcb from queue and make it the running process
push queue
call dequeue
;set esp register to esp value in the current processes pcb
mov esp, [eax]
;sub esp, 4
;restore (pop) gs fs es ds
;restore all the general purpose registers (popad)
pop gs
pop fs
pop es
pop ds
popad

;iret (return from interrupt. Why does this work?)
iret