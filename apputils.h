#ifndef APPUTILS_H_INCLUDED
#define APPUTILS_H_INCLUDED

#include <stdlib.h>
#include <gtk/gtk.h>

void gtk_button_set_bold(GtkButton *button);

const char pathdelim = '/';

const char* gethomedir();
char* expandpath(const char* filename);
char* extractdir(const char* filename);

bool fileexists(const char* filename);
bool canaccess(const char* filename, int mode);
void filecopy(const char* src, const char* dest);

const char* param_name(const char* str, size_t &namelen);
const char* param_value(const char* str, size_t &valuelen);

gchar** strtidy(gchar** items, const gchar* pattern);

#endif // APPUTILS_H_INCLUDED
