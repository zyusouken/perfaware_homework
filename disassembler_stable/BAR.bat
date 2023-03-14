:::::::::::::::::::
:: BUILD AND RUN ::
:::::::::::::::::::
@echo off
cls

::Set hwName manually for each homework.
::This should be the name of the .c file
::for the disassembler code, w/o the .c
set hwName=disass
::inASM is the .asm passed to this .bat as an argument
set inASM=%1

::Assemble
nasm.exe %inASM%
::New var for bin filename (.asm truncated)
set inBIN=%inASM:.asm=%

::Compile
gcc %hwName%.c -o %hwName%.exe
::Run (Params are input bin, output asm)
%hwName%.exe %inBIN% disassembled_output.asm

echo.
echo.
echo.
echo [Re-assembling for binary comparison...]
::RE-assemble
nasm.exe disassembled_output.asm
::Compare
FC /B %inBIN% disassembled_output

::Clean up output
del %hwName%.exe
del disassembled_output.asm
del disassembled_output
del %inBIN%