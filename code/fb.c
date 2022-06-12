// Code by https://github.com/isometimes/rpi4-osdev
// Comments by Adrian, Natasha, (isometimes)

#include "io.h"
#include "mb.h"
#include "terminal.h"

unsigned int width, height, pitch, isrgb;
unsigned char *fb;

void fb_init()
{
    mbox[0] = 35*4; // Length of message in bytes (comment from isometimes)
    mbox[1] = MBOX_REQUEST;

    mbox[2] = MBOX_TAG_SETPHYWH; // Tag identifier (comment from isometimes)
    mbox[3] = 8; // Value size in bytes (comment from isometimes)
    mbox[4] = 0;
    mbox[5] = 1920; // Value(width) (comment from isometimes)
    mbox[6] = 1080; // Value(height) (comment from isometimes)

    mbox[7] = MBOX_TAG_SETVIRTWH;
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = 1920;
    mbox[11] = 1080;

    mbox[12] = MBOX_TAG_SETVIRTOFF;
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0; // Value(x) (comment from isometimes)
    mbox[16] = 0; // Value(y) (comment from isometimes)

    mbox[17] = MBOX_TAG_SETDEPTH;
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32; // Bits per pixel (comment from isometimes)

    mbox[21] = MBOX_TAG_SETPXLORDR;
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1; // RGB (comment from isometimes)

    mbox[25] = MBOX_TAG_GETFB;
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096; // FrameBufferInfo.pointer (comment from isometimes)
    mbox[29] = 0;    // FrameBufferInfo.size (comment from isometimes)

    mbox[30] = MBOX_TAG_GETPITCH;
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0; // Bytes per line (comment from isometimes)

    mbox[34] = MBOX_TAG_LAST;

    // Check call is successful and we have a pointer with depth 32 (comment from isometimes)
    if (mbox_call(MBOX_CH_PROP) && mbox[20] == 32 && mbox[28] != 0) {
        mbox[28] &= 0x3FFFFFFF; // Convert GPU address to ARM address (comment from isometimes)
        width = mbox[10];       // Actual physical width (comment from isometimes)
        height = mbox[11];      // Actual physical height (comment from isometimes)
        pitch = mbox[33];       // Number of bytes per line (comment from isometimes)
        isrgb = mbox[24];       // Pixel order
        fb = (unsigned char *)((long)mbox[28]);
    }
}
// Method to draw one pixel. This method is used every time we want to draw a shape on the screen.
void drawPixel(int x, int y, unsigned char attr)
{
    int offs = (y * pitch) + (x * 4);
    *((unsigned int*)(fb + offs)) = vgapal[attr & 0x0f];
}

// Method to draw a rectangle on given coordinates.
void drawRect(int x1, int y1, int x2, int y2, unsigned char attr, int fill)
{
    int y=y1;

    while (y <= y2) {
       int x=x1;
       // we start at (x1, y1)
       while (x <= x2) {
	  if ((x == x1 || x == x2) || (y == y1 || y == y2)) drawPixel(x, y, attr);
      // When we are either on the right x or y coordinate than we draw the pixel
	  else if (fill) drawPixel(x, y, (attr & 0xf0) >> 4);
          // if the 'fill' variable is '1' than not only the margins but also all pixels in between will be changed to the desired color. 
          x++;
       }
       y++;
    }
}

// Method to draw a line from point (x1,y1) to (x2, y2)
void drawLine(int x1, int y1, int x2, int y2, unsigned char attr)  
{  
    int dx, dy, p, x, y;

    dx = x2-x1; // length from x1 to x2
    dy = y2-y1; // length from y1 to y2
    x = x1;
    y = y1;
    p = 2*dy-dx; 
    // only draw if x is smaller than x2
    while (x<x2) {
        if (p >= 0) {
            // end up here if 2*dy >= dx
            drawPixel(x,y,attr);
            y++;
            p = p+2*dy-2*dx;
        } else {
            drawPixel(x,y,attr);
            p = p+2*dy;
        }
       x++;
    }
}

// Method to draw a circle with (x0, y0) as the center of the circle
void drawCircle(int x0, int y0, int radius, unsigned char attr, int fill)
{
    int x = radius;
    int y = 0;
    int err = 0;
 
    // starting with x == radius, y == 0 and err = 0
    while (x >= y) {
        if (fill) {
            // in here we fill up the circle with color. ignoring the offsets (x0, y0), we draw lines from:
            drawLine(x0 - y, y0 + x, x0 + y, y0 + x, (attr & 0xf0) >> 4); // -y, x, y, x (line from -y to y on hight x parallel to the y-axis)
            drawLine(x0 - x, y0 + y, x0 + x, y0 + y, (attr & 0xf0) >> 4); // -x, y, x, y (line from -x to x on hight y parallel to the y-axis)
            drawLine(x0 - x, y0 - y, x0 + x, y0 - y, (attr & 0xf0) >> 4); // -x, -y, x, y (line diagonal down to the right) 
            drawLine(x0 - y, y0 - x, x0 + y, y0 - x, (attr & 0xf0) >> 4); // -y, -x, y, -x (line diagonal down to the left)
        }
        // ignoring the offsets (x0, y0), we draw pixels on:
        drawPixel(x0 - y, y0 + x, attr); // -y, x
        drawPixel(x0 + y, y0 + x, attr); // y, x
        drawPixel(x0 - x, y0 + y, attr); // -x, y
        drawPixel(x0 + x, y0 + y, attr); // x, y
        drawPixel(x0 - x, y0 - y, attr); // -x, -y
        drawPixel(x0 + x, y0 - y, attr); // x, -y
        drawPixel(x0 - y, y0 - x, attr); // -x, -y
        drawPixel(x0 + y, y0 - x, attr); // y, -x

        if (err <= 0) {
            // making y bigger which creates a new layer of the circle
            y += 1;
            err += 2*y + 1;
        }
    
        if (err > 0) {
            // we only get here if 2*y + 1 < 0
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

// Method to draw chars out of the font enum from 'terminal.h'
void drawChar(unsigned char ch, int x, int y, unsigned char attr, int zoom)
{
    unsigned char *glyph = (unsigned char *)&font + (ch < FONT_NUMGLYPHS ? ch : 0) * FONT_BPG;

    // the zoom value is for the size of the char
    // here we draw every pixel from the values of the font enum (used through the glyph pointer)
    for (int i=1;i<=(FONT_HEIGHT*zoom);i++) {
	for (int j=0;j<(FONT_WIDTH*zoom);j++) {
	    unsigned char mask = 1 << (j/zoom);
	    unsigned char col = (*glyph & mask) ? attr & 0x0f : (attr & 0xf0) >> 4;

	    drawPixel(x+j, y+i, col);
	}
    // ? : for conditioal expressions:
    // if (i%zoom) than 0 else FONT_BPL
	glyph += (i%zoom) ? 0 : FONT_BPL;
    }
}

// Method to draw String using the drawChar method
void drawString(int x, int y, char *s, unsigned char attr, int zoom)
{
    while (*s) {
       if (*s == '\r') {
            // if the char is '\r' than we go back to the start of the line
          x = 0;
       } else if(*s == '\n') {
            // if the char is '\n' start a new line 
          x = 0; y += (FONT_HEIGHT*zoom); // FONT_HEIGHT * zoom is the line spacing
       } else {
	  drawChar(*s, x, y, attr, zoom);
          x += (FONT_WIDTH*zoom); // change where the next letter will start (more on the right with the size of FONT_WIDTH * zoom)
       }
       // get to the next letter of the char array (String)
       s++;
    }
}

// This method is used to wait in miliseconds
void wait_msec(unsigned int n)
{
    register unsigned long f, t, r;

    // Get the current counter frequency (comment from isometimes)
    asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));
    // Read the current counter
    asm volatile ("mrs %0, cntpct_el0" : "=r"(t));
    // Calculate expire value for counter (comment from isometimes)
    t+=((f/1000)*n)/1000;
    do{asm volatile ("mrs %0, cntpct_el0" : "=r"(r));}while(r<t);
}
