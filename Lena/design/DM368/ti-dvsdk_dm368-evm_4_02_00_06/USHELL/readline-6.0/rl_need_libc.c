//#include <kth.h>
#include <sys/types.h>
#include <termios.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>
#include <stdio.h>

#if 0
int tcgetattr(int __fd, struct termios *__s)
{
    return kthread_tty_getattr(__fd, __s);
}

int tcsetattr(int __fd, int __opt, const struct termios *__s)
{
    return kthread_tty_setattr(__fd, __opt, __s);
}

extern int kth_tty_tcflow(int __fd, int __action);

int tcflow(int __fd, int __action)
{
	return kth_tty_tcflow(__fd, __action);
}
#endif
int tputs(const char *str, int affcnt, int (*putc_func)(char))
{
    if ((!str) || (!putc_func))
    	return -1;
    
    while(*str != '\0')
    {
        (*putc_func)(*str);
        str++;
    }
    return 0;
}

char *tgoto(const char *cap, int col, int row)
{
    return NULL;
}

int tgetnum(char *id)
{
    return 0;
}

char *tgetstr(char *id, char **area)
{
    return NULL;
}

 int tgetent(char *bp, const char *name)
 {
     return 0;
 }

 int tgetflag(char *id)
 {
     return 0;
 }

#if 0
/*以下为complete.c tilde.c添加*/
#include "pwd.h"
struct passwd	*getpwent (void)
{
    return (struct passwd	*)NULL;
}
void		 setpwent (void)
{
}
void		 endpwent (void)
{
}

struct passwd	*getpwnam (const char *wname)
{
    return (struct passwd	*)NULL;
}

struct passwd	*getpwuid (uid_t uid)
{
    return (struct passwd	*)NULL;
}

uid_t getuid()
{
    return 0;
}



char *
sh_get_env_value (const char *varname)
/*     const char *varname;*/
{
  return NULL;
}
extern struct passwd *getpwuid (uid_t);
extern uid_t getuid(void);
#include <pwd.h>
char *
sh_get_home_dir (void)
{
  char *home_dir;
  struct passwd *entry;

  home_dir = (char *)NULL;
  entry = getpwuid (getuid ());
  if (entry)
    home_dir = entry->pw_dir;
  return (home_dir);
}

#endif

