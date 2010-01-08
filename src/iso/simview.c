/* simview.c
 *
 * Copyright (c) 2001,2002 Hansjörg Malthaner
 * hansjoerg.malthaner@gmx.de
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


#include "simview.h"
#include "world_view.h"
#include "world_adaptor.h"
#include "simgraph.h"


/**
 * Draws the full iso-view display
 * @author Hj. Malthaner
 */
void display()
{
    const int IMG_SIZE = display_get_tile_size();

    const int const_x_off = display_get_width()/2 + get_x_off();
    const int dpy_width = display_get_width()/IMG_SIZE + 2;
    const int dpy_height = (display_get_height()*4)/IMG_SIZE;


    const int i_off = get_i_off();
    const int j_off = get_j_off();

    int x,y;
    
//    puts("displaying");


    // Hajo: draw grounds first

    for(y=-5; y<dpy_height+10; y++) {
	const int ypos = y*IMG_SIZE/4+16 + get_y_off();
        
	for(x=-dpy_width + (y & 1); x<=dpy_width+2; x+=2) {

	    const int i = ((y+x) >> 1) + i_off;
	    const int j = ((y-x) >> 1) + j_off;
	    const int xpos = x*IMG_SIZE/2 + const_x_off;

	    display_boden(i, j, xpos, ypos);
	}
    }

    // Hajo: then draw the objects

    for(y=-5; y<dpy_height+10; y++) {
	const int ypos = y*IMG_SIZE/4+16 + get_y_off();
        
	for(x=-dpy_width + (y & 1); x<=dpy_width+2; x+=2) {

	    const int i = ((y+x) >> 1) + i_off;
	    const int j = ((y-x) >> 1) + j_off;
	    const int xpos = x*IMG_SIZE/2 + const_x_off;


	    display_dinge(i, j, xpos, ypos);
	}
    }
}

