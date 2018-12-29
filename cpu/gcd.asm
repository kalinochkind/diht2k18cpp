call @func_readint
push R0
call @func_readint
pop R1
@startloop
and R1 R1
jiz @endloop
umod R0 R1
mov R2 R0
mov R0 R1
mov R1 R2
jmp @startloop
@endloop
call @func_printint
jmp @end

@func_readint
xor R0 R0
set R3 48
set R4 10
@startloop_readint
in R1
sub R1 R3
jio @ret_readint
umul R0 R4
add R0 R1
jmp @startloop_readint
@ret_readint
ret

@func_printint
set R2 10
set R3 48
xor R4 R4
set R5 1
@startloop_printint
mov R1 R0
umod R1 R2
add R1 R3
store8 R4 R1
add R4 R5
udiv R0 R2
and R0 R0
juz @startloop_printint
@ret_printint
sub R4 R5
load8 R6 R4
out R6
juz @ret_printint
ret

@end
