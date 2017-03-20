#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gio/gsettings.h>
#include <glib/gi18n.h>
#include "main.h"
#include "mainwnd.h"
#include "authdlg.h"

void read_auth_settings(AuthDialog *dialog)
{
  GSettings *settings = g_settings_new(GSETTINGS_SCHEMA_HTTP);
  // use authentication
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->use_auth_checkbox),
    g_settings_get_boolean(settings, GSETTINGS_KEY_USE_AUTH)
  );
  // username
  gtk_entry_set_text(GTK_ENTRY(dialog->username_entry),
    g_settings_get_string(settings, GSETTINGS_KEY_AUTH_USER)
  );
  // password
  gtk_entry_set_text(GTK_ENTRY(dialog->password_entry),
    g_settings_get_string(settings, GSETTINGS_KEY_AUTH_PASS)
  );
}

void write_auth_settings(AuthDialog *dialog)
{
  GSettings *settings = g_settings_new(GSETTINGS_SCHEMA_HTTP);
  // use authentication
  g_settings_set_boolean(settings, GSETTINGS_KEY_USE_AUTH,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->use_auth_checkbox))
  );
  // username
  g_settings_set_string(settings, GSETTINGS_KEY_AUTH_USER,
    gtk_entry_get_text(GTK_ENTRY(dialog->username_entry))
  );
  // password
  g_settings_set_string(settings, GSETTINGS_KEY_AUTH_PASS,
    gtk_entry_get_text(GTK_ENTRY(dialog->password_entry))
  );
}

void read_settings(MainWindow *window)
{
  GSettings *settings = g_settings_new(GSETTINGS_SCHEMA_PROXY);

  // same proxy
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(window->use_same_proxy),
    g_settings_get_boolean(settings, GSETTINGS_KEY_USE_SAME_PROXY)
  );

  // autoconfig url
  gtk_entry_set_text(GTK_ENTRY(window->autoconfig_url),
    g_settings_get_string(settings, GSETTINGS_KEY_AUTOCONFIG_URL)
  );

  // ignoring hosts
  GtkTextBuffer *buff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(window->ignore_hosts));
  gtk_text_buffer_set_text(buff, "", -1);
  gchar **ignore_hosts = g_settings_get_strv(settings, GSETTINGS_KEY_IGNORE_HOSTS);
  for (int i = 0; ignore_hosts[i] != NULL; i++)
  {
    gtk_text_buffer_insert_at_cursor(buff, ignore_hosts[i], -1);
    gtk_text_buffer_insert_at_cursor(buff, "\r", -1);
  }

  // mode
  const gchar *mode = g_settings_get_string(settings, GSETTINGS_KEY_MODE);
  GtkWidget *radio =
    !strcmp(mode, GSETTINGS_MODE_AUTO) ? window->mode_auto :
    !strcmp(mode, GSETTINGS_MODE_MANUAL) ? window->mode_manual :
    window->mode_none;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);

  // per proto proxy addresses
  const struct { const gchar *schema; AddrWidgets *addr; } addr_binds[] = {
    { GSETTINGS_SCHEMA_HTTP, &window->http },
    { GSETTINGS_SCHEMA_HTTPS, &window->https },
    { GSETTINGS_SCHEMA_FTP, &window->ftp },
    { GSETTINGS_SCHEMA_SOCKS, &window->socks }
  };
  for (int i = 0; i < 4; i++)
  {
    GSettings *sub = g_settings_new(addr_binds[i].schema);
    gtk_entry_set_text(GTK_ENTRY(addr_binds[i].addr->host),
      g_settings_get_string(sub, GSETTINGS_KEY_HOST)
    );
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(addr_binds[i].addr->port),
      g_settings_get_int(sub, GSETTINGS_KEY_PORT)
    );
  }
}

void write_settings(MainWindow *window)
{
  GSettings *settings = g_settings_new(GSETTINGS_SCHEMA_PROXY);

  // same proxy
  g_settings_set_boolean(settings, GSETTINGS_KEY_USE_SAME_PROXY,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(window->use_same_proxy))
  );

  // autoconfig url
  g_settings_set_string(settings, GSETTINGS_KEY_AUTOCONFIG_URL,
    gtk_entry_get_text(GTK_ENTRY(window->autoconfig_url))
  );

  // ignoring hosts
  GtkTextBuffer *buff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(window->ignore_hosts));
  GtkTextIter start, end;
  gtk_text_buffer_get_start_iter(buff, &start);
  gtk_text_buffer_get_end_iter(buff, &end);
  const gchar *text = gtk_text_iter_get_slice(&start, &end);
  gchar **items = g_strsplit_set(text, "\t\r\n, ;", -1);
  guint n = 0, count = g_strv_length(items);
  const gchar *hostlist[count + 1];
  for (guint i = 0; i < count; i++)
    if (strcmp(items[i], ""))
      hostlist[n++] = items[i];
  hostlist[n] = NULL;
  g_settings_set_strv(settings, GSETTINGS_KEY_IGNORE_HOSTS, hostlist);
  g_strfreev(items);

  // mode
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(window->mode_manual)))
    g_settings_set_string(settings, GSETTINGS_KEY_MODE, GSETTINGS_MODE_MANUAL);
  else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(window->mode_auto)))
    g_settings_set_string(settings, GSETTINGS_KEY_MODE, GSETTINGS_MODE_AUTO);
  else
    g_settings_set_string(settings, GSETTINGS_KEY_MODE, GSETTINGS_MODE_NONE);

  // per proto proxy addresses
  const struct { const gchar *schema; AddrWidgets *addr; } addr_binds[] = {
    { GSETTINGS_SCHEMA_HTTP, &window->http },
    { GSETTINGS_SCHEMA_HTTPS, &window->https },
    { GSETTINGS_SCHEMA_FTP, &window->ftp },
    { GSETTINGS_SCHEMA_SOCKS, &window->socks }
  };
  for (int i = 0; i < 4; i++)
  {
    GSettings *sub = g_settings_new(addr_binds[i].schema);
    g_settings_set_string(sub, GSETTINGS_KEY_HOST,
      gtk_entry_get_text(GTK_ENTRY(addr_binds[i].addr->host))
    );
    g_settings_set_int(sub, GSETTINGS_KEY_PORT,
      gtk_spin_button_get_value(GTK_SPIN_BUTTON(addr_binds[i].addr->port))
    );
  }
}

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

static void on_read_settings_clicked(GtkWidget *sender, MainWindow *ref)
{
  read_settings(ref);
}

static void on_write_settings_clicked(GtkWidget *sender, MainWindow *ref)
{
  write_settings(ref);
}

static void on_proxy_auth_clicked(GtkWidget *sender, MainWindow *ref)
{
  AuthDialog *dlg = new AuthDialog(GTK_WINDOW(ref->win));
  gtk_button_set_bold(GTK_BUTTON(dlg->use_auth_checkbox));
  switch (dlg->show_modal())
  {
    case GTK_RESPONSE_OK:
      write_auth_settings(dlg);
    break;
  }
  delete dlg;
}

int main (int argc, char *argv[])
{
  /* Initialize GTK+ */
  g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc)gtk_false, NULL);
  gtk_init(&argc, &argv);
  g_log_set_handler("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

  textdomain(g_get_prgname());

  MainWindow *wnd = new MainWindow();
  gtk_button_set_bold(GTK_BUTTON(wnd->mode_none));
  gtk_button_set_bold(GTK_BUTTON(wnd->mode_manual));
  gtk_button_set_bold(GTK_BUTTON(wnd->mode_auto));

  g_signal_connect(wnd->auth_button, "clicked", G_CALLBACK(on_proxy_auth_clicked), wnd);
  g_signal_connect(wnd->reset_button, "clicked", G_CALLBACK(on_read_settings_clicked), wnd);
  g_signal_connect(wnd->apply_button, "clicked", G_CALLBACK(on_write_settings_clicked), wnd);
  g_signal_connect(wnd->cancel_button, "clicked", G_CALLBACK(gtk_main_quit), wnd);

  read_settings(wnd);
  wnd->show();

  /* Enter the main loop */
  gtk_main();
  delete wnd;

  return 0;
}










