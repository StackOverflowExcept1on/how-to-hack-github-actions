@echo off

set PROJECT_NAME=patcher
set DIRECTORIES=dist .vs .idea %PROJECT_NAME%\bin %PROJECT_NAME%\obj

for %%d in (%DIRECTORIES%) do (
  if exist %%d (
    del /s /f /q %%d\*.*
    for /f %%f in ('dir /ad /b %%d\') do rd /s /q %%d\%%f
    rd %%d
  )
)
