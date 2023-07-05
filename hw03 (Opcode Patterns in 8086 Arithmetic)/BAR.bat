:::::::::::::::::::
:: BUILD AND RUN ::
:::::::::::::::::::
@echo off
cls

::Set hwName manually for each homework.
::This should be the name of the .c file
::for the disassembler code, w/o the .c
set hwName=hw03
::inASM is the .asm passed to this .bat as an argument
set inASM=%1
set inBIN=assembled_input

::Assemble
nasm.exe %inASM% -o assembled_input

::Compile
gcc %hwName%.c -o %hwName%.exe
::Run (Params are input bin, output asm)
%hwName%.exe %inBIN% disassembled_output.asm

echo.
echo [Re-assembling for binary comparison...]
echo.
::RE-assemble
nasm.exe disassembled_output.asm
::Compare
FC /B %inBIN% disassembled_output

::Clean up output
del %hwName%.exe
del disassembled_output.asm
del disassembled_output
del %inBIN%

:END