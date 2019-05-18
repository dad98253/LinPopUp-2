#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xmu/WinUtil.h>	/* for XmuClientWindow() */



/* -*- Mode:C; tab-width: 8 -*-
 * remote.c --- remote control of Netscape Navigator for Unix.
 * version 1.1.3, for Netscape Navigator 1.1 and newer.
 *
 * Copyright © 1996 Netscape Communications Corporation, all rights reserved.
 * Created: Jamie Zawinski <jwz@netscape.com>, 24-Dec-94.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * To compile:
 *
 *    cc -o netscape-remote remote.c -DSTANDALONE -lXmu -lX11
 *
 * To use:
 *
 *    netscape-remote -help
 *
 * Documentation for the protocol which this code implements may be found at:
 *
 *    http://home.netscape.com/newsref/std/x-remote.html
 *
 * Bugs and commentary to x_cbug@netscape.com.
 */



/* vroot.h is a header file which lets a client get along with `virtual root'
   window managers like swm, tvtwm, olvwm, etc.  If you don't have this header
   file, you can find it at "http://home.netscape.com/newsref/std/vroot.h".
   If you don't care about supporting virtual root window managers, you can
   comment this line out.
 */
/* #include "vroot.h" */
/* Begin vroot.h */
/*****************************************************************************/
/**                   Copyright 1991 by Andreas Stolcke                     **/
/**               Copyright 1990 by Solbourne Computer Inc.                 **/
/**                          Longmont, Colorado                             **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    name of Solbourne not be used in advertising                         **/
/**    in publicity pertaining to distribution of the  software  without    **/
/**    specific, written prior permission.                                  **/
/**                                                                         **/
/**    ANDREAS STOLCKE AND SOLBOURNE COMPUTER INC. DISCLAIMS ALL WARRANTIES **/
/**    WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF    **/
/**    MERCHANTABILITY  AND  FITNESS,  IN  NO  EVENT SHALL ANDREAS STOLCKE  **/
/**    OR SOLBOURNE BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL    **/
/**    DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA   **/
/**    OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER    **/
/**    TORTIOUS ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE    **/
/**    OR PERFORMANCE OF THIS SOFTWARE.                                     **/
/*****************************************************************************/
/*
 * vroot.h -- Virtual Root Window handling header file
 *
 * This header file redefines the X11 macros RootWindow and DefaultRootWindow,
 * making them look for a virtual root window as provided by certain `virtual'
 * window managers like swm and tvtwm. If none is found, the ordinary root
 * window is returned, thus retaining backward compatibility with standard
 * window managers.
 * The function implementing the virtual root lookup remembers the result of
 * its last invocation to avoid overhead in the case of repeated calls
 * on the same display and screen arguments. 
 * The lookup code itself is taken from Tom LaStrange's ssetroot program.
 *
 * Most simple root window changing X programs can be converted to using
 * virtual roots by just including
 *
 * #include <X11/vroot.h>
 *
 * after all the X11 header files.  It has been tested on such popular
 * X clients as xphoon, xfroot, xloadimage, and xaqua.
 * It also works with the core clients xprop, xwininfo, xwd, and editres
 * (and is necessary to get those clients working under tvtwm).
 * It does NOT work with xsetroot; get the xsetroot replacement included in
 * the tvtwm distribution instead.
 *
 * Andreas Stolcke <stolcke@ICSI.Berkeley.EDU>, 9/7/90
 * - replaced all NULL's with properly cast 0's, 5/6/91
 * - free children list (suggested by Mark Martin <mmm@cetia.fr>), 5/16/91
 * - include X11/Xlib.h and support RootWindowOfScreen, too 9/17/91
 */

#ifndef _VROOT_H_
#define _VROOT_H_


static Window
VirtualRootWindowOfScreen(screen)
	Screen *screen;
{
	static Screen *save_screen = (Screen *)0;
	static Window root = (Window)0;

	if (screen != save_screen) {
		Display *dpy = DisplayOfScreen(screen);
		Atom __SWM_VROOT = None;
		int i;
		Window rootReturn, parentReturn, *children;
		unsigned int numChildren;

		root = RootWindowOfScreen(screen);

		/* go look for a virtual root */
		__SWM_VROOT = XInternAtom(dpy, "__SWM_VROOT", False);
		if (XQueryTree(dpy, root, &rootReturn, &parentReturn,
				 &children, &numChildren)) {
			for (i = 0; i < numChildren; i++) {
				Atom actual_type;
				int actual_format;
				unsigned long nitems, bytesafter;
				Window *newRoot = (Window *)0;

				if (XGetWindowProperty(dpy, children[i],
					__SWM_VROOT, 0, 1, False, XA_WINDOW,
					&actual_type, &actual_format,
					&nitems, &bytesafter,
					(unsigned char **) &newRoot) == Success
				    && newRoot) {
				    root = *newRoot;
				    break;
				}
			}
			if (children)
				XFree((char *)children);
		}

		save_screen = screen;
	}

	return root;
}

#undef RootWindowOfScreen
#define RootWindowOfScreen(s) VirtualRootWindowOfScreen(s)

#undef RootWindow
#define RootWindow(dpy,screen) VirtualRootWindowOfScreen(ScreenOfDisplay(dpy,screen))

#undef DefaultRootWindow
#define DefaultRootWindow(dpy) VirtualRootWindowOfScreen(DefaultScreenOfDisplay(dpy))

#endif /* _VROOT_H_ */
/* End vroot.h */













#ifdef DIAGNOSTIC
#ifdef STANDALONE
 static const char *progname = 0;
 static const char *expected_mozilla_version = "1.1";
#else  /* !STANDALONE */
 extern const char *progname;
 extern const char *expected_mozilla_version;
#endif /* !STANDALONE */
#endif /* DIAGNOSTIC */

#define MOZILLA_VERSION_PROP   "_MOZILLA_VERSION"
#define MOZILLA_LOCK_PROP      "_MOZILLA_LOCK"
#define MOZILLA_COMMAND_PROP   "_MOZILLA_COMMAND"
#define MOZILLA_RESPONSE_PROP  "_MOZILLA_RESPONSE"
static Atom XA_MOZILLA_VERSION  = 0;
static Atom XA_MOZILLA_LOCK     = 0;
static Atom XA_MOZILLA_COMMAND  = 0;
static Atom XA_MOZILLA_RESPONSE = 0;

static void
mozilla_remote_init_atoms (Display *dpy)
{
  if (! XA_MOZILLA_VERSION)
    XA_MOZILLA_VERSION = XInternAtom (dpy, MOZILLA_VERSION_PROP, False);
  if (! XA_MOZILLA_LOCK)
    XA_MOZILLA_LOCK = XInternAtom (dpy, MOZILLA_LOCK_PROP, False);
  if (! XA_MOZILLA_COMMAND)
    XA_MOZILLA_COMMAND = XInternAtom (dpy, MOZILLA_COMMAND_PROP, False);
  if (! XA_MOZILLA_RESPONSE)
    XA_MOZILLA_RESPONSE = XInternAtom (dpy, MOZILLA_RESPONSE_PROP, False);
}

static Window
mozilla_remote_find_window (Display *dpy)
{
  int i;
  Window root = RootWindowOfScreen (DefaultScreenOfDisplay (dpy));
  Window root2, parent, *kids;
  unsigned int nkids;
  Window result = 0;

  if (! XQueryTree (dpy, root, &root2, &parent, &kids, &nkids))
    {
#ifdef DIAGNOSTIC
      fprintf (stderr, "%s: XQueryTree failed on display %s\n", progname,
	       DisplayString (dpy));
#endif
      return (0);
    }

  /* root != root2 is possible with virtual root WMs. */

  if (! (kids && nkids))
    {
#ifdef DIAGNOSTIC
      fprintf (stderr, "%s: root window has no children on display %s\n",
	       progname, DisplayString (dpy));
#endif
      return (0);
    }

  for (i = nkids-1; i >= 0; i--)
    {
      Atom type;
      int format;
      unsigned long nitems, bytesafter;
      unsigned char *version = 0;
      Window w = XmuClientWindow (dpy, kids[i]);
      int status = XGetWindowProperty (dpy, w, XA_MOZILLA_VERSION,
				       0, (65536 / sizeof (long)),
				       False, XA_STRING,
				       &type, &format, &nitems, &bytesafter,
				       &version);
      if (! version)
	continue;

      XFree (version);
      if (status == Success && type != None)
	{
	  result = w;
	  break;
	}
    }

  return result;
}

static char *lock_data = 0;
#define LOCK_DATA_SIZE 255

static int
mozilla_remote_obtain_lock (Display *dpy, Window window)
{
  Bool locked = False;
  Bool waited = False;

  if (! lock_data)
    {
      lock_data = (char *) malloc (LOCK_DATA_SIZE);
      g_snprintf (lock_data, LOCK_DATA_SIZE, "pid%d@", getpid ());
      if (gethostname (lock_data + strlen (lock_data), 100))
	{
	  return (-1);
	}
    }

  do
    {
      int result;
      Atom actual_type;
      int actual_format;
      unsigned long nitems, bytes_after;
      unsigned char *data = 0;

      XGrabServer (dpy);   /* ################################# DANGER! */

      result = XGetWindowProperty (dpy, window, XA_MOZILLA_LOCK,
				   0, (65536 / sizeof (long)),
				   False, /* don't delete */
				   XA_STRING,
				   &actual_type, &actual_format,
				   &nitems, &bytes_after,
				   &data);
      if (result != Success || actual_type == None)
	{
	  /* It's not now locked - lock it. */
#ifdef DEBUG_PROPS
	  fprintf (stderr, "%s: (writing " MOZILLA_LOCK_PROP
		   " \"%s\" to 0x%x)\n",
		   progname, lock_data, (unsigned int) window);
#endif
	  XChangeProperty (dpy, window, XA_MOZILLA_LOCK, XA_STRING, 8,
			   PropModeReplace, (unsigned char *) lock_data,
			   strlen (lock_data));
	  locked = True;
	}

      XUngrabServer (dpy); /* ################################# danger over */
      XSync (dpy, False);

      if (! locked)
	{
	  /* We tried to grab the lock this time, and failed because someone
	     else is holding it already.  So, wait for a PropertyDelete event
	     to come in, and try again. */

#ifdef DIAGNOSTIC
	  fprintf (stderr, "%s: window 0x%x is locked by %s; waiting...\n",
		   progname, (unsigned int) window, data);
#endif
	  waited = True;

	  while (1)
	    {
	      XEvent event;
	      XNextEvent (dpy, &event);
	      if (event.xany.type == DestroyNotify &&
		  event.xdestroywindow.window == window)
		{
#ifdef DIAGNOSTIC
		  fprintf (stderr, "%s: window 0x%x unexpectedly destroyed.\n",
			   progname, (unsigned int) window);
#endif
		  return (6);
		}
	      else if (event.xany.type == PropertyNotify &&
		       event.xproperty.state == PropertyDelete &&
		       event.xproperty.window == window &&
		       event.xproperty.atom == XA_MOZILLA_LOCK)
		{
		  /* Ok!  Someone deleted their lock, so now we can try
		     again. */
#ifdef DEBUG_PROPS
		  fprintf (stderr, "%s: (0x%x unlocked, trying again...)\n",
			   progname, (unsigned int) window);
#endif
		  break;
		}
	    }
	}
      if (data)
	XFree (data);
    }
  while (! locked);

#ifdef DIAGNOSTIC
  if (waited)
    fprintf (stderr, "%s: obtained lock.\n", progname);
#endif
  return (0);
}


static void
mozilla_remote_free_lock (Display *dpy, Window window)
{
  int result;
  Atom actual_type;
  int actual_format;
  unsigned long nitems, bytes_after;
  unsigned char *data = 0;

#ifdef DEBUG_PROPS
	  fprintf (stderr, "%s: (deleting " MOZILLA_LOCK_PROP
		   " \"%s\" from 0x%x)\n",
		   progname, lock_data, (unsigned int) window);
#endif

  result = XGetWindowProperty (dpy, window, XA_MOZILLA_LOCK,
			       0, (65536 / sizeof (long)),
			       True, /* atomic delete after */
			       XA_STRING,
			       &actual_type, &actual_format,
			       &nitems, &bytes_after,
			       &data);
  if (result != Success)
    {
#ifdef DIAGNOSTIC
      fprintf (stderr, "%s: unable to read and delete " MOZILLA_LOCK_PROP
	       " property\n",
	       progname);
#endif
      return;
    }
  else if (!data || !*data)
    {
#ifdef DIAGNOSTIC
      fprintf (stderr, "%s: invalid data on " MOZILLA_LOCK_PROP
	       " of window 0x%x.\n",
	       progname, (unsigned int) window);
#endif
      return;
    }
  else if (strcmp ((char *) data, lock_data))
    {
#ifdef DIAGNOSTIC
      fprintf (stderr, "%s: " MOZILLA_LOCK_PROP
	       " was stolen!  Expected \"%s\", saw \"%s\"!\n",
	       progname, lock_data, data);
#endif
      return;
    }

  if (data)
    XFree (data);
}


static int
mozilla_remote_command (Display *dpy, Window window, const char *command)
{
  int result = 0;
  Bool done = False;

#ifdef DEBUG_PROPS
  fprintf (stderr, "%s: (writing " MOZILLA_COMMAND_PROP " \"%s\" to 0x%x)\n",
	   progname, command, (unsigned int) window);
#endif

  XChangeProperty (dpy, window, XA_MOZILLA_COMMAND, XA_STRING, 8,
		   PropModeReplace, (unsigned char *) command,
		   strlen (command));

  while (!done)
    {
      XEvent event;
      XNextEvent (dpy, &event);
      if (event.xany.type == DestroyNotify &&
	  event.xdestroywindow.window == window)
	{
	  /* Print to warn user...*/
#ifdef DIAGNOSTIC
	  fprintf (stderr, "%s: window 0x%x was destroyed.\n",
		   progname, (unsigned int) window);
#endif
	  result = 6;
	  goto DONE;
	}
      else if (event.xany.type == PropertyNotify &&
	       event.xproperty.state == PropertyNewValue &&
	       event.xproperty.window == window &&
	       event.xproperty.atom == XA_MOZILLA_RESPONSE)
	{
	  Atom actual_type;
	  int actual_format;
	  unsigned long nitems, bytes_after;
	  unsigned char *data = 0;

	  result = XGetWindowProperty (dpy, window, XA_MOZILLA_RESPONSE,
				       0, (65536 / sizeof (long)),
				       True, /* atomic delete after */
				       XA_STRING,
				       &actual_type, &actual_format,
				       &nitems, &bytes_after,
				       &data);
#ifdef DEBUG_PROPS
	  if (result == Success && data && *data)
	    {
	      fprintf (stderr, "%s: (server sent " MOZILLA_RESPONSE_PROP
		       " \"%s\" to 0x%x.)\n",
		       progname, data, (unsigned int) window);
	    }
#endif

	  if (result != Success)
	    {
#ifdef DIAGNOSTIC
	      fprintf (stderr, "%s: failed reading " MOZILLA_RESPONSE_PROP
		       " from window 0x%0x.\n",
		       progname, (unsigned int) window);
#endif
	      result = 6;
	      done = True;
	    }
	  else if (!data || strlen((char *) data) < 5)
	    {
#ifdef DIAGNOSTIC
	      fprintf (stderr, "%s: invalid data on " MOZILLA_RESPONSE_PROP
		       " property of window 0x%0x.\n",
		       progname, (unsigned int) window);
#endif
	      result = 6;
	      done = True;
	    }
	  else if (*data == '1')	/* positive preliminary reply */
	    {
#ifdef DIAGNOSTIC
	      fprintf (stderr, "%s: %s\n", progname, data + 4);
#endif
	      /* keep going */
	      done = False;
	    }
#if 1
	  else if (!strncmp ((char *)data, "200", 3)) /* positive completion */
	    {
	      result = 0;
	      done = True;
	    }
#endif
	  else if (*data == '2')		/* positive completion */
	    {
#ifdef DIAGNOSTIC
	      fprintf (stderr, "%s: %s\n", progname, data + 4);
#endif
	      result = 0;
	      done = True;
	    }
	  else if (*data == '3')	/* positive intermediate reply */
	    {
#ifdef DIAGNOSTIC
	      fprintf (stderr, "%s: internal error: "
		       "server wants more information?  (%s)\n",
		       progname, data);
#endif
	      result = 3;
	      done = True;
	    }
	  else if (*data == '4' ||	/* transient negative completion */
		   *data == '5')	/* permanent negative completion */
	    {
#ifdef DIAGNOSTIC
	      fprintf (stderr, "%s: %s\n", progname, data + 4);
#endif
	      result = (*data - '0');
	      done = True;
	    }
	  else
	    {
#ifdef DIAGNOSTIC
	      fprintf (stderr,
		       "%s: unrecognised " MOZILLA_RESPONSE_PROP
		       " from window 0x%x: %s\n",
		       progname, (unsigned int) window, data);
#endif
	      result = 6;
	      done = True;
	    }

	  if (data)
	    XFree (data);
	}
#ifdef DEBUG_PROPS
      else if (event.xany.type == PropertyNotify &&
	       event.xproperty.window == window &&
	       event.xproperty.state == PropertyDelete &&
	       event.xproperty.atom == XA_MOZILLA_COMMAND)
	{
	  fprintf (stderr, "%s: (server 0x%x has accepted "
		   MOZILLA_COMMAND_PROP ".)\n",
		   progname, (unsigned int) window);
	}
#endif /* DEBUG_PROPS */
    }

 DONE:

  return result;
}


int
mozilla_remote (char *command)
{
  int status = 0;
  Display *dpy;
  Window window;

  if (!(dpy = XOpenDisplay (NULL)))
    return (0);

  mozilla_remote_init_atoms (dpy);

  if (!(window = mozilla_remote_find_window (dpy)))
    return (0);

  XSelectInput (dpy, window, (PropertyChangeMask|StructureNotifyMask));

  if (mozilla_remote_obtain_lock (dpy, window))
    return (0);

  status = mozilla_remote_command (dpy, window, command);

  /* When status = 6, it means the window has been destroyed */
  /* It is invalid to free the lock when window is destroyed. */

  if ( status != 6 )
    mozilla_remote_free_lock (dpy, window);

  return (!status);
}
