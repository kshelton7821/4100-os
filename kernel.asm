global k_print
global dispatch
global lidtr
global go
global outportb
global init_timer_dev
extern Running
extern dequeue
extern enqueue
extern queue


;Start of k_print(char *string, int string_length, int row, int column)
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


;Start of lidtr(idtr_t *help)
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


;Start of dispatch()
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

push eax
mov al, 0x20
out 0x20, al
pop eax
iret


;Start of go()
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


;Start of outportb(uint16_t port, uint8_t value)
outportb:
;Save Registers
push ebp
mov ebp, esp
pushf
pushad

;Move 2 byte and 1 byte values then call out dx, al
mov edx, [ebp+8]
mov eax, [ebp+12]

out dx, al

;Restore Registers
popad
popf
pop ebp
ret


;Start of init_timer_dev(uint8_t value)
init_timer_dev:
;1) Do the normal preamble for assembly functions (set up ebp and save any registers
; that will be used). The first arg is time in ms
;2) move the ms argument value into a register (say, edx)
;3) Multiply dx (only the bottom 16 bits will be used) by 1193. 
; Why? because the timer cycles at 1193180 times a second, which is 1193 times a ms 
; note: The results must fit in 16 bits, so ms can't be more than 54.
; So, put your code for steps 1 - 3 HERE:
push ebp
mov ebp, esp
pushad
pushf

mov edx, [ebp+8]
imul dx, 1193


;The code for steps 4-6 is given to you for free below...
;4) Send the command word to the PIT (Programmable Interval Timer) that initializes Counter 0
; (there are three counters, but you will only use counter 0).
; The PIT will be initalized to mode 3, Read or Load LSB first then MSB, and
; Channel (counter) 0 with the following bits: 0b00110110 =
; Counter 0 |Read then load|Mode 3|Binary. So, the instructions will be:
mov al, 0b00110110 ; 0x43 is the Write control word
out 0x43, al
;5) Load the LSB first then the MSB.
; 0x40 = Load counter 0 with the following code: 
mov ax, dx
out 0x40, al ;LSB
xchg ah, al
out 0x40, al ;MSB
;6) clean up (pop ebp and other regs used) and return
popf
popad
pop ebp
ret