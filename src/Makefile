CFLAGS=-Wall -g -O6 -foptimize-register-move -fexpensive-optimizations -march=pentiumpro 
OBJS=fwindow.o fractal.o fractalr.o mandel.o gifsave.o
.SUFFIXES: .c .o .rc .asm

fractal.exe:$(OBJS) 
	gcc $(OBJS) -lcomdlg32 -lgdi32 -o fractal.exe -mwindows 

# 	gcc $(OBJS) -lcomdlg32 -lgdi32 -o fractal.exe -mwindows -mno-cygwin

install:
	strip fractal.exe 

clean:
	rm  fractal.exe $(OBJS)

con:$(OBJS) 
	gcc -g $(OBJS) -lcomdlg32 -lgdi32 -o fractal.exe
.rc.o:
	windres.exe -o $@ $<
.cc.o:
	gcc $(CFLAGS) -c -g  $<
	
.c.o:
	gcc $(CFLAGS) -c -g  $<

.asm.o:
	nasm -f win32 -o $@ $<
