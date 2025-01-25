@echo off
:: Prompt for the number of iterations
del log2.txt
set /p n=Enter the number of times to execute the simulation: 
:: Set the program to be called
set program=es2_minimal.exe

:: Loop to call the program n times
for /L %%i in (1,1,%n%) do (
    echo Calling %%i: %program%
    %program% %%i >> log2.txt
)

set program=count_occurrences.exe
echo Running %program% to count occurrences...
%program% log2.txt