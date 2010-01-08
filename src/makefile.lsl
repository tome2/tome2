# File: Makefile.lsl

# Purpose: Makefile for Linux + SVGA library

SRCS = \
  z-util.c z-virt.c z-form.c z-rand.c z-term.c z-sock.c \
  variable.c tables.c util.c cave.c cmovie.c \
  object1.c object2.c traps.c monster1.c monster2.c monster3.c \
  xtra1.c xtra2.c spells1.c spells2.c help.c \
  melee1.c melee2.c save.c files.c notes.c \
  cmd1.c cmd2.c cmd3.c cmd4.c cmd5.c cmd6.c cmd7.c  \
  status.c randart.c gods.c modules.c \
  store.c birth.c loadsave.c ghost.c \
  wizard1.c wizard2.c wild.c powers.c \
  generate.c gen_maze.c gen_evol.c dungeon.c init1.c init2.c \
  bldg.c levels.c squeltch.c plots.c \
  main-ami.c main.c

OBJS = \
  z-util.o z-virt.o z-form.o z-rand.o z-term.o z-sock.o \
  variable.o tables.o util.o cave.o cmovie.o \
  object1.o object2.o traps.o monster1.o monster2.o monster3.o \
  xtra1.o xtra2.o spells1.o spells2.o help.o \
  melee1.o melee2.o files.o notes.o \
  cmd1.o cmd2.o cmd3.o cmd4.o cmd5.o cmd6.o cmd7.o \
  status.o randart.o gods.o modules.o \
  store.o birth.o loadsave.o ghost.c \
  wizard1.o wizard2.o wild.o powers.o \
  generate.o gen_maze.o gen_evol.o dungeon.o init1.o init2.o \
  bldg.o levels.o squeltch.o plots.o \
  main-lsl.o main.o

CC = gcc

CFLAGS = -Wall -O6 -D"USE_LSL"
LIBS = -lvgagl -lvga

# Build the program

angsvga: $(SRCS) $(OBJS)
	$(CC) $(CFLAGS)  -o angband $(OBJS) $(LDFLAGS) $(LIBS)

