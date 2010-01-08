/* world_adaptor.h
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


#ifndef hajo_world_adaptor_h
#define hajo_world_adaptor_h

/* world_adapter.h
 *
 * adpater between angband code and simugraph engine
 * Hj. Malthaner, Jan 2001
 */

// I need angband's bool
#include "../h-type.h"

/* 
 * Highlit location - use read only! 
 */
extern int high_x;
extern int high_y;

int init_adaptor();
int close_adaptor();

int refresh_display();

/**
 * remember targetted location
 * @author J. Frieling
 */
bool iso_target_hook(char *fmt);

/**
 * Highlite (mark) location x,y
 * @author Hj. Malthaner
 */
void highlite_spot(int x, int y);

/**
 * Set a grid type (takes argument modulo 3)
 * @author Hj. Malthaner
 */
void set_grid(int no);


/**
 * Show which grid type ?
 * @author Hj. Malthaner
 */
int get_grid();

                  
/**
 * Turn shadows on/off (0=off, 1=on)
 * @author Hj. Malthaner
 */
//void set_shadow(int yesno);



/**
 * Ermittelt x-Offset gescrollter Karte
 * @author Hj. Malthaner
 */
int get_x_off();


/**
 * Ermittelt y-Offset gescrollter Karte
 * @author Hj. Malthaner
 */
int get_y_off();

                                     
/**
 * Determines i-offset of the watch point
 * @author Hj. Malthaner
 */
int get_i_off();


/**
 * Determines j-offset of the watch point
 * @author Hj. Malthaner
 */
int get_j_off();


#endif

