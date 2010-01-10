ADD_LIBRARY (lua STATIC 
  lapi.c lcode.c ldebug.c ldo.c lfunc.c lgc.c
  llex.c lmem.c lobject.c lparser.c lstate.c lstring.c
  ltable.c ltests.c ltm.c lundump.c lvm.c lzio.c
  lauxlib.c lbaselib.c ldblib.c liolib.c lstrlib.c
  tolua_lb.c tolua_rg.c tolua_tt.c tolua_tm.c tolua_gp.c
  tolua_eh.c tolua_bd.c)

ADD_EXECUTABLE(tolua tolua.c tolualua.c lua)

TARGET_LINK_LIBRARIES(tolua lua)
