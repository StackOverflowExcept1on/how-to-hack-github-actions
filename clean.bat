@echo off

setlocal ENABLEDELAYEDEXPANSION

for /f "tokens=*" %%i in (.gitignore) do (
  set filename=%%i
  set filename=!filename:/=\!
  if exist !filename!\* (
    for /d %%d in (!filename!) do (
      del /s /f /q %%d\*.*
      for /f %%f in ('dir /ad /b %%d\') do rd /s /q %%d\%%f
      rd %%d
    )
  ) else if exist !filename! (
    del /f /q !filename!
  )
)

cd projects\csharp\patcher
call clean.bat
