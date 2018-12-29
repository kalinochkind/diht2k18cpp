call @func_readint
mov R251 R0
call @func_readint
mov R252 R0
call @func_readint
mov R253 R0
set R250 100

and R251 R251
jiz @solve_linear

mov R249 R252
smul R249 R252
set R248 4
smul R248 R251
smul R248 R253
sub R249 R248
jis @print_none
jiz @zero_d
mov R0 R249
smul R0 R250
smul R0 R250
call @func_sqrt
mov R249 R0
neg R252
smul R252 R250
add R251 R251
mov R248 R252
sub R248 R249
sdiv R248 R251
mov R0 R248
call @func_printreal
set R1 32
out R1
mov R248 R252
add R248 R249
sdiv R248 R251
mov R0 R248
call @func_printreal
jmp @end


@zero_d
add R251 R251
neg R252
smul R252 R250
sdiv R252 R251
mov R0 R252
call @func_printreal
jmp @end


@solve_linear
and R252 R252
jiz @solve_const
smul R253 R250
neg R253
sdiv R253 R252
mov R0 R253
call @func_printreal
jmp @end



@solve_const
and R253 R253
jiz @print_inf
jmp @print_none


@func_sqrt
set R1 1
set R2 0
@startloop_sqrt
mov R3 R2
smul R3 R3
sub R3 R0
jus @ret_sqrt
add R2 R1
jmp @startloop_sqrt
@ret_sqrt
mov R0 R2
ret



@func_printreal
and R0 R0
jus @skipsign_printreal
set R1 45
out R1
neg R0
@skipsign_printreal
mov R9 R0
sdiv R0 R250
call @func_printint
set R0 46
out R0
smod R9 R250
mov R0 R9
set R8 10
sdiv R0 R8
call @func_printint
mov R0 R9
smod R0 R8
call @func_printint
ret



@print_inf
set R0 105
out R0
set R0 110
out R0
set R0 102
out R0
jmp @end

@print_none
set R0 110
out R0
set R0 111
out R0
set R0 110
out R0
set R0 101
out R0
jmp @end


@func_readint  ; reads an integer and stores it in R0
xor R0 R0
set R5 1
set R3 45
set R4 3
set R2 10
@startloop_readint
in R1
sub R1 R3
juz @skipsign_readint
set R5 -1
jmp @startloop_readint
@skipsign_readint
sub R1 R4
jis @ret_readint
smul R0 R2
add R0 R1
jmp @startloop_readint
@ret_readint
smul R0 R5
ret

@func_printint  ; prints an integer in R0
set R2 10
set R3 48
xor R1 R1
push R1
and R0 R0
jus @startloop_printint
set R4 45
out R4
neg R0
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
