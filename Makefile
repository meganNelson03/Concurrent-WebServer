# An admittedly primitive Makefile
# To compile, type "make" or make "all"
# To remove files, type "make clean"

CC = gcc
CFLAGS = -Wall
OBJS = wserver.o wclient.o request.o io_helper.o 
SUBMITDIR = webserver-submit

.SUFFIXES: .c .o 

all: wserver wclient spin.cgi

wserver: wserver.o request.o io_helper.o
	$(CC) $(CFLAGS) -o wserver wserver.o request.o io_helper.o 

wclient: wclient.o io_helper.o
	$(CC) $(CFLAGS) -o wclient wclient.o io_helper.o

spin.cgi: spin.c
	$(CC) $(CFLAGS) -o spin.cgi spin.c

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm -rf $(OBJS) wserver wclient spin.cgi webserver-submit

submit:
	-@rm -rf $(SUBMITDIR)
	-@rm -rf .tmp 
	@mkdir .tmp
	@cp * .tmp
	@mv .tmp $(SUBMITDIR)
	submit334 $(SUBMITDIR) $(SUBMITDIR)
