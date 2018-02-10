#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <pwd.h>
#include "apputils.h"

void gtk_button_set_bold(GtkButton *button)
{
  GtkWidget *label = gtk_bin_get_child(GTK_BIN(button));
//  PangoAttrList *attrs = pango_attr_list_new();
//  pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
//  gtk_label_set_attributes(GTK_LABEL(label), attrs);
//  pango_attr_list_unref(attrs);

  gchar *markup = g_strconcat("<b>", gtk_label_get_text(GTK_LABEL(label)), "</b>", NULL);
  gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);
}

const char* gethomedir()
{
  const char* home = getenv("HOME");
  if (!home)
  {
    struct passwd *pw = getpwuid(getuid());
    home = pw->pw_dir;
  }
  return home;
}

char* expandpath(const char* filename)
{
  if (filename && *filename == '~')
  {
    const char* home = gethomedir();
    if (home)
    {
      // exclude trailing path delimiter
      size_t homelen = strlen(home);
      if (home[homelen - 1] == pathdelim) --homelen;
      // exclude heading path delimiter
      if (*(++filename) == pathdelim) ++filename;
      // concat homedir + pathdelim + filename
      char buf[PATH_MAX];
      strncpy(&buf[0], home, homelen);
      strncpy(&buf[homelen], &pathdelim, 1);
      strcpy(&buf[homelen + 1], filename);
      filename = buf;
    }
  }
    // return dup of the path
    //return realpath(filename, NULL);
  return strdup(filename);
}

char* extractdir(const char* filename)
{
  const char* pos = strrchr(filename, pathdelim);
  return *pos ? strndup(filename, pos - filename) : NULL;
}

bool fileexists(const char* filename)
{
  return ( access(filename, F_OK) == 0 );
}

bool canaccess(const char* filename, int mode)
{
  return ( access(filename, mode) == 0 );
}

void filecopy(const char* src, const char* dest)
{
  int sf = open(src, O_RDONLY, 0);

  // struct required, rationale: function stat() exists also
  struct stat stat_source;
  fstat(sf, &stat_source);

  int df = open(dest, O_WRONLY | O_CREAT | O_TRUNC/**/, /*0644*/stat_source.st_mode);
  sendfile(df, sf, 0, stat_source.st_size);

  close(df);
  close(sf);
}

const char* param_name(const char* str, size_t &namelen)
{
  str += strspn(str, " \t\r\n");
  namelen = strcspn(str, " \t=#\r\n");
  return namelen ? str : NULL;
}

const char* param_value(const char* str, size_t &valuelen)
{
  valuelen = 0;
  str += strcspn(str, "=#");
  if (*str++ != '=')
    return NULL;

  str += strspn(str, " \t\r\n");
  valuelen = (*str == '"') ? strcspn(++str, "\"\r\n") : strcspn(str, "#\r\n");
  return str;
}

gchar** strtidy(gchar** items, const gchar* pattern)
{
  for (guint index = 0, skip = 0; items[index]; index++)
    if (!g_strcmp0(items[index], pattern))
    {
      skip++;
      g_free(items[index]);
      items[index] = NULL;
    }
    else if (skip)
    {
      items[index - skip] = items[index];
      items[index] = NULL;
    }
  return items;
}
