/**
 * Copyright Alexandru Olaru
 * See LICENSE file for copyright and license details
 */

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/shape.h>
#include <stdio.h>
#include <stdlib.h>

static Window nightlight_create_window(Display* dpy, Window root, int screen)
{
    XVisualInfo vinfo;
    XMatchVisualInfo(dpy, screen, 32, TrueColor, &vinfo);

    XSetWindowAttributes wa;
    wa.colormap = XCreateColormap(dpy, root, vinfo.visual, AllocNone);
    if (!wa.colormap)
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

    int XA_NET_WM_STATE = XInternAtom(dpy, "_NET_WM_STATE", False);
    int XA_NET_WM_STATE_ABOVE = XInternAtom(dpy, "_NET_WM_STATE_ABOVE", False);

    Atom va;
    va = XA_NET_WM_STATE_ABOVE;
    XChangeProperty(
        dpy,
        win,
        XA_NET_WM_STATE,
        XA_ATOM,
        32,
        PropModeReplace,
        (unsigned char*)&va,
        1);

    /* Set the input area of the window be empty, so no events are captured */
    XserverRegion region = XFixesCreateRegion(dpy, 0, 0);
    if (!region)
    {
        fprintf(stderr, "Failed to create X server region!\n");
        return 0;
    }

    XFixesSetWindowShapeRegion(dpy, win, ShapeBounding, 0, 0, 0);
    XFixesSetWindowShapeRegion(dpy, win, ShapeInput, 0, 0, region);
    XFixesDestroyRegion(dpy, region);

    XMapRaised(dpy, win);

    /* Override Redirect windows don't generate a MapRequest therefore the WM
     * doesn't get notified of them, so we force one here. I don't know if this
     * works for other WMs but in (my modified) dwm, override redirect windows
     * are "managed" separately and always pushed to the top of the window stack
     * on any change. This makes sure they are alaways on top even when
     * floating. This however, doesn't fix them still being behind other
     * override redirect windows that don't generate their own forced
     * MapRequest. I don't think this can be fixed without having those windows
     * also map themselves...
     * */
    XEvent ev;
    ev.xmaprequest.type = MapRequest;
    ev.xmaprequest.window = win;
    XSendEvent(
        dpy,
        DefaultRootWindow(dpy),
        False,
        SubstructureRedirectMask | SubstructureNotifyMask,
        &ev);

    return win;
}

int main(int argc, char** argv)
{
    Display* dpy = XOpenDisplay(NULL);
    if (!dpy)
    {
        fprintf(stderr, "Failed to open X display\n!");
        return 1;
    }

    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);

    Window win = nightlight_create_window(dpy, root, screen);
    if (!win)
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
