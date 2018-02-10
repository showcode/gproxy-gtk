#ifndef MAINWND_H
#define MAINWND_H

#include <gtk/gtk.h>
#include <glib/gi18n.h>

typedef struct { GtkWidget *label, *host; union { GtkWidget *port; GtkWidget *checkbox; }; } RowWidgets;

typedef enum { MODE_NOTSET, MODE_NONE, MODE_MANUAL, MODE_AUTO } ProxyMode;
typedef enum { PARAM_HTTP, PARAM_HTTPS, PARAM_FTP, PARAM_SOCKS, PARAM_NOPROXY } ProxyParam;
typedef enum { TAB_GSETTINGS, TAB_ENVIRONMENT } TabIndex;

class MainWindow
{
  public:
    GtkWidget* win;
    GtkWidget *auth_button;
    GtkWidget *reset_button;
    GtkWidget *apply_button;
    GtkWidget *cancel_button;
    GtkWidget* env_source;
  protected:
    GtkWidget* notebook;
    // gsettings tab
    GtkWidget* mode_none;
    GtkWidget* mode_manual;
    GtkWidget* mode_auto;
    GtkWidget* manualbox;
    GtkWidget* autobox;
    GtkWidget* single_proxy;
    GtkWidget* autoconfig_url;
    RowWidgets gs_http;
    RowWidgets gs_https;
    RowWidgets gs_ftp;
    RowWidgets gs_socks;
    GtkWidget* gs_noproxy;
    // envionment tab
    GtkWidget* envbox;
    RowWidgets env_http;
    RowWidgets env_https;
    RowWidgets env_ftp;
    RowWidgets env_socks;
    RowWidgets env_noproxy;

  public:
    MainWindow();
    virtual ~MainWindow();
    void show();
    TabIndex get_tab_index();

    ProxyMode get_mode();
    void set_gs_mode(ProxyMode mode);
    const gchar* get_gs_autoconfig_url();
    void set_gs_autoconfig_url(const gchar* url);
    gboolean get_gs_single_proxy();
    void set_gs_single_proxy(gboolean enabled);
    const gchar* get_gs_param(ProxyParam param, int* intval);
    void set_gs_param(ProxyParam param, const gchar* value, int intval);
    gchar** get_gs_noproxy();
    void set_gs_noproxy(gchar** hosts);

    void add_env_source_name(const gchar* filename);
    void set_env_source_index(gint index_);
    gchar* get_env_source_name();
    void set_env_editable(gboolean editable);
    const gchar* get_env_param(ProxyParam param);
    void set_env_param(ProxyParam param, const gchar* proxy);

  protected:
    static void on_mode_changed(GtkWidget *sender, MainWindow *ref) { ref->mode_changed(); }
    static void on_single_proxy_toggled(GtkWidget *sender, MainWindow *ref) { ref->single_proxy_changed(); }
    static void on_env_checkbox_toggled(GtkWidget *sender, RowWidgets* ref)
    {
      gboolean sensitive = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ref->checkbox));
      gtk_widget_set_sensitive(GTK_WIDGET(ref->host), sensitive);
    };
  private:
    void create_gs_tab(GtkWidget *notebook);
    void create_env_tab(GtkWidget *notebook);
    void create_button_box(GtkWidget *container);

    void append_gs_row(GtkTable *table, guint row, const gchar *title, RowWidgets *addr);
    void append_env_row(GtkTable *table, guint row, const gchar *title, RowWidgets *addr);
    void mode_changed();
    void single_proxy_changed();
};

#endif // MAINWND_H
