#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <gio/gsettings.h>
#include <glib/gi18n.h>

#include "main.h"
#include "mainwnd.h"
#include "authdlg.h"
#include "apputils.h"

#define GSETTINGS_PARAM_COUNT 4
const struct { const gchar * name; ProxyParam param; } gs_params[GSETTINGS_PARAM_COUNT] = {
  { GSETTINGS_SCHEMA_HTTP, PARAM_HTTP },
  { GSETTINGS_SCHEMA_HTTPS, PARAM_HTTPS },
  { GSETTINGS_SCHEMA_FTP, PARAM_FTP},
  { GSETTINGS_SCHEMA_SOCKS, PARAM_SOCKS }
};

#define ENVIRONMENT_PARAM_COUNT 5
const struct { const gchar *name1, *name2; ProxyParam param; } env_params[ENVIRONMENT_PARAM_COUNT] = {
  { "http_proxy", "HTTP_PROXY", PARAM_HTTP },
  { "https_proxy", "HTTPS_PROXY", PARAM_HTTPS },
  { "ftp_proxy", "FTP_PROXY", PARAM_FTP },
  { "socks_proxy", "SOCKS_PROXY", PARAM_SOCKS },
  { "no_proxy", "NO_PROXY", PARAM_NOPROXY }
};

void load_auth_settings(AuthDialog *dialog)
{
  GSettings *settings = g_settings_new(GSETTINGS_SCHEMA_HTTP);
  // use authentication
  dialog->set_use_auth(
    g_settings_get_boolean(settings, GSETTINGS_KEY_USE_AUTH));
  // username & password
  dialog->set_auth(
    g_settings_get_string(settings, GSETTINGS_KEY_AUTH_USER),
    g_settings_get_string(settings, GSETTINGS_KEY_AUTH_PASS));
}

void save_auth_settings(AuthDialog *dialog)
{
  GSettings *settings = g_settings_new(GSETTINGS_SCHEMA_HTTP);
  // use authentication
  g_settings_set_boolean(settings, GSETTINGS_KEY_USE_AUTH,
    dialog->get_use_auth());
  // username & password
  const gchar *username, *password;
  dialog->get_auth(username, password);
  g_settings_set_string(settings, GSETTINGS_KEY_AUTH_USER, username);
  g_settings_set_string(settings, GSETTINGS_KEY_AUTH_PASS, password);
}

void load_gsettings(MainWindow *window)
{
  gchar* str;
  GSettings *settings;

  settings = g_settings_new(GSETTINGS_SCHEMA_PROXY);

  // mode
  str = g_settings_get_string(settings, GSETTINGS_KEY_MODE);
  if (!strcmp(str, GSETTINGS_MODE_AUTO))
    window->set_gs_mode(MODE_AUTO);
  else if (!strcmp(str, GSETTINGS_MODE_MANUAL))
    window->set_gs_mode(MODE_MANUAL);
  else if (!strcmp(str, GSETTINGS_MODE_NONE))
    window->set_gs_mode(MODE_NONE);
  else
    window->set_gs_mode(MODE_NOTSET);
  g_free(str);

  // same proxy
  window->set_gs_single_proxy(
    g_settings_get_boolean(settings, GSETTINGS_KEY_USE_SAME_PROXY));

  // autoconfig url
  str = g_settings_get_string(settings, GSETTINGS_KEY_AUTOCONFIG_URL);
  window->set_gs_autoconfig_url(str);
  g_free(str);

  // ignoring hosts
  gchar** noproxy_lines = g_settings_get_strv(settings, GSETTINGS_KEY_IGNORE_HOSTS);
  window->set_gs_noproxy(noproxy_lines);
  g_free(noproxy_lines);

  // cleanup
  g_clear_object(&settings);

  // per proto proxy addresses
  for (int i = 0; i < GSETTINGS_PARAM_COUNT; i++)
  {
    settings = g_settings_new(gs_params[i].name);

    str = g_settings_get_string(settings, GSETTINGS_KEY_HOST);
    int port = g_settings_get_int(settings, GSETTINGS_KEY_PORT);
    window->set_gs_param(gs_params[i].param, str, port);
    g_free(str);

    // cleanup
    g_clear_object(&settings);
  }
}

void save_gsettings(MainWindow *window)
{
  GSettings *settings = g_settings_new(GSETTINGS_SCHEMA_PROXY);

  // mode
  ProxyMode mode = window->get_mode();
  if (mode == MODE_AUTO)
    g_settings_set_string(settings, GSETTINGS_KEY_MODE, GSETTINGS_MODE_AUTO);
  else if (mode == MODE_MANUAL)
    g_settings_set_string(settings, GSETTINGS_KEY_MODE, GSETTINGS_MODE_MANUAL);
  else
    g_settings_set_string(settings, GSETTINGS_KEY_MODE, GSETTINGS_MODE_NONE);

  // same proxy
  g_settings_set_boolean(settings, GSETTINGS_KEY_USE_SAME_PROXY,
    window->get_gs_single_proxy());

  // autoconfig url
  g_settings_set_string(settings, GSETTINGS_KEY_AUTOCONFIG_URL,
    window->get_gs_autoconfig_url());

  // ignoring hosts
  gchar** noproxy_lines = window->get_gs_noproxy();
  g_settings_set_strv(settings, GSETTINGS_KEY_IGNORE_HOSTS, noproxy_lines);
  g_strfreev(noproxy_lines);

  // cleanup
  g_clear_object(&settings);

  // per proto proxy addresses
  for (int i = 0; i < GSETTINGS_PARAM_COUNT; i++)
  {
    settings = g_settings_new(gs_params[i].name);

    int port;
    const gchar* host = window->get_gs_param(gs_params[i].param, &port);
    g_settings_set_string(settings, GSETTINGS_KEY_HOST, host);
    g_settings_set_int(settings, GSETTINGS_KEY_PORT, port);

    // cleanup
    g_clear_object(&settings);
  }
}

void load_env_settings(MainWindow *window)
{
  /* clear editables */

  for (int i = 0; i < ENVIRONMENT_PARAM_COUNT; i++)
    window->set_env_param(env_params[i].param, NULL);

  gchar* sourcename = window->get_env_source_name();
  gchar* filename = expandpath(sourcename);
  g_free(sourcename);

  /* check write permission */

 bool cansave = false;
  if (canaccess(filename, R_OK | W_OK))
    cansave = true;
  else if (!fileexists(filename))
  {
    gchar* filedir = extractdir(filename);
    if (canaccess(filedir, W_OK))
      cansave = true;
    g_free(filedir);
  }
  window->set_env_editable(cansave);

  /* parse the file */

  GIOChannel* infile = g_io_channel_new_file(filename, "r", NULL);
  if (infile)
  {
    GIOStatus status;
    GString* sline = g_string_new(NULL);
    while (infile)
    {
      status = g_io_channel_read_line_string(infile, sline, NULL, NULL);
      if (status == G_IO_STATUS_AGAIN)
        continue;
      if (status == G_IO_STATUS_EOF)
        break;
      if (status == G_IO_STATUS_ERROR)
        break;

      // try to extract param name
      size_t namelen;
      const gchar* name = param_name(sline->str, namelen);
      if (!namelen)
        continue;

      // find proxy params and display values
      for (int i = 0; i < ENVIRONMENT_PARAM_COUNT; i++)
        if (!strncmp(env_params[i].name1, name, namelen)
          || !strncmp(env_params[i].name2, name, namelen))
        {
          size_t valuelen;
          const gchar* val = param_value(sline->str, valuelen);
          gchar* value = g_strndup(val, valuelen);
          window->set_env_param(env_params[i].param, value);
          g_free(value);
          break;
        }
    }//while
    g_string_free(sline, TRUE);
    // close infile
    g_io_channel_shutdown(infile, FALSE, NULL);
    g_clear_pointer(&infile, g_io_channel_unref);
  }//if
  g_free(filename);
}

void save_env_settings(MainWindow *window)
{
  gchar* sourcename = window->get_env_source_name();
  gchar* filename = expandpath(sourcename);
  g_free(sourcename);

  /* check write permission */

  bool cansave = false;
  if (canaccess(filename, R_OK | W_OK))
    cansave = true;
  else if (!fileexists(filename))
  {
    gchar* filedir = extractdir(filename);
    if (canaccess(filedir, W_OK))
      cansave = true;
    g_free(filedir);
  }
  if (!cansave)
  {
    g_free(filename);
    return;
  }

  /* open files */

  bool success = false;
  gchar *tempname = g_strconcat(filename, ".tmp", NULL);
  GIOChannel* infile = g_io_channel_new_file(filename, "r", NULL);
  GIOChannel* outfile = g_io_channel_new_file(tempname, "w", NULL);
  if (outfile)
  {
    GIOStatus status;
    GString* sline = g_string_new(NULL);

    /* copy ahead lines */

    while (infile)
    {
      status = g_io_channel_read_line_string(infile, sline, NULL, NULL);
      if (status == G_IO_STATUS_AGAIN)
        continue;
      if (status == G_IO_STATUS_ERROR)
        break;

      /* find begin of block or eof */

      if (status == G_IO_STATUS_EOF)
        break;

      status = g_io_channel_write_chars(outfile, sline->str, sline->len, NULL, NULL);

      if (!strncmp(sline->str, BLOCK_BEGIN, sizeof(BLOCK_BEGIN)))
        break;
    }

    /* write block header */

    if (!infile || status == G_IO_STATUS_EOF)
    {
      status = g_io_channel_write_chars(outfile, "\n", 1, NULL, NULL);
      status = g_io_channel_write_chars(outfile, BLOCK_BEGIN, strlen(BLOCK_BEGIN), NULL, NULL);
      status = g_io_channel_write_chars(outfile, BLOCK_COMMENT, strlen(BLOCK_COMMENT), NULL, NULL);
    }

    /* copy lines without proxy params */

    while (infile)
    {
      status = g_io_channel_read_line_string(infile, sline, NULL, NULL);
      if (status == G_IO_STATUS_AGAIN)
        continue;
      if (status == G_IO_STATUS_ERROR)
        break;

      /* find end of block or eof */

      if (status == G_IO_STATUS_EOF)
        break;
      if (!strncmp(sline->str, BLOCK_END, sizeof(BLOCK_END)))
        break;

      /* parse line */
      bool skip = false;
      size_t namelen;
      const gchar* name = param_name(sline->str, namelen);
      if (name)
      {
        for (int i = 0; i < ENVIRONMENT_PARAM_COUNT; i++)
          if (!strncmp(env_params[i].name1, name, namelen)
            || !strncmp(env_params[i].name2, name, namelen))
          {
            skip = true;
            break;
          }
      }
      if (!skip)
      {
        status = g_io_channel_write_chars(outfile, sline->str, sline->len, NULL, NULL);
      }
    }

    /* write proxy params */

    for (int i = 0; i < ENVIRONMENT_PARAM_COUNT; i++)
    {
      const gchar* value = window->get_env_param(env_params[i].param);

      // do not to set param if value (pointer) is NULL
      if (!value)
        continue;

      gchar* line;
      line = g_strconcat(env_params[i].name1, "=", value, "\n", NULL);
      status = g_io_channel_write_chars(outfile, line, strlen(line), NULL, NULL);
      g_free(line);
      line = g_strconcat(env_params[i].name2, "=", value, "\n", NULL);
      status = g_io_channel_write_chars(outfile, line, strlen(line), NULL, NULL);
      g_free(line);
    }

    /* write block trailer */

    status = g_io_channel_write_chars(outfile, BLOCK_END, strlen(BLOCK_END), NULL, NULL);

    /* copy behind lines */

    while (infile)
    {
      status = g_io_channel_read_line_string(infile, sline, NULL, NULL);
      if (status == G_IO_STATUS_AGAIN)
        continue;
      if (status == G_IO_STATUS_ERROR)
        break;
      if (status == G_IO_STATUS_EOF)
        break;

      status = g_io_channel_write_chars(outfile, sline->str, sline->len, NULL, NULL);
    }

    success = true;

    g_string_free(sline, TRUE);
    // close outfile
    g_io_channel_shutdown(outfile, TRUE, NULL);
    g_clear_pointer(&outfile, g_io_channel_unref);
  }//if

  if (infile)
  {
    // close infile
    g_io_channel_shutdown(infile, FALSE, NULL);
    g_clear_pointer(&infile, g_io_channel_unref);
  }

  if (success)
  {
    // exchange files
    gchar *bakname = g_strconcat(filename, "~", NULL);
    filecopy(filename, bakname);
    rename(tempname, filename);
    g_free(bakname);
  }

  g_free(tempname);
  g_free(filename);
}

static void on_reset_clicked(GtkWidget *sender, MainWindow *window)
{
  switch (window->get_tab_index())
  {
    case TAB_GSETTINGS: load_gsettings(window); break;
    case TAB_ENVIRONMENT: load_env_settings(window); break;
  }
}

static void on_apply_clicked(GtkWidget *sender, MainWindow *window)
{
  switch (window->get_tab_index())
  {
    case TAB_GSETTINGS: save_gsettings(window); break;
    case TAB_ENVIRONMENT: save_env_settings(window); break;
  }
}

static void on_proxy_auth_clicked(GtkWidget *sender, MainWindow *window)
{
  AuthDialog *dlg = new AuthDialog(GTK_WINDOW(window->win));
  switch (dlg->show_modal())
  {
    case GTK_RESPONSE_OK:
      save_auth_settings(dlg);
    break;
  }
  delete dlg;
}

static void on_envsource_changed(GtkWidget *sender, MainWindow *window)
{
  load_env_settings(window);
}

int main (int argc, char *argv[])
{
  /* Initialize GTK+ */
  g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc)gtk_false, NULL);
  gtk_init(&argc, &argv);
  g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

  textdomain(g_get_prgname());

  MainWindow *wnd = new MainWindow();

  // assign event handlers
  g_signal_connect(wnd->auth_button, "clicked", G_CALLBACK(on_proxy_auth_clicked), wnd);
  g_signal_connect(wnd->reset_button, "clicked", G_CALLBACK(on_reset_clicked), wnd);
  g_signal_connect(wnd->apply_button, "clicked", G_CALLBACK(on_apply_clicked), wnd);
  g_signal_connect(wnd->cancel_button, "clicked", G_CALLBACK(gtk_main_quit), wnd);
  g_signal_connect(wnd->env_source, "changed", G_CALLBACK(on_envsource_changed), wnd);

  // initialize env_source
  wnd->add_env_source_name("~/.bashrc");
  wnd->add_env_source_name("~/.bash_profile");
  wnd->add_env_source_name("/etc/environment");
  wnd->add_env_source_name("/etc/bash.bashrc");
  wnd->add_env_source_name("/etc/profile");
  wnd->set_env_source_index(0);

  load_gsettings(wnd);

  wnd->show();

  /* Enter the main loop */

  gtk_main();
  delete wnd;

  return 0;
}

