/* LinPopUp - A Linux enhanced port of Winpopup, running over Samba.
 * Copyright (c)1998-2000 Jean-Marc Jacquet <jm@littleigloo.org>  
 *           (c)2003-2005 Grigory Batalov <bga@altlinux.org>
 * Little Igloo Org  http://www.LittleIgloo.org
 *
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <stdio.h>
#include <string.h>
#include <ctype.h>


char *
strexpand (char *string, char *search_string, char *replace_string, size_t bufsize)
{
  int string_index = 0, sub_index;
  char sub_string[100], new_string[2048]="";

  size_t len = strlen (string);
  if (len >= bufsize)
      len = bufsize - 1;

  do
    {
      sub_index = 0;
      sub_string[0] = 0;
      while ((string[string_index]) && (string[string_index] != 32) &&
	(sub_index < sizeof(sub_string) - 1) && (string_index < len))
	    sub_string[sub_index++] = string[string_index++];
      sub_string[sub_index] = 0;
      if (!strncmp (sub_string, search_string, sizeof(sub_string)))
	 g_strlcpy (sub_string, replace_string, sizeof(sub_string));
      g_strlcat (new_string, sub_string, sizeof(new_string));
      g_strlcat (new_string, " ", sizeof(new_string));
    }
  while (++string_index < len);

  g_strlcpy (string, new_string, bufsize);
  return string;
}
