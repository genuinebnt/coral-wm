#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "log.h"

bool g_wm_detected;
Display *g_display;
Window g_root;

int OnWMDetected(Display *display, XErrorEvent *error) {
  if (error->error_code == BadAccess) {
    g_wm_detected = true;
  }

  return 0;
}

int OnXError(Display *display, XErrorEvent *error) {
  Logger("Error", "Error Code: ", error->error_code);
}

void OnCreateNotify(const XCreateWindowEvent *event) {}
void OnConfigureRequest(const XConfigureRequestEvent *event) {
  XWindowChanges *changes;
  changes->x = event->x;
  changes->y = event->y;
  changes->width = event->width;
  changes->height = event->height;
  changes->border_width = event->border_width;
  changes->sibling = event->above;
  changes->stack_mode = event->detail;

  XConfigureWindow(g_display, event->window, event->value_mask, changes);
  Logger("INFO", "Resize", event->window, " to", event->width, "x",
         event->height);
}

void Frame(Window window) {
  const unsigned int BORDER_COLOR = 0x00FF00;
  const unsigned int BORDER_WIDTH = 3;
  const unsigned int BG_COLOR = 0x0000FF;

  XWindowAttributes x_win_attr;
  XGetWindowAttributes(g_display, window, &x_win_attr);

  const Window frame = XCreateSimpleWindow(
      g_display, g_root, x_win_attr.x, x_win_attr.y, x_win_attr.width,
      x_win_attr.height, BORDER_WIDTH, BORDER_COLOR, BG_COLOR);

  XSelectInput(g_display, frame,
               SubstructureRedirectMask | SubstructureNotifyMask);
  XAddToSaveSet(g_display, window);

  XReparentWindow(g_display, window, frame, 0, 0);
  XMapWindow(g_display, frame);

  Logger("INFO", "Framed window");
}

void OnReparentNotify(const XReparentEvent *event) {}
void OnMapNotify(const XMapEvent *event) {}

void OnMapRequest(const XMapRequestEvent *event) { Frame(event->window); }
void OnDestroyNofity(const XDestroyWindowEvent *event){};

void Run() {
  g_wm_detected = false;
  XSetErrorHandler(&OnWMDetected);

  XSelectInput(g_display, g_root,
               SubstructureRedirectMask | SubstructureNotifyMask);

  XSync(g_display, false);
  if (g_wm_detected) {
    Logger("Error", "Detected another window manager running",
           XDisplayString(g_display));
    return;
  }

  XSetErrorHandler(OnXError);

  for (;;) {
    XEvent *e;
    XNextEvent(g_display, e);

    switch (e->type) {
    case CreateNotify:
      OnCreateNotify(&e->xcreatewindow);
      break;
    case DestroyNotify:
      OnDestroyNotify(&e->xdestroywindow);
      break;
    case ReparentNotify:
      OnReparentNotify(&e->xreparent);
      break;
    default:
      Logger("Warning", "Ignored event");
    }
  }
}

int main() {

  g_display = XOpenDisplay(NULL);
  if (g_display == NULL) {
    Logger("Error", "Failed to open display: ", XDisplayName(NULL));
    return -1;
  }

  g_root = DefaultRootWindow(g_display);

  Run();

  XCloseDisplay(g_display);
}