/*
** $Id: lgc.h,v 1.3 2001/11/26 23:00:24 darkgod Exp $
** Garbage Collector
** See Copyright Notice in lua.h
*/

#ifndef lgc_h
#define lgc_h


#include "lobject.h"


void luaC_collect (lua_State *L, int all);
void luaC_checkGC (lua_State *L);


#endif
