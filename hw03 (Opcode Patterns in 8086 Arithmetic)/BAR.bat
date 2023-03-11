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

::Assemble
nasm %inASM%
::New var for bin filename (.asm truncated)
set inBIN=%inASM:.asm=%

::Compile
gcc %hwName%.c -o %hwName%.exe
::Run (Params are input bin, output asm)
%hwName%.exe %inBIN% %hwName%_output.asm

echo.
echo.
echo.
echo [Re-assembling and comparing with original bin...]
::RE-assemble
nasm %hwName%_output.asm
::Compare
FC /B %inBIN% %hwName%_output

::Clean up output
del %hwName%.exe
del %hwName%_output.asm
del %hwName%_output
del %inBIN%