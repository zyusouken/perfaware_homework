:::::::::::::::::::
:: BUILD AND RUN ::
:::::::::::::::::::
@echo off
cls

::Set hwName manually for each homework.
set hwName=hw03
::DELETE old output (So it won't run if it doesn't build.)
::del %hwName%.exe

::inASM is the .asm passed to this .bat
set inASM=%1
::Assemble
nasm %inASM%
::Truncate .asm extension
set inBIN=%inASM:.asm=%

::Compile
gcc %hwName%.c -o %hwName%.exe

::Test run
::1st param is input bin (passed to this .bat)
::2nd param is output .asm filename
%hwName%.exe %inBIN% %hwName%_output.asm

::Reassemble and compare with examble bin
echo.
echo.
echo.
echo [Re-assembling and comparing with source file...]
nasm %hwName%_output.asm
FC /B %inBIN% %hwName%_output

::Clean up output
del %hwName%.exe
del %hwName%_output.asm
del %hwName%_output
del %inBIN%