#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <dos.h>
#include <dir.h>
#include <stdarg.h>
#include <alloc.h>
#include <stdlib.h>

#include <cxl\cxlwin.h>

#include "defines.h"
#include "lora.h"
#include "externs.h"
#include "prototyp.h"

static void raise_priv (void);
static void lower_priv (void);
static int set_priv (char);
static void big_char (char);
static void big_string (int, char *, ...);
static FILE *get_system_file (char *);

int read_system_file(name)
char *name;
{
        char filename[128];

        strcpy(filename, text_path);
        strcat(filename, name);

        return (read_file(filename));
}

static FILE *get_system_file (name)
char *name;
{
   FILE *fp;
   char linea[80];

   fp = fopen(name,"rb");
   if (fp == NULL && !usr.ansi && !usr.avatar)
   {
      sprintf(linea,"%s.ASC",name);
      fp = fopen(linea,"rb");
   }
   if (fp == NULL && usr.ansi)
   {
      sprintf(linea,"%s.ANS",name);
      fp = fopen(linea,"rb");
   }
   if (fp == NULL && usr.avatar)
   {
      sprintf(linea,"%s.AVT",name);
      fp = fopen(linea,"rb");
   }
   if (fp == NULL)
   {
      sprintf(linea,"%s.BBS",name);
      fp = fopen(linea,"rb");
   }

   return (fp);
}

int read_file(name)
char *name;
{
   FILE *fp, *answer, *fpc;
   char linea[258], stringa[80], parola[80], *p, lastresp[80];
   char chain[40], onexit[40], resp, c;
   int line, required, more, m, a, day, mont, year, bignum;
   word search;
   long tempo;
   struct ffblk blk;
   struct tm *tim;

   XON_ENABLE();
   _BRK_ENABLE();

   if (!name || !(*name))
      return(0);

   if ((p=strchr(name,'/')) != NULL)
      *(p-1) = '\0';

   fp = get_system_file (name);

   if (p != NULL)
      *(p-1) = ' ';

   if (fp == NULL)
      return (0);

   more = line = 1;
   nopause = bignum = required = 0;
   fpc = answer = NULL;
   resp = ' ';
   chain[0] = onexit[0] = '\0';

loop:
   change_attr(LGREY|_BLACK);

   while (fgets(linea, 255, fp) != NULL)
   {
      linea [256] = '\0';

      while (strstr (linea, "\r\n") == NULL && strlen (linea) < 256)
         if (fgets (&linea[strlen(linea)], 255-strlen(linea), fp) == NULL)
            break;

      for (p=linea;(*p) && (*p != 0x1A);p++)
      {
         if (!CARRIER || RECVD_BREAK())
         {
            CLEAR_OUTBOUND();

            fclose (fp);
            if (answer)
               fclose (answer);
            return (0);
         }

         switch (*p)
         {
         case CTRLA:
            big_string (bignum, bbstxt[B_PRESS_ENTER]);
            input (stringa, 0);
            line = 1;
            break;
         case CTRLD:
            _BRK_ENABLE ();
            more=1;
            break;
         case CTRLE:
            _BRK_DISABLE ();
            more=0;
            break;
         case CTRLF:
            p++;
            switch(toupper(*p))
            {
            case 'C':
               if (cps)
                  big_string (bignum, "%ld", cps);
               break;
            case 'D':
               big_string (bignum, "%s",usr.dataphone);
               break;
            case 'E':
               big_string (bignum, "%s",usr.voicephone);
               break;
            case 'F':
               usr.ldate[9] = '\0';
               big_string (bignum, "%s", usr.ldate);
               usr.ldate[9] = ' ';
               break;
            case 'G':
               big_string (bignum, "%s",&usr.ldate[11]);
               break;
            case 'H':
               if (cps)
                  big_string (bignum, "%ld", (cps * 100) / (rate / 10));
               break;
            case 'L':
               big_string (bignum, "%d", usr.credit);
               break;
            case 'M':
               big_string (bignum, "%d", last_mail);
               break;
            case 'N':
               big_string (bignum, "%d", lastread);
               break;
            case 'O':
               big_string (bignum, "%s", get_priv_text(usr.priv));
               break;
            case 'Q':
               big_string (bignum, "%u", usr.n_upld);
               break;
            case 'R':
               big_string (bignum, "%lu", usr.upld);
               break;
            case 'S':
               big_string (bignum, "%u", usr.n_dnld);
               break;
            case 'T':
               big_string (bignum, "%lu", usr.dnld);
               break;
            case 'V':
               big_string (bignum, "%d", usr.len);
               break;
            case 'X':
               big_string (bignum, "%s",usr.ansi ? bbstxt[B_YES] : bbstxt[B_NO]);
               break;
            case 'Y':
               big_string (bignum, "%s",usr.more ? bbstxt[B_YES] : bbstxt[B_NO]);
               break;
            case 'Z':
               big_string (bignum, "%s",usr.formfeed ? bbstxt[B_YES] : bbstxt[B_NO]);
               break;
            case '0':
               big_string (bignum, "%s",usr.use_lore ? bbstxt[B_NO] : bbstxt[B_YES]);
               break;
            case '2':
               big_string (bignum, "%s",usr.hotkey ? bbstxt[B_YES] : bbstxt[B_NO]);
               break;
            case '3':
               big_string (bignum, "%s",usr.handle);
               break;
            case '4':
               big_string (bignum, "%s",usr.firstdate);
               break;
            case '5':
               big_string (bignum, "%s",usr.birthdate);
               break;
            case '6':
               big_string (bignum, "%s",usr.subscrdate);
               break;
            case '8':
               big_string (bignum, "%s",usr.avatar ? bbstxt[B_YES] : bbstxt[B_NO]);
               break;
            case '!':
               big_string (bignum, "%s",usr.color ? bbstxt[B_YES] : bbstxt[B_NO]);
               break;
            case CTRLA:
               show_quote();
               break;
            case 'A':
            case CTRLB:
               big_string (bignum, "%s", usr.name);
               break;
            case 'B':
            case CTRLC:
               big_string (bignum, "%s", usr.city);
               break;
            case CTRLD:
               data(stringa);
               stringa[9] = '\0';
               big_string (bignum, "%s",stringa);
               break;
            case 'P':
            case CTRLE:
               big_string (bignum, "%ld",usr.times);
               break;
            case 'W':
            case CTRLF:
               strcpy(stringa,usr.name);
               get_fancy_string(stringa, parola);
               big_string (bignum, parola);
               break;
            case CTRLG:
               timer(10);
               break;
            case CTRLK:
               m = (int)((time(NULL)-start_time)/60);
               m += usr.time;
               big_string (bignum, "%d",m);
               break;
            case 'U':
            case CTRLL:
               m = (int)((time(0)-start_time)/60);
               big_string (bignum, "%d",m);
               break;
            case CTRLN:
               modem_hangup();
               return (0);
            case CTRLO:
               big_string (bignum, "%d",time_remain());
               break;
            case CTRLP:
               big_string (bignum, "%s",ctime(&start_time));
               break;
            case CTRLQ:
               big_string (bignum, "%lu",sysinfo.total_calls);
               break;
            case CTRLR:
               big_string (bignum, "%lu",usr.dnld-usr.upld);
               break;
            case CTRLT:
               data(stringa);
               big_string (bignum, &stringa[10]);
               break;
            case CTRLU:
               required=1;
               break;
            case CTRLV:
               required=0;
               break;
            case CTRLW:
               big_string (bignum, "%lu", usr.upld);
               break;
            case CTRLX:
               big_string (bignum, "%lu",usr.dnld);
               break;
            case '9':
               if(usr.n_upld == 0)
                  big_string (bignum, "0:%u", usr.n_dnld);
               else
               {
                   m = (unsigned int)(usr.n_dnld / usr.n_upld);
                   big_string (bignum, "1:%u", m);
               }
               break;
            case CTRLY:
            case ':':
               if (usr.upld == 0)
                  big_string (bignum, "0:%lu", usr.dnld);
               else {
                  m = (unsigned int)(usr.dnld / usr.upld);
                     big_string (bignum, "1:%u", m);
               }
               break;
            case ';':
               big_string (bignum, "%s",usr.full_read ? bbstxt[B_YES] : bbstxt[B_NO]);
               break;
            case '[':
               big_string (bignum, "%lu", class[usr_class].max_dl - usr.dnldl);
               break;
            case '\\':
               big_string (bignum, "%s", lang_descr[usr.language]);
               break;
            case ']':
               big_string (bignum, "%s", usr.comment);
               break;
            }
            break;
         case CR:
            break;
         case LF:
            if (!local_mode)
            {
               BUFFER_BYTE('\r');
               BUFFER_BYTE('\n');
            }
            if (snooping)
               wputs("\n");

            if (!(line++ < (usr.len-1)) && usr.len != 0)
            {
               if (!more)
                  continue;
               if (!(line = more_question (line)))
               {
                   fclose(fp);
                   fp = NULL;
               }
            }
            break;
         case CTRLK:
            p++;
            switch(toupper(*p)) {
            case 'A':
               big_string (bignum, "%lu",sysinfo.total_calls);
               break;
            case 'B':
               big_string (bignum, "%s",lastcall.name);
               break;
            case 'D':
               big_string (bignum, "%d",first_msg);
               break;
            case 'E':
               big_string (bignum, "%d",last_msg);
               break;
            case 'F':
               p++;
               sscanf(usr.ldate, "%2d %3s %2d", &day, parola, &year);
               parola[3] = '\0';
               for (mont = 0; mont < 12; mont++)
               {
                  if ((!stricmp(mtext[mont], parola)) || (!stricmp(mesi[mont], parola)))
                     break;
               }
               search=(year-80)*512+(mont+1)*32+day;
               translate_filenames (p, resp, lastresp);
               if (findfirst(p,&blk,0) || blk.ff_fdate < search)
                  p = strchr(linea,'\0') - 1;
               break;
            case 'G':
               tempo = time(0);
               tim = localtime(&tempo);
               big_string (bignum, "%s", bbstxt[B_SUNDAY + tim->tm_wday]);
               break;
            case 'I':
               data(stringa);
               big_string (bignum, &stringa[10]);
               break;
            case 'J':
               data(stringa);
               stringa[9] = '\0';
               big_string (bignum, "%s",stringa);
               break;
            case 'K':
               m = (int)((time(NULL)-start_time)/60);
               big_string (bignum, "%d",m);
               break;
            case 'M':
               big_string (bignum, "%d",max_priv_mail);
               break;
            case 'O':
               big_string (bignum, "%d",time_remain());
               break;
            case 'Q':
               big_string (bignum, "%d",class[usr_class].max_call);
               break;
            case 'R':
               big_string (bignum, "%d", local_mode ? 0 : rate);
               break;
            case 'T':
               big_string (bignum, "%d",class[usr_class].max_dl);
               break;
            case 'U':
               big_string (bignum, "%d", time_to_next (1));
               break;
            case 'W':
               big_string (bignum, "%d", line_offset);
               break;
            case 'X':
               terminating_call ();
               return (0);
            case 'Y':
               big_string (bignum, "%s",sys.msg_name);
               break;
            case 'Z':
               big_string (bignum, "%s",sys.file_name);
               break;
            case '0':
               big_string (bignum, "%d",num_msg);
               break;
            case '1':
               big_string (bignum, "%d",usr.msg);
               break;
            case '2':
               big_string (bignum, "%d",usr.files);
               break;
            case '7':
               big_string (bignum, "%d",usr.account);
               break;
            case '[':
               p++;
               change_attr (*p);
               break;
            case '\\':
               del_line ();
               break;
            }
            break;
         case CTRLL:
            cls();
            line=1;
            break;
         case CTRLO:
            p++;
            switch(toupper(*p)) {
            case 'C':
               p++;
               translate_filenames (p, resp, lastresp);
               open_outside_door (p);
               break;
            case 'D':
               p++;
               strcpy(chain,p);
               break;
            case 'E':
               if (!usr.ansi && !usr.avatar)
                  p = strchr(linea,'\0') - 1;
               break;
            case 'F':
               p++;
               strcpy(onexit,p);
               p = strchr(linea,'\0') - 1;
               break;
            case 'M':
               if(answer)
               {
                  p++;
                  fprintf(answer,"%s %c\r\n",p,resp);
               }

               p = strchr(linea,'\0') - 1;
               break;
            case 'N':
               while (CARRIER && time_remain() > 0)
               {
                  input(stringa,usr.width-1);
                  if(strlen(stringa))
                     break;
                  if(!required)
                     break;
                  big_string (bignum, bbstxt[B_TRY_AGAIN]);
               }

               strcpy (lastresp, stringa);

               if(answer)
               {
                  putc(' ',answer);
                  putc(' ',answer);
                  p++;
                  fprintf(answer,"%s",p);
                  putc(':',answer);
                  putc(' ',answer);
                  fprintf(answer,"%s\r\n",stringa);
               }

               p = strchr(linea,'\0') - 1;
               break;
            case 'O':
               if(answer)
                  fclose(answer);
               p++;
               p[strlen(p)-2] = '\0';
               translate_filenames (p, resp, lastresp);
               answer = fopen (p, "ab");
               if(answer == NULL)
                  status_line(msgtxt[M_UNABLE_TO_OPEN],p);

               p = strchr(linea,'\0') - 1;
               break;
            case 'P':
               if(answer)
               {
                  fprintf(answer,"\r\n");
                  usr.ptrquestion = ftell(answer);
                  fprintf(answer,"* %s\t%s\t%s\r\n",usr.name,usr.city,data(parola));
               }
               break;
            case 'Q':
               if (fp != NULL)
                  fclose(fp);

               UNBUFFER_BYTES ();
               FLUSH_OUTPUT ();

               if (fpc == NULL)
               {
                  fp = fopen(onexit,"rb");
                  if (fp == NULL)
                  {
                     if (answer)
                        fclose (answer);
                     return (line);
                  }
               }
               else
               {
                  fp = fpc;
                  fpc = NULL;
                  p[1] = '\0';
               }

               p = strchr(p,'\0') - 1;
               break;
            case 'R':
               line=1;
               p++;
               while (CARRIER && time_remain() > 0)
               {
                  input (stringa, 1);
                  c = toupper(stringa[0]);
                  if(c == '\0')
                     c = 0x7C;
                  if(strchr(strupr(p),c) != NULL)
                     break;
                  big_string (bignum, bbstxt[B_TRY_AGAIN]);
               }

               p = strchr(linea,'\0') - 1;
               resp = c;
               big_string (bignum, "\n");
               break;
            case 'S':
               p++;
               fclose(fp);
               fp = fopen(p,"rb");
               if(fp == NULL)
               {
                  fp = fopen(onexit,"rb");
                  onexit[0]='\0';
                  if (fp == NULL)
                     return(line);
               }

               p = strchr(linea,'\0') - 1;
               break;
            case 'T':
               rewind(fp);
               break;
            case 'U':
               p++;
               if(toupper(*p) != resp)
                  p = strchr(linea,'\0') - 1;
               break;
            case 'V':
               p++;
               fseek (fp, atol (p), SEEK_SET);
               if(toupper(*p) != resp)
                  p = strchr(linea,'\0') - 1;
               break;
            }
            break;
         case CTRLP:
            p++;
            if ( *p == 'B' )
            {
               p++;
               a = set_priv ( *p );
               if (usr.priv > a)
                  p = strchr(linea,'\0') - 1;
            }
            else if ( *p == 'L' )
            {
               p++;
               a = set_priv ( *p );
               if (usr.priv < a)
                  p = strchr(linea,'\0') - 1;
            }
            else if ( *p == 'Q' )
            {
               p++;
               a = set_priv ( *p );
               if (a != usr.priv)
                  p = strchr(linea,'\0') - 1;
            }
            else if ( *p == 'X' )
            {
               p++;
               a = set_priv ( *p );
               if (usr.priv == a)
                  p = strchr(linea,'\0') - 1;
            }
            else
            {
               a = set_priv ( *p );
               if (usr.priv < a)
                  fseek (fp, 0L, SEEK_END);
            }
            break;
         case CTRLV:
            p++;
            switch(*p) {
            case CTRLA:
               p++;
               if(*p == CTRLP)
               {
                  p++;
                  *p &= 0x7F;
               }

               if (!*p)
                  change_attr(13);
               else
                  change_attr(*p);
               break;
            case CTRLC:
               cup (1);
               line--;
               break;
            case CTRLD:
               cdo (1);
               line++;
               break;
            case CTRLE:
               cle (1);
               break;
            case CTRLF:
               cri (1);
               break;
            case CTRLG:
               del_line();
               break;
            case CTRLH:
               cpos ( *(p+1), *(p+2) );
               line = *(p+1);
               p+=2;
               break;
            }
            break;
         case CTRLW:
            p++;
            switch(*p) {
            case 'A':
               p++;
               p[strlen(p)-2] = '\0';
               translate_filenames (p, resp, lastresp);
               status_line (p);
               p = strchr(linea,'\0') - 1;
               break;
            case 'a':
               p++;
               p[strlen(p)-2] = '\0';
               translate_filenames (p, resp, lastresp);
               broadcast_message (p);
               p = strchr(linea,'\0') - 1;
               break;
            case CTRLA:
               usr.ldate[9] = '\0';
               big_string (bignum, "%s",usr.ldate);
               usr.ldate[9] = ' ';
               break;
            case 'B':
               bignum = bignum ? 0 : 1;
               break;
            case 'c':
               p++;
               if ( *p == 'A' && !local_mode)
                  p = strchr(linea,'\0') - 1;
               else if ( *p == 'R' && local_mode)
                  p = strchr(linea,'\0') - 1;
               break;
            case CTRLB:
               p++;
               if ( (*p == '1' && rate >= 1200) ||
                    (*p == '2' && rate >= 2400) ||
                    (*p == '9' && rate >= 9600)
                  )
                  break;
               p = strchr(linea,'\0') - 1;
               break;
            case CTRLC:
               big_string (bignum, system_name);
               break;
            case CTRLD:
               big_string (bignum, sysop);
               break;
            case CTRLE:
               big_string (bignum, lastresp);
               break;
            case CTRLG:
               sound (1000);
               timer (3);
               nosound ();
               break;
            case CR:
               p++;
               if ( *p == 'A')
                  big_string (bignum, "%d", usr.msg);
               if ( *p == 'L')
                  big_string (bignum, "%d",lastread);
               else if ( *p == 'N')
                  big_string (bignum, "%s",sys.msg_name);
               else if ( *p == 'H')
                  big_string (bignum, "%d",last_msg);
               else if ( *p == '#')
                  big_string (bignum, "%d",first_msg - last_msg + 1);
               break;
            case CTRLN:
               p++;
               if ( *p == 'B' || *p == 'C' )
                  big_string (bignum, "%d", usr.credit);
               else if ( *p == 'D' )
                  SENDBYTE ('0');
               break;
            case '8':
               if (usr.len < 79)
                  p = strchr(linea,'\0') - 1;
               break;
            case 'D':
               p++;
               p[strlen(p)-2] = '\0';
               translate_filenames (p, resp, lastresp);
               unlink (p);
               p = strchr(linea,'\0') - 1;
               break;
            case CTRLF:
            case 'G':
               p++;
               if ( *p == 'A')
                  big_string (bignum, "%d", usr.files);
               else if ( *p == 'N')
                  big_string (bignum, "%s",sys.file_name);
               break;
            case 'I':
               p++;
               if ( *p == 'L' && !local_mode)
                  p = strchr(linea,'\0') - 1;
               else if ( *p == 'R' && local_mode)
                  p = strchr(linea,'\0') - 1;
               break;
            case 'k':
               p++;
               if ( *p == 'F' )
               {
                  p++;
                  usr.flags &= ~get_flags (p);
                  while (*p != ' ' && *p != '\0')
                     p++;
               }
               else if ( *p == 'I' )
               {
                  p++;
                  if ((usr.flags & get_flags (p)) != get_flags (p))
                     p = strchr(linea,'\0') - 1;
                  else
                  {
                     while (*p != ' ' && *p != '\0')
                        p++;
                  }
               }
               else if ( *p == 'O' )
               {
                  p++;
                  usr.flags |= get_flags (p);
                  while (*p != ' ' && *p != '\0')
                     p++;
               }
               else if ( *p == 'T' )
               {
                  p++;
                  usr.flags ^= get_flags (p);
                  while (*p != ' ' && *p != '\0')
                     p++;
               }
               break;
            case 'L':
               p++;
               fpc = fp;
               p[strlen(p)-2] = '\0';
               fp = fopen (p, "rb");
               if (fp == NULL && usr.ansi)
               {
                  sprintf(stringa,"%s.ANS",p);
                  fp = fopen(stringa,"rb");
               }
               if (fp == NULL)
               {
                  sprintf(stringa,"%s.AVT",p);
                  fp = fopen(stringa,"rb");
               }
               if (fp == NULL)
               {
                  sprintf(stringa,"%s.BBS",p);
                  fp = fopen(stringa,"rb");
               }
               if (fp == NULL)
               {
                  sprintf(stringa,"%s.ASC",p);
                  fp = fopen(stringa,"rb");
               }
               if (fp == NULL)
               {
                  fp = fpc;
                  fpc = NULL;
               }
               p = strchr(p,'\0') - 1;
               break;
            case 'p':
               p++;
               if ( *p == 'D' )
                  lower_priv ();
               else if ( *p == 'U' )
                  raise_priv ();
               break;
            case 'P':
               big_string (bignum, "%s", usr.voicephone);
               break;
            case 'R':
               big_string (bignum, "%s", usr.handle);
               break;
            case 's':
               p++;
               usr.priv = set_priv ( *p );
               usr_class = get_class(usr.priv);
               break;
            case 'W':
               p++;
               if(answer)
               {
                  translate_filenames (p, resp, lastresp);
                  fprintf(answer, "%s", p);
               }
               p = strchr(p,'\0') - 1;
               break;
            case 'w':
               online_users (1);
               break;
            case 'X':
               p++;
               if ( *p == 'D' || *p == 'R' )
               {
                  p++;
                  translate_filenames (p, resp, lastresp);
                  open_outside_door (p);
                  p = strchr(p,'\0') - 1;
               }
               break;
            default:
               p--;
               timer(5);
               break;
            }
            break;
         case CTRLX:
            p++;
            translate_filenames (p, resp, lastresp);
            open_outside_door(p);
            *p = '\0';
            p--;
            break;
         case CTRLY:
            c = *(++p);
            a = *(++p);
            if(usr.avatar && !local_mode)
            {
               BUFFER_BYTE(CTRLY);
               BUFFER_BYTE(c);
               BUFFER_BYTE(a);
            }
            else if (!local_mode)
               for(m=0;m<a;m++)
                  BUFFER_BYTE(c);
            if (snooping)
               wdupc (c, a);
            break;
         case CTRLZ:
            break;
         default:
            if (!bignum)
            {
               if (!local_mode)
                  BUFFER_BYTE(*p);
               if (snooping)
                  wputc(*p);
            }
            else
               big_char (*p);
            break;
         }

         if (fp == NULL)
            break;
      }

      if (*p == CTRLZ || fp == NULL)
      {
         if (fp != NULL)
            fclose(fp);

         UNBUFFER_BYTES ();
         FLUSH_OUTPUT ();

         if (fpc == NULL)
         {
            fp = fopen(onexit,"rb");
            if (fp == NULL)
               break;
         }
         else
         {
            fp = fpc;
            fpc = NULL;
         }
      }
   }

   fclose(fp);

   if (fpc == NULL)
   {
      fp = fopen(onexit,"rb");
      if (fp != NULL)
         goto loop;
   }
   else
   {
      fp = fpc;
      fpc = NULL;
      goto loop;
   }

   if (fp != NULL)
      fclose(fp);
   if (answer != NULL)
      fclose(answer);

   UNBUFFER_BYTES ();
   FLUSH_OUTPUT();

   return(line);
}

void show_quote()
{
   FILE *quote;
   char linea[129];

   strcpy(linea, text_path);
   strcat(linea, "QUOTES.BBS");

   quote = fopen(linea,"rt");
   if(quote == NULL)
      return;
   fseek(quote,sysinfo.quote_position,0);

   for (;;)
   {
      if ((fgets(linea,128,quote) == NULL) || linea[0] == CTRLZ)
      {
         rewind(quote);
         continue;
      }

      if(linea[0] == '\n')
      {
         sysinfo.quote_position = ftell(quote);
         break;
      }

      m_print( "%s", linea);
   }

   fclose(quote);

   write_sysinfo();
}

char far macro_bitmap[95][5]=
{
  {  0x00,0x00,0x00,0x00,0x00  },   //
  {  0x00,0x00,0x3d,0x00,0x00  },   //  !
  {  0x00,0x30,0x00,0x30,0x00  },   //  "
  {  0x12,0x3f,0x12,0x3f,0x12  },   //  #
  {  0x09,0x15,0x3f,0x15,0x12  },   //  $
  {  0x19,0x1a,0x04,0x0b,0x13  },   //  %
  {  0x16,0x29,0x15,0x02,0x05  },   //  &
  {  0x00,0x08,0x30,0x00,0x00  },   //  '
  {  0x00,0x1e,0x21,0x00,0x00  },   //  (
  {  0x00,0x21,0x1e,0x00,0x00  },   //  )
  {  0x04,0x15,0x0e,0x15,0x04  },   //  *
  {  0x04,0x04,0x1f,0x04,0x04  },   //  +
  {  0x00,0x00,0x01,0x02,0x00  },   //  ,
  {  0x04,0x04,0x04,0x04,0x04  },   //  -
  {  0x00,0x00,0x03,0x03,0x00  },   //  .
  {  0x01,0x02,0x04,0x08,0x10  },   //  /
  {  0x1e,0x21,0x25,0x21,0x1e  },   //  0
  {  0x00,0x11,0x3f,0x01,0x00  },   //  1
  {  0x11,0x23,0x25,0x29,0x11  },   //  2
  {  0x21,0x29,0x29,0x29,0x16  },   //  3
  {  0x06,0x0a,0x12,0x3f,0x02  },   //  4
  {  0x3a,0x29,0x29,0x29,0x26  },   //  5
  {  0x1e,0x29,0x29,0x29,0x26  },   //  6
  {  0x21,0x22,0x24,0x28,0x30  },   //  7
  {  0x16,0x29,0x29,0x29,0x16  },   //  8
  {  0x10,0x29,0x29,0x29,0x1e  },   //  9
  {  0x00,0x00,0x12,0x00,0x00  },   //  :
  {  0x00,0x01,0x12,0x00,0x00  },   //  ;
  {  0x00,0x04,0x0a,0x11,0x00  },   //  <
  {  0x0a,0x0a,0x0a,0x0a,0x0a  },   //  =
  {  0x00,0x11,0x0a,0x04,0x00  },   //  >
  {  0x10,0x20,0x25,0x28,0x10  },   //  ?
  {  0x1e,0x29,0x35,0x29,0x15  },   //  @
  {  0x1f,0x24,0x24,0x24,0x1f  },   //  A
  {  0x3f,0x29,0x29,0x29,0x16  },   //  B
  {  0x1e,0x21,0x21,0x21,0x12  },   //  C
  {  0x3f,0x21,0x21,0x21,0x1e  },   //  D
  {  0x3f,0x29,0x29,0x29,0x21  },   //  E
  {  0x3f,0x28,0x28,0x28,0x20  },   //  F
  {  0x1e,0x21,0x25,0x25,0x16  },   //  G
  {  0x3f,0x08,0x08,0x08,0x3f  },   //  H
  {  0x00,0x21,0x3f,0x21,0x00  },   //  I
  {  0x02,0x01,0x01,0x01,0x3e  },   //  J
  {  0x3f,0x08,0x14,0x22,0x01  },   //  K
  {  0x3f,0x01,0x01,0x01,0x01  },   //  L
  {  0x3f,0x10,0x08,0x10,0x3f  },   //  M
  {  0x3f,0x10,0x08,0x04,0x3f  },   //  N
  {  0x1e,0x21,0x21,0x21,0x1e  },   //  O
  {  0x3f,0x24,0x24,0x24,0x18  },   //  P
  {  0x1e,0x21,0x25,0x23,0x1e  },   //  Q
  {  0x3f,0x24,0x24,0x26,0x19  },   //  R
  {  0x12,0x29,0x29,0x29,0x26  },   //  S
  {  0x20,0x20,0x3f,0x20,0x20  },   //  T
  {  0x3e,0x01,0x01,0x01,0x3e  },   //  U
  {  0x38,0x06,0x01,0x06,0x38  },   //  V
  {  0x3e,0x01,0x06,0x01,0x3e  },   //  W
  {  0x23,0x14,0x08,0x14,0x23  },   //  X
  {  0x38,0x04,0x03,0x04,0x38  },   //  Y
  {  0x23,0x25,0x29,0x31,0x21  },   //  Z
  {  0x00,0x3f,0x21,0x21,0x00  },   //  [
  {  0x10,0x08,0x04,0x02,0x01  },   //  \
  {  0x00,0x21,0x21,0x3f,0x00  },   //  ]
  {  0x00,0x10,0x20,0x10,0x00  },   //  ^
  {  0x01,0x01,0x01,0x01,0x01  },   //  _
  {  0x00,0x00,0x30,0x08,0x00  },   //  `
  {  0x02,0x15,0x15,0x15,0x0f  },   //  a
  {  0x3f,0x09,0x09,0x09,0x06  },   //  b
  {  0x0e,0x11,0x11,0x11,0x0a  },   //  c
  {  0x06,0x09,0x09,0x09,0x3f  },   //  d
  {  0x0e,0x15,0x15,0x15,0x09  },   //  e
  {  0x00,0x09,0x1f,0x29,0x00  },   //  f
  {  0x08,0x15,0x15,0x15,0x0e  },   //  g
  {  0x3f,0x08,0x08,0x08,0x07  },   //  h
  {  0x00,0x01,0x17,0x01,0x00  },   //  i
  {  0x02,0x01,0x01,0x01,0x16  },   //  j
  {  0x3f,0x04,0x0a,0x11,0x00  },   //  k
  {  0x00,0x21,0x3f,0x01,0x00  },   //  l
  {  0x1f,0x10,0x1f,0x10,0x0f  },   //  m
  {  0x1f,0x10,0x10,0x10,0x0f  },   //  n
  {  0x0e,0x11,0x11,0x11,0x0e  },   //  o
  {  0x1f,0x12,0x12,0x12,0x0c  },   //  p
  {  0x0c,0x12,0x12,0x12,0x1f  },   //  q
  {  0x0f,0x10,0x10,0x10,0x10  },   //  r
  {  0x09,0x15,0x15,0x15,0x12  },   //  s
  {  0x10,0x3e,0x11,0x11,0x00  },   //  t
  {  0x1e,0x01,0x01,0x01,0x1e  },   //  u
  {  0x1c,0x02,0x01,0x02,0x1c  },   //  v
  {  0x1e,0x01,0x0e,0x01,0x1e  },   //  w
  {  0x11,0x0a,0x04,0x0a,0x11  },   //  x
  {  0x19,0x05,0x05,0x05,0x1e  },   //  y
  {  0x11,0x13,0x15,0x19,0x11  },   //  z
  {  0x08,0x16,0x21,0x21,0x00  },   //  {
  {  0x00,0x00,0x37,0x00,0x00  },   //  |
  {  0x00,0x21,0x21,0x16,0x08  },   //  }
  {  0x00,0x10,0x20,0x10,0x20  }    //  ~
};

static void big_char (ch)
char ch;
{
  char chars_bitmap[]=" ���";
  char bitbuf[3][5];
  char far *q;

  q = (char far *)&macro_bitmap[ch-' '][0];

  for (ch = 0; ch < 5; ch++)
  {
    bitbuf[0][ch] = chars_bitmap[q[ch] >> 4];
    bitbuf[1][ch] = chars_bitmap[(q[ch] >> 2) & 3];
    bitbuf[2][ch] = chars_bitmap[q[ch] & 3];
  }

  cup (2);
  m_print ("%5.5s ", bitbuf[0]);
  cle (6);
  cdo (1);
  m_print ("%5.5s ", bitbuf[1]);
  cle (6);
  cdo (1);
  m_print ("%5.5s ", bitbuf[2]);
}

static void big_string (int big, char *format, ...)
{
   char *s;
   va_list var_args;
   char *string;

   string=(char *)malloc(256);

   if (string==NULL || strlen(format) > 256)
   {
      if (string)
         free(string);
      return;
   }

   va_start(var_args,format);
   vsprintf(string,format,var_args);
   va_end(var_args);

   if (big)
      for (s = string; *s; s++)
         big_char (*s);
   else
      m_print (string);

   free(string);
}

static void lower_priv ()
{
   switch (usr.priv)
   {
      case DISGRACE:
         usr.priv = TWIT;
         break;
      case LIMITED:
         usr.priv = DISGRACE;
         break;
      case NORMAL:
         usr.priv = LIMITED;
         break;
      case WORTHY:
         usr.priv = NORMAL;
         break;
      case PRIVIL:
         usr.priv = WORTHY;
         break;
      case FAVORED:
         usr.priv = PRIVIL;
         break;
      case EXTRA:
         usr.priv = FAVORED;
         break;
      case CLERK:
         usr.priv = EXTRA;
         break;
      case ASSTSYSOP:
         usr.priv = CLERK;
         break;
      case SYSOP:
         usr.priv = ASSTSYSOP;
         break;
   }

   usr_class = get_class(usr.priv);
}

static void raise_priv ()
{
   switch (usr.priv)
   {
      case TWIT:
         usr.priv = DISGRACE;
         break;
      case DISGRACE:
         usr.priv = LIMITED;
         break;
      case LIMITED:
         usr.priv = NORMAL;
         break;
      case NORMAL:
         usr.priv = WORTHY;
         break;
      case WORTHY:
         usr.priv = PRIVIL;
         break;
      case PRIVIL:
         usr.priv = FAVORED;
         break;
      case FAVORED:
         usr.priv = EXTRA;
         break;
      case EXTRA:
         usr.priv = CLERK;
         break;
      case CLERK:
         usr.priv = ASSTSYSOP;
         break;
      case ASSTSYSOP:
         usr.priv = SYSOP;
         break;
   }

   usr_class = get_class(usr.priv);
}

static int set_priv (c)
char c;
{
   register int i;

   i = usr.priv;

   switch (c)
   {
      case 'T':
         i = TWIT;
         break;
      case 'D':
         i = DISGRACE;
         break;
      case 'L':
         i = LIMITED;
         break;
      case 'N':
         i = NORMAL;
         break;
      case 'W':
         i = WORTHY;
         break;
      case 'P':
         i = PRIVIL;
         break;
      case 'F':
         i = FAVORED;
         break;
      case 'E':
         i = EXTRA;
         break;
      case 'C':
         i = CLERK;
         break;
      case 'A':
         i = ASSTSYSOP;
         break;
      case 'S':
         i = SYSOP;
         break;
   }

   return (i);
}

