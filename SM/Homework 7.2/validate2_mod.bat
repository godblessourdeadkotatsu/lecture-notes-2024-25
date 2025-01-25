@echo off
:: Prompt for the number of iterations
del log2mod.txt
set /p n=Enter the number of times to execute the simulation: 
:: Set the program to be called
set program=es2_minimal_mod.exe

:: Loop to call the program n times
for /L %%i in (1,1,%n%) do (
    echo Calling %%i: %program%
    %program% %%i >> log2mod.txt
)

set program=count_occurrences.exe
echo Running %program% to count occurrences...
%program% log2mod.txt