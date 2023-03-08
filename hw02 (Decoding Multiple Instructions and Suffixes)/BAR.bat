:::::::::::::::::::
:: BUILD AND RUN ::
:::::::::::::::::::
@echo off
cls

::Set hwName manually for each homework.
set hwName=hw02
::Wipe old output (So it won't run if it doesn't build.)
del %hwName%.exe
del %hwName%_output.asm
del %hwName%_output ::binary

::arg1 is the bin passed to this .bat
set arg1=%1
gcc %hwName%.c -o %hwName%.exe

::Test run
::1st param is input bin (passed to this .bat)
::2nd param is output .asm filename
%hwName%.exe %arg1% %hwName%_output.asm
echo.
echo.
echo.
nasm %hwName%_output.asm
FC /B %arg1% %hwName%_output