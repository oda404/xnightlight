/**
 * Copyright Alexandru Olaru
 * See LICENSE file for copyright and license details
 */

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/shape.h>

static Window nightlight_create_window(Display *dpy, Window root, int screen)
{
    XVisualInfo vinfo;
    XMatchVisualInfo(dpy, screen, 32, TrueColor, &vinfo);

    XSetWindowAttributes wa;
    wa.colormap = XCreateColormap(dpy, root, vinfo.visual, AllocNone);
    if (wa.colormap == 0)
    {
        fprintf(stderr, "Failed to create X colormap!\n");
        return 0;
    }

    wa.border_pixel = 0;
    // value fresh from my ass (gimp)
    wa.background_pixel = 0x88150f00;
    wa.override_redirect = 1;

    Window win = XCreateWindow(
        dpy,
        root,
        0,
        0,
        DisplayWidth(dpy, screen),
        DisplayHeight(dpy, screen),
        0,
        vinfo.depth,
        InputOutput,
        vinfo.visual,
        CWOverrideRedirect | CWColormap | CWBackPixel | CWBorderPixel,
        &wa);

    if (!win)
    {
        fprintf(stderr, "Failed to create X window!\n");
        return 0;
    }

    XSelectInput(dpy, win, ExposureMask);

    /* Set the input area of the window be empty, so no events are captured */
    XserverRegion region = XFixesCreateRegion(dpy, 0, 0);
    if (region == 0)
    {
        fprintf(stderr, "Failed to create X server region!\n");
        return 0;
    }

    XFixesSetWindowShapeRegion(dpy, win, ShapeBounding, 0, 0, 0);
    XFixesSetWindowShapeRegion(dpy, win, ShapeInput, 0, 0, region);
    XFixesDestroyRegion(dpy, region);

    XMapRaised(dpy, win);
    return win;
}

int main(int argc, char **argv)
{
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy)
    {
        fprintf(stderr, "Failed to open X display\n!");
        return 1;
    }

    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);

    Window win = nightlight_create_window(dpy, root, screen);
    if (win == 0)
    {
        XCloseDisplay(dpy);
        return 1;
    }

    XEvent ev;
    while (!XNextEvent(dpy, &ev))
        XRaiseWindow(dpy, win);

    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    return 0;
}
