# File: Makefile.gdb
# By DarkGod, to create a tome.bin to be used with gdb

# Purpose: Makefile support for "main-dos.c"

#
# Note: Rename to "Makefile" before using
#
# Allegro support by Robert Ruehlmann (rr9@angband.org)
#

# Compiling with MOD-file support:
# - Get the JG-MOD library from http://www.jgmod.home.ml.org and install it.
# - Insert -ljgmod in front of -lalleg to the Libraries section.
# - Add -DUSE_MOD_FILES to the compiler flags.
# - Copy your MOD-files into the "lib/xtra/music" folder.

# Enable lua scripting supoprt
LUA = TRUE

#
# Basic definitions
#

# Objects
OBJS = \
  main.o main-dos.o main-ibm.o \
  generate.o gen_maze.o gen_evol.o dungeon.o init1.o init2.o plots.o help.o \
  store.o birth.o wizard1.o wizard2.o bldg.o cmovie.o \
  cmd1.o cmd2.o cmd3.o cmd4.o cmd5.o cmd6.o cmd7.o \
  loadsave.o files.o levels.o notes.o squeltch.o \
  status.o randart.o gods.o skills.o modules.o \
  xtra1.o xtra2.o spells1.o spells2.o melee1.o melee2.o \
  object1.o object2.o traps.o monster1.o monster2.o monster3.o \
  variable.o tables.o util.o cave.o ghost.o wild.o powers.o \
  z-term.o z-rand.o z-form.o z-virt.o z-util.o z-sock.o

LUAOBJS = \
  script.o lua_bind.o \
  w_util.o w_player.o w_z_pack.o w_obj.o w_mnster.o w_spells.o w_quest.o w_play_c.o w_dun.o

TOLUAOBJS = \
  lua/lapi.o lua/lcode.o lua/ldebug.o lua/ldo.o lua/lfunc.o lua/lgc.o \
  lua/llex.o lua/lmem.o lua/lobject.o lua/lparser.o lua/lstate.o lua/lstring.o \
  lua/ltable.o lua/ltests.o lua/ltm.o lua/lundump.o lua/lvm.o lua/lzio.o \
  lua/lauxlib.o lua/lbaselib.o lua/ldblib.o lua/liolib.o lua/lstrlib.o \
  lua/tolua_lb.o lua/tolua_rg.o lua/tolua_tt.o lua/tolua_tm.o lua/tolua_gp.o \
  lua/tolua_eh.o lua/tolua_bd.o

ifdef LUA
OBJS += $(LUAOBJS)
OBJS += $(TOLUAOBJS)
endif

# Compiler
CC = gcc

ifdef LUA
LUAFLAGS = -DUSE_LUA -DLUA_NUM_TYPE='long long' -I. -I./lua
endif

# Compiler flags
CFLAGS = -Wall -g -DUSE_DOS -DUSE_IBM -DUSE_BACKGROUND \
-DUSE_TRANSPARENCY $(LUAFLAGS)

# Libraries
LIBS = -lpc -lalleg $(LUALIBS)


#
# Targets
#

TOLUA = tolua.exe

default: ../tome.exe $(TOLUA)

release: ../tome.exe
	upx -9 ../tome.exe
#         copy tome.exe ..
#         del tome.exe

install: ../tome.exe
#        copy tome.exe ..

all: ../tome.exe
#        @echo All done.  Use 'make install' to install.

$(TOLUA): $(TOLUAOBJS) lua/tolua.c lua/tolualua.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TOLUAOBJS) lua/tolua.c lua/tolualua.c $(LIBS)


#
# Link executables
#

../tome.exe: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)


#
# Compile source files
#

.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $*.c


#
# Clean up
#

clean:
	del *.o

cleanall: clean
	del *.exe

plots.o: q_rand.c q_main.c q_one.c \
	q_thief.c q_hobbit.c q_nazgul.c q_troll.c q_wight.c q_shroom.c \
	q_spider.c q_poison.c \
	q_eol.c q_nirna.c q_invas.c \
	q_betwen.c \
	q_narsil.c q_wolves.c q_dragons.c q_haunted.c q_evil.c

LUA_RECOMP = true
ifdef LUA_RECOMP
w_mnster.c: monster.pkg $(TOLUA)
	$(TOLUA) -n monster -o w_mnster.c monster.pkg

w_player.c: player.pkg $(TOLUA)
	$(TOLUA) -n player -o w_player.c player.pkg

w_play_c.c: player_c.pkg $(TOLUA)
	$(TOLUA) -n player_c -o w_play_c.c player_c.pkg

w_z_pack.c: z_pack.pkg $(TOLUA)
	$(TOLUA) -n z_pack -o w_z_pack.c z_pack.pkg

w_obj.c: object.pkg $(TOLUA)
	$(TOLUA) -n object -o w_obj.c object.pkg

w_util.c: util.pkg $(TOLUA)
	$(TOLUA) -n util -o w_util.c util.pkg

w_spells.c: spells.pkg $(TOLUA)
	$(TOLUA) -n spells -o w_spells.c spells.pkg

w_quest.c: quest.pkg $(TOLUA)
	$(TOLUA) -n quest -o w_quest.c quest.pkg

w_dun.c: dungeon.pkg $(TOLUA)
	$(TOLUA) -n dungeon -o w_dun.c dungeon.pkg

endif
