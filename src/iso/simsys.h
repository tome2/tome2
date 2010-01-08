/* simsys.h
 *
 * Copyright (c) 2001 Hansjörg Malthaner
 * hansjoerg.malthaner@gmx.de
 *
 * This file is part of the Simugraph graphics engine.
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

#ifndef simsys_h
#define simsys_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif


/* Variablen zur Messageverarbeitung */

/* Klassen */

#define SIM_NOEVENT         0
#define SIM_MOUSE_BUTTONS   1
#define SIM_KEYBOARD        2
#define SIM_MOUSE_MOVE      3
#define SIM_IGNORE_EVENT    255

/* Aktionen */ /* added RIGHTUP and MIDUP */
#define SIM_MOUSE_LEFTUP            1
#define SIM_MOUSE_RIGHTUP           2
#define SIM_MOUSE_MIDUP             3
#define SIM_MOUSE_LEFTBUTTON        4
#define SIM_MOUSE_RIGHTBUTTON       5
#define SIM_MOUSE_MIDBUTTON         6
#define SIM_MOUSE_MOVED             7


/**
 * inits operating system stuff
 * @author Hj. Malthaner
 */
int dr_os_init(int n, int *parameter);


/**
 * opens graphics device/context/window of size w*h
 * @param w width
 * @param h height
 * @author Hj. Malthaner
 */
int dr_os_open(int w, int h);


/**
 * closes operating system stuff
 * @author Hj. Malthaner
 */
int dr_os_close();


/**
 * retrieve display width
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
int dr_get_width();


/**
 * retrieve display height
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
int dr_get_height();


/**
 * creates a (maybe virtual) array of graphics data
 * @author Hj. Malthaner
 */
unsigned short * dr_textur_init();


/**
 * displays the array of graphics data
 * @author Hj. Malthaner
 */
void dr_textur(int xp, int yp, int w, int h);


/**
 * use this method to flush graphics pipeline (undrawn stuff) onscreen.
 * @author Hj. Malthaner
 */
void dr_flush();


/**
 * set colormap entries
 * @author Hj. Malthaner
 */ 
void dr_setRGB8multi(int first, int count, unsigned char * data);


/**
 * display/hide mouse pointer
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
void show_pointer(int yesno);


/**
 * move mouse pointer
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
void move_pointer(int x, int y);


/**
 * update softpointer position
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
void ex_ord_update_mx_my();


/**
 * get events from the system
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
void GetEvents();


/**
 * get events from the system without waiting
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
void GetEventsNoWait();


/**
 * @returns time since progrma start in milliseconds
 * @author Hj. Malthaner
 */
long long dr_time(void);


/**
 * sleeps some microseconds
 * @author Hj. Malthaner
 */
void dr_sleep(unsigned long usec);


/**
 * loads a sample
 * @return a handle for that sample or -1 on failure
 * @author Hj. Malthaner
 */
int dr_load_sample(const char *filename);


/**
 * plays a sample
 * @param key the key for the sample to be played
 * @author Hj. Malthaner
 */
void dr_play_sample(int key, int volume);

#ifdef __cplusplus
}
#endif

#endif

