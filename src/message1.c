#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <dir.h>
#include <io.h>
#include <fcntl.h>
#include <ctype.h>

#include <cxl\cxlstr.h>

#include "defines.h"
#include "lora.h"
#include "externs.h"
#include "prototyp.h"

static char *firstchar(char *, char *, int);

void gather_origin_netnode (s)
char *s;
{
   int zone, net, node, point;
   char *p;

   p = strchr(s, '\0');

   while (p != s && !isdigit (*p))
      p--;

   while (p > s)
   {
      if (!isdigit(*p) && *p != '.' && *p != ':' && *p != '/')
         break;
      p--;
   }

   if (p == s)
      return;

   p++;
   parse_netnode (p, &zone, &net, &node, &point);

   msg_fzone = zone;
   msg.orig_net = net;
   msg.orig = node;
   msg_fpoint = point;
}

void parse_netnode(netnode, zone, net, node, point)
char *netnode;
int *zone, *net, *node, *point;
{
   char *p;

   *zone = alias[0].zone;
   *net = alias[0].net;
   *node = 0;
   *point = 0;

   p = netnode;

   /* If we have a zone (and the caller wants the zone to be passed back).. */

   if (strchr(netnode,':') && zone) {
      *zone=atoi(p);
      p = firstchar(p,":",2);
   }

   /* If we have a net number... */

   if (p && strchr(netnode,'/') && net) {
      *net=atoi(p);
      p=firstchar(p,"/",2);
   }

   /* We *always* need a node number... */

   if (p && node)
      *node=atoi(p);

   /* And finally check for a point number... */

   if (p && strchr(netnode,'.') && point) {
      p=firstchar(p,".",2);

      if (p)
         *point=atoi(p);
      else
         *point=0;
   }
}


char *firstchar(strng, delim, findword)
char *strng, *delim;
int findword;
{
   int x, isw, sl_d, sl_s, wordno=0;
   char *string, *oldstring;

   /* We can't do *anything* if the string is blank... */

   if (! *strng)
      return NULL;

   string=oldstring=strng;

   sl_d=strlen(delim);

   for (string=strng;*string;string++)
   {
      for (x=0,isw=0;x <= sl_d;x++)
         if (*string==delim[x])
            isw=1;

      if (isw==0) {
         oldstring=string;
         break;
      }
   }

   sl_s=strlen(string);

   for (wordno=0;(string-oldstring) < sl_s;string++)
   {
      for (x=0,isw=0;x <= sl_d;x++)
         if (*string==delim[x])
         {
            isw=1;
            break;
         }

      if (!isw && string==oldstring)
         wordno++;

      if (isw && (string != oldstring))
      {
         for (x=0,isw=0;x <= sl_d;x++) if (*(string+1)==delim[x])
         {
            isw=1;
            break;
         }

         if (isw==0)
            wordno++;
      }

      if (wordno==findword)
         return((string==oldstring || string==oldstring+sl_s) ? string : string+1);
   }

   return NULL;
}


