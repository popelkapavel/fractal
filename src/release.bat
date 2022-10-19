set VER=016
del fractal%VER%.zip
strip fractal.exe
7z a -tzip fractal%VER%.zip fractal.exe
7z a -tzip fractal%VER%src.zip *.h *.c *.asm fractal.ico fractalr.rc Makefile
copy fractal.exe c:\bin