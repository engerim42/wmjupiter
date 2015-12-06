/* wmjupiter - Copyright (c) 2001 Thomas Kuiper <tkuiper@inxsoft.net> */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <X11/X.h>
#include <X11/xpm.h>
#include <math.h>
#include <values.h>
#include "xutils.h"
#include "wmJupiter_master.xpm"
#include "wmJupiter_mask.xbm"

typedef struct Coordinates {
    double x;
    double y;
} Coordinates;

#define PI 3.141592653589793238462643383279502884197169399375105

double lambda, lambda1, lambda2;
double jdistance;
int spotlat;

double De;			// planetocentric ang. dist. of earth from jup. equator
// Angles of each of the Galilean satellites, in radians,
// expressed relative to each satellite's inferior conjunction:
double moonAngles[5];
// And their distances from the planet:
double moonDist[5];
char moonLabels[5];

/* compute the julian date between 1901 - 2099 (Sinnott 1991, p. 183) */

double getJulianDate(double year, double month, double day,
		     double uh, double um, double us)
{
    return 367 * year - (int) (7 * (year + (int) ((month + 9) / 12)) / 4)
	+ (int) (275 * month / 9) + day + 1721013.5
	+ (uh + (um + us / 60.0) / 60.0) / 24.0;

}

double oangle(double a)
{
    while (a > 2 * PI)
	a -= 2. * PI;
    while (a < 0)
	a += 2. * PI;
    return a;
}


double angle(double a)
{
    if (a < 10000)
	return oangle(a);
    a = a - 2. * PI * (int) (a / 2. / PI);
    if (a < 0)
	a += 2 * PI;
    return a;
}

Coordinates getRedSpotXY(int spot_in_deg)
{
    double spotlong = angle(lambda2 - spot_in_deg * PI / 180);
    Coordinates coord;


    // See if the spot is visible:
    if (spotlong > PI * .5 && spotlong < PI * 1.5) {
	coord.x = coord.y = MAXDOUBLE;
    } else {
	coord.x = sin(spotlong);
	coord.y = .42;		// completely random wild-assed guess

    }


    return coord;
}

struct tm *tmjupiter = NULL;
void computepos(double d);
void findspot()
{
    time_t t;
    double d;

    t = time(0);

    tmjupiter = gmtime(&t);


    d = getJulianDate(tmjupiter->tm_year + 1900, tmjupiter->tm_mon + 1,
		      tmjupiter->tm_mday, tmjupiter->tm_hour,
		      tmjupiter->tm_min, tmjupiter->tm_sec) - 2415020;
    computepos(d);

}

void drawspot()
{
    int jsize;
    int xcenter;
    int ycenter;
    int x, y, j2width, spotwidth;
    Coordinates coord;
    int redSpotWidth, redSpotHeight;
    redSpotWidth = 3;
    redSpotHeight = 4;
    xcenter = 26;
    jsize = 10;
    ycenter = 32;

    if (tmjupiter == NULL)
	return;

    coord = getRedSpotXY(spotlat);

    if (coord.y == MAXDOUBLE) {
    } else {

    }

    if (coord.x != MAXDOUBLE && coord.y != MAXDOUBLE) {
	// We only want the spot to draw to the edge of the disk,
	// so adjust the width if necessary:
	// (approx) width/2 of the planet (pixels) at the spot's latitude:
	j2width = (int) (jsize - abs(coord.y * jsize) / 3);
	x = xcenter + (int) (coord.x * jsize);
	y = ycenter + (int) (coord.y * jsize) - redSpotHeight / 2;
	spotwidth = redSpotWidth;
	if (x + redSpotWidth > xcenter + j2width) {
	    spotwidth = xcenter + j2width - x;
	    x = xcenter + j2width - spotwidth;
	} else if (x - redSpotWidth / 2 < xcenter - j2width) {
	    spotwidth = x + redSpotWidth / 2 - xcenter + j2width;
	    x = xcenter - j2width;
	} else {
	    x -= redSpotWidth / 2;
	}

	copyXPMArea(73, 88, 4, 3, x + 2, 28);

    }

}


double getMoonDist(int whichmoon)
{
    return moonDist[whichmoon];
}

void drawmoons()
{
    Coordinates coord;
    int i;
    char chr;
    double r;
    int moonsize = 5;

    for (i = 0; i < 4; i++) {
	r = getMoonDist(i);
	coord.x = r * sin(moonAngles[i]);
	coord.y = -r * cos(moonAngles[i]) * sin(De);

	if (coord.x < 1. && coord.x > -1.
	    && moonAngles[i] > PI * .5 && moonAngles[i] < PI * 1.5) {
	    coord.x = coord.y = MAXDOUBLE;
	}

	if ((coord.x != MAXDOUBLE)
	    && (coord.y != MAXDOUBLE)) {
	    int x = 32 + (int) (coord.x * 10) - moonsize / 2;
	    int y = 32 - (int) (coord.y * 10) - moonsize / 2;
	    if ((x > 2) && (x < 55) && (y > 2) && (y < 60)) {
		copyXPMArea(78, 88, 4, 4, x, y);
		chr = moonLabels[i] - 65;
		copyXPMArea(chr * 5 + 2, 128, 5, 6, x, 6);
	    }
	}
    }
}


void computepos(double d)
{


    double psi;
    double delta;		/* Earth-Jupiter distance */
    double V, M, N, J, A, B, K, R, r, G, H;

    // Argument for the long-period term in the motion of Jupiter:
    V = angle((134.63 + .00111587 * d) * PI / 180);

    // Mean anomalies of Earth and Jupiter:
    M = angle((358.476 + .9856003 * d) * PI / 180);
    N = angle((225.328 + .0830853 * d + .33 * sin(V))
	      * PI / 180);

    // Diff between the mean heliocentric longitudes of Earth & Jupiter:
    J = angle((221.647 + .9025179 * d - .33 * sin(V))
	      * PI / 180);

    // Equations of the center of Earth and Jupiter:
    A = angle((1.916 * sin(M) + .020 * sin(2 * M))
	      * PI / 180);
    B = angle((5.552 * sin(N) + .167 * sin(2 * N))
	      * PI / 180);

    K = angle(J + A - B);

    // Distances are specified in AU:
    // Radius vector of the earth:
    R = 1.00014 - .01672 * cos(M) - .00014 * cos(2 * M);
    // Radius vector of Jupiter:
    r = 5.20867 - .25192 * cos(N) - .00610 * cos(2 * N);

    // Earth-Jupiter distance:
    delta = sqrt(r * r + R * R - 2 * r * R * cos(K));
    jdistance = delta;

    // Phase angle of Jupiter (always btw. -12 and 12 degrees):
    psi = asin(R / delta * sin(K));

    // Longitude of system 1:
    lambda1 = angle((268.28 * 877.8169088 * (d - delta / 173))
		    * PI / 180 + psi - B);
    // Longitude of system 2:
    lambda2 = angle((290.28 + 870.1869088 * (d - delta / 173))
		    * PI / 180 + psi - B);

    // calculate the angles of each of the satellites:
    moonAngles[0] = angle((84.5506 + 203.4058630 * (d - delta / 173))
			  * PI / 180 + psi - B);
    moonAngles[1] = angle((41.5015 + 101.2916323 * (d - delta / 173))
			  * PI / 180 + psi - B);
    moonAngles[2] = angle((109.9770 + 50.2345169 * (d - delta / 173))
			  * PI / 180 + psi - B);
    moonAngles[3] = oangle((176.3586 + 21.4879802 * (d - delta / 173))
			   * PI / 180 + psi - B);

    // and the planetocentric angular distance of the earth
    // from the equator of Jupiter:
    lambda = angle((238.05 + .083091 * d + .33 * sin(V))
		   * PI / 180 + B);
    De = ((3.07 * sin(lambda + 44.5 * PI / 180)
	   - 2.15 * sin(psi) * cos(lambda - 24. * PI / 180)
	   - 1.31 * (r - delta) / delta * sin(lambda - 99.4 * PI / 180))
	  * PI / 180);

    G = angle((187.3 + 50.310674 * (d - delta / 173)) * PI / 180);
    H = angle((311.1 + 21.569229 * (d - delta / 173)) * PI / 180);

    // Calculate the distances before any corrections are applied:
    moonDist[0] = 5.9061 -
	.0244 * cos(2 * (moonAngles[0] - moonAngles[1]));
    moonDist[1] = 9.3972 -
	.0889 * cos(2 * (moonAngles[1] - moonAngles[2]));
    moonDist[2] = 14.9894 - .0227 * cos(G);
    moonDist[3] = 26.3649 - .1944 * cos(H);

    // apply some first-order correction terms to the angles:
    moonAngles[0] = angle(moonAngles[0] +
			  sin(2 * (moonAngles[0] - moonAngles[1]))
			  * .472 * PI / 180);
    moonAngles[1] = angle(moonAngles[1] +
			  sin(2 * (moonAngles[1] - moonAngles[2]))
			  * 1.073 * PI / 180);
    moonAngles[2] = angle(moonAngles[2] + sin(G) * .174 * PI / 180);
    moonAngles[3] = angle(moonAngles[3] + sin(H) * .845 * PI / 180);

}

/* 
 *  Delay between refreshes (in microseconds) 
 */
#define DELAY 10000L
#define DEFAULT_UPDATEDELAY 900L

void ButtonPressEvent(XButtonEvent *);
void KeyPressEvent(XKeyEvent *);
char *StringToUpper(char *);


int UpToDate = 0;
long UpdateDelay;
int GotFirstClick1, GotDoubleClick1;
int GotFirstClick2, GotDoubleClick2;
int GotFirstClick3, GotDoubleClick3;
int DblClkDelay;


char LabelColor[30] = "#79bdbf";
char WindGustColor[30] = "#ff0000";
char DataColor[30] = "#ffbf50";
char BackColor[30] = "#181818";
char StationTimeColor[30] = "#c5a6ff";



int main(int argc, char *argv[])
{

    XEvent event;
    int m, dt1, dt2, dt3;
    Window myWindow;
    char tempstr[255];
    int q, i;
    double val;
    int digit;
    m = 101;
    initXwindow(argc, argv);
    myWindow =
	openXwindow(argc, argv, wmJupiter_master, wmJupiter_mask_bits,
		    wmJupiter_mask_width, wmJupiter_mask_height, BackColor,
		    LabelColor, WindGustColor, DataColor,
		    StationTimeColor);

    if (argc < 2) {
	printf
	    ("usage: wmJupiter <spot position>\n  spot position in lat. (e.g. 80)\n");
	return 1;
    }

    spotlat = atoi(argv[1]);

    moonLabels[0] = 'I';
    moonLabels[1] = 'E';
    moonLabels[2] = 'G';
    moonLabels[3] = 'C';

    findspot();

    DblClkDelay = 32000;
    UpToDate = 0;
    while (1) {

	if (DblClkDelay > 15) {

	    DblClkDelay = 0;
	    GotFirstClick1 = 0;
	    GotDoubleClick1 = 0;
	    GotFirstClick2 = 0;
	    GotDoubleClick2 = 0;
	    GotFirstClick3 = 0;
	    GotDoubleClick3 = 0;

	} else {

	    ++DblClkDelay;

	}


	/* 
	 *   Process any pending X events.
	 */
	while (XPending(display)) {
	    XNextEvent(display, &event);
	    switch (event.type) {
	    case Expose:
		RedrawWindow();
		break;
	    case ButtonPress:
		ButtonPressEvent(&event.xbutton);
		break;
	    case KeyPress:
		KeyPressEvent(&event.xkey);
		break;
	    case ButtonRelease:
		break;
	    case EnterNotify:
		XSetInputFocus(display, PointerRoot, RevertToParent,
			       CurrentTime);
		break;
	    case LeaveNotify:
		XSetInputFocus(display, PointerRoot, RevertToParent,
			       CurrentTime);
		break;

	    }
	}

	q = 0;
/*	    copyXPMArea(5, 69, 54, 54, 5, 5);
	    chr = 'A' - 65;
			 copyXPMArea(chr*5+2, 128, 5, 6, 7+q, 6); q+= 5;
	    chr = 'B' - 65;
			 copyXPMArea(chr*5+2, 128, 5, 6, 7+q, 6); q+= 5;
*/
	copyXPMArea(5, 69, 54, 54, 4, 4);

	drawspot();
	copyXPMArea(82, 90, 7, 18, 14, 15);
	copyXPMArea(82, 90, 7, 18, 41, 15);
	drawmoons();

	sprintf(tempstr, "%2.5f", jdistance);


	q = 0;
	val = jdistance;
	for (i = 0; i < 7; i++) {
	    if (tempstr[i] == '\0')
		break;
	    digit = tempstr[i] - 48;
	    if (digit >= 0) {
		copyXPMArea(digit * 5 + 66, 57, 5, 6, 22 + q, 50);
		q += 5;
	    } else {
		copyXPMArea(10 * 5 + 66, 57, 5, 6, 22 + q, 50);
		q += 5;
	    }
	}

	if (m > 100) {

	    findspot();
	    m = 0;
	    ++dt1;
	    ++dt2;
	    ++dt3;


	} else {

	    /*
	     *  Increment counter
	     */
	    ++m;

	}

	RedrawWindow();

	usleep(DELAY);
    }
}




void ButtonPressEvent(XButtonEvent * xev)
{

    /*
     *  Process single clicks.
     */
    DblClkDelay = 0;
    if ((xev->button == Button1) && (xev->type == ButtonPress)) {

	if (GotFirstClick1)
	    GotDoubleClick1 = 1;
	else
	    GotFirstClick1 = 1;

    } else if ((xev->button == Button2) && (xev->type == ButtonPress)) {

	if (GotFirstClick2)
	    GotDoubleClick2 = 1;
	else
	    GotFirstClick2 = 1;

    } else if ((xev->button == Button3) && (xev->type == ButtonPress)) {

	if (GotFirstClick3)
	    GotDoubleClick3 = 1;
	else
	    GotFirstClick3 = 1;

    }




    /*
     *  We got a double click on Mouse Button1 (i.e. the left one)
     */
    if (GotDoubleClick1) {
	GotFirstClick1 = 0;
	GotDoubleClick1 = 0;
    }


    /*
     *  We got a double click on Mouse Button2 (i.e. the left one)
     */
    if (GotDoubleClick2) {
	GotFirstClick2 = 0;
	GotDoubleClick2 = 0;
    }


    /*
     *  We got a double click on Mouse Button3 (i.e. the left one)
     */
    if (GotDoubleClick3) {
	GotFirstClick3 = 0;
	GotDoubleClick3 = 0;
    }

    return;


}




/*
 *  This routine handles key presses.
 *
 */
void KeyPressEvent(XKeyEvent * xev)
{

    return;

}


char *StringToUpper(char *String)
{

    int i;

    for (i = 0; i < strlen(String); i++)
	String[i] = toupper(String[i]);

    return String;

}
