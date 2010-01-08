/* hackdef.h
 *
 * Copyright (c) 2001 Hansjörg Malthaner
 * hansjoerg.malthaner@gmx.de
 *
 * This file is part of the Simugraph<->Angband adaption code.
 *
 *
 * This file may be copied and modified freely so long as the above credits,
 * this paragraph, and the below disclaimer of warranty are retained; no
 * financial profit is derived from said modification or copying; and all
 * licensing rights to any modifications are granted to the original author,
 * Hansjörg Malthaner.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* hackdef.c
 * definion of hacklevel for iso view
 * 
 * Hj. Maltahner, Feb. 2001
 */
                          

/*
 * this enables somthing like a bigscreen mode but keeps a
 * resonable clean interfacing between iso-view and angband
 * this requires changes to map_info() to support clipping.
 * 
 * Hj. Maltahner, Feb. 2001
 */ 
#define USE_SMALL_ISO_HACK
