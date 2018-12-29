; https://informatics.msk.ru/mod/statements/view.php?id=2550

call @func_readint
mov R253 R0
set R252 1
set R251 3
call @func_hanoi
jmp @end


@func_hanoi  ; R253 - level, R252 - from, R251 - to
and R253 R253
jiz @ret_hanoi
push R251
push R252
push R253
set R10 1
sub R253 R10
set R11 6
sub R11 R252
sub R11 R251
mov R251 R11
call @func_hanoi  ; hanoi(level - 1, from, 6 - from - to)
pop R253
mov R0 R253
call @func_printint ; print(level)
set R20 32
out R20 ; space
pop R252
mov R0 R252
call @func_printint ; print(from)
out R20 ; space
pop R251
mov R0 R251
call @func_printint ; print(to)
set R20 10
out R20  ; newline
set R10 1
sub R253 R10
set R11 6
sub R11 R252
sub R11 R251
mov R252 R11
jmp @func_hanoi  ; hanoi(level - 1, 6 - from - to, to), tail call
@ret_hanoi
ret

@func_readint  ; reads an integer and stores it in R0
xor R0 R0
set R3 48
set R2 10
@startloop_readint
in R1
sub R1 R3
jio @ret_readint
umul R0 R2
add R0 R1
jmp @startloop_readint
@ret_readint
ret

@func_printint  ; prints an integer in R0
set R2 10
set R3 48
xor R1 R1
push R1
@startloop_printint
mov R1 R0
umod R1 R2
add R1 R3
push R1
udiv R0 R2
and R0 R0
juz @startloop_printint
@outloop_printint
pop R1
and R1 R1
jiz @ret_printint
out R1
jmp @outloop_printint
@ret_printint
ret

@end
