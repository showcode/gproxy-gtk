#ifndef MAINWND_H
#define MAINWND_H

#include <gtk/gtk.h>
#include <glib/gi18n.h>

typedef struct { GtkWidget *label, *host, *port; } AddrWidgets;

class MainWindow
{
  public:
    GtkWidget* win;
    GtkWidget* mode_none;
    GtkWidget* mode_manual;
    GtkWidget* mode_auto;
    GtkWidget* manualbox;
    GtkWidget* autobox;
    GtkWidget* use_same_proxy;
    GtkWidget* ignore_hosts;
    GtkWidget* autoconfig_url;
    GtkWidget *auth_button;
    GtkWidget *reset_button;
    GtkWidget *apply_button;
    GtkWidget *cancel_button;
    AddrWidgets http;
    AddrWidgets https;
    AddrWidgets ftp;
    AddrWidgets socks;

  public:
    MainWindow();
    virtual ~MainWindow();
    void show();

  protected:
    static void on_mode_changed(GtkWidget *sender, MainWindow *ref) { ref->mode_changed(); }
    static void on_use_same_proxy_toggled(GtkWidget *sender, MainWindow *ref) { ref->use_same_proxy_changed(); }
  private:
    void gtk_button_set_bold(GtkButton *button);
    void append_hostaddr(GtkTable *table, guint row, const gchar *title, AddrWidgets *addr);
    void mode_changed();
    void use_same_proxy_changed();
};

#endif // MAINWND_H
