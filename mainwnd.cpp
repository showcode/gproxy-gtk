#include "mainwnd.h"

MainWindow::MainWindow()
{
  /* Create the main window */
  this->win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(win), 8);
  gtk_window_set_title(GTK_WINDOW(win), _("Proxy properties"));
  gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
  GdkGeometry geometry /*= { max_height: -1, max_width: G_MAXINT }*/;
  geometry.max_height = -1;
  geometry.max_width = G_MAXINT;
  gtk_window_set_geometry_hints(GTK_WINDOW(win), win, &geometry, GDK_HINT_MAX_SIZE);
  gtk_widget_realize(win);
  g_signal_connect(win, "destroy", gtk_main_quit, this);
  // disable maximization
  GdkWindow* window = gtk_widget_get_window(win);
  gdk_window_set_functions(window, GdkWMFunction(GDK_FUNC_RESIZE | GDK_FUNC_MOVE | GDK_FUNC_MINIMIZE | GDK_FUNC_CLOSE));
  gdk_window_set_decorations(window, GdkWMDecoration(GDK_DECOR_BORDER | GDK_DECOR_RESIZEH | GDK_DECOR_TITLE | GDK_DECOR_MENU | GDK_DECOR_MINIMIZE));

  /* Create a vertical box */
  GtkWidget *mainbox = gtk_vbox_new(FALSE, 5);
  gtk_container_add(GTK_CONTAINER(win), mainbox);

  GtkWidget *widget = NULL;
  GSList *radiogroup = NULL;

  /* No proxy panel */
  this->mode_none = gtk_radio_button_new_with_label(radiogroup, _("Direct connect"));
  radiogroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(this->mode_none));
  g_signal_connect(this->mode_none, "toggled", G_CALLBACK(on_mode_changed), this);
  gtk_box_pack_start(GTK_BOX(mainbox), this->mode_none, FALSE, FALSE, 0);

  /* Manual proxy panel */
  this->mode_manual = gtk_radio_button_new_with_label(radiogroup, _("Manual settings"));
  radiogroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(this->mode_manual));
  g_signal_connect(this->mode_manual, "toggled", G_CALLBACK(on_mode_changed), this);
  gtk_box_pack_start(GTK_BOX(mainbox), this->mode_manual, FALSE, FALSE, 0);

  this->manualbox = gtk_vbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(mainbox), this->manualbox, FALSE, FALSE, 0);

  // table layout
  GtkWidget *table = gtk_table_new(5, 3, FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(table), 2);
  gtk_box_pack_start(GTK_BOX(manualbox), table, TRUE, TRUE, 0);

  // same_proxy checkbox
  this->use_same_proxy = gtk_check_button_new_with_label(_("Use same proxy for all proto"));
  g_signal_connect(this->use_same_proxy, "toggled", G_CALLBACK(on_use_same_proxy_toggled), this);
  gtk_table_attach_defaults(GTK_TABLE(table), this->use_same_proxy, 1, 2, 1, 2);
  // auth button
  this->auth_button = gtk_button_new_with_label(_("More"));
  gtk_table_attach(GTK_TABLE(table), this->auth_button, 2, 3, 1, 2, GTK_FILL, GtkAttachOptions(0), 0, 0);

  append_hostaddr(GTK_TABLE(table), 0, _("HTTP"), &this->http);
  append_hostaddr(GTK_TABLE(table), 2, _("HTTPS"), &this->https);
  append_hostaddr(GTK_TABLE(table), 3, _("FTP"), &this->ftp);
  append_hostaddr(GTK_TABLE(table), 4, _("SOCKS"), &this->socks);

  // ignore hosts label
  widget = gtk_label_new(_("Ignore proxy for this hosts"));
  gtk_misc_set_alignment(GTK_MISC(widget), 0, 0.5);
  gtk_box_pack_start(GTK_BOX(manualbox), widget, FALSE, FALSE, 0);
  // textbox with scroll
  GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll), GTK_SHADOW_IN);
  gtk_box_pack_start(GTK_BOX(manualbox), scroll, TRUE, TRUE, 0);
  this->ignore_hosts = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(this->ignore_hosts), GTK_WRAP_WORD_CHAR);
  gtk_text_view_set_accepts_tab(GTK_TEXT_VIEW(this->ignore_hosts), FALSE);
  gtk_container_add(GTK_CONTAINER(scroll), this->ignore_hosts);

  /* Autoconfig proxy panel */
  this->mode_auto = gtk_radio_button_new_with_label(radiogroup, _("Autoconfig url"));
  radiogroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(this->mode_auto));
  g_signal_connect(this->mode_auto, "toggled", G_CALLBACK(on_mode_changed), this);
  gtk_box_pack_start(GTK_BOX(mainbox), this->mode_auto, FALSE, FALSE, 0);

  this->autobox = gtk_hbox_new(FALSE, 2);
  gtk_box_pack_start(GTK_BOX(mainbox), autobox, FALSE, FALSE, 0);

  widget = gtk_label_new(_("Url"));
  gtk_box_pack_start(GTK_BOX(autobox), widget, FALSE, FALSE, 0);

  this->autoconfig_url = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(autobox), this->autoconfig_url, TRUE, TRUE, 0);

  /* Button panel */
  widget = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(mainbox), widget, FALSE, FALSE, 2);

  GtkWidget *buttonbox = gtk_hbutton_box_new();
  gtk_hbutton_box_set_layout_default(GTK_BUTTONBOX_END);
  gtk_box_pack_end(GTK_BOX(mainbox), buttonbox, FALSE, FALSE, 0);

  reset_button = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
  gtk_box_pack_start(GTK_BOX(buttonbox), reset_button, FALSE, FALSE, 0);

  apply_button = gtk_button_new_from_stock(GTK_STOCK_APPLY);
  gtk_box_pack_start(GTK_BOX(buttonbox), apply_button, FALSE, FALSE, 0);

  cancel_button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
  gtk_box_pack_start(GTK_BOX(buttonbox), cancel_button, FALSE, FALSE, 0);

  // preinitialize state
  mode_changed();
  use_same_proxy_changed();
}

MainWindow::~MainWindow()
{
  //dtor
}

void MainWindow::show()
{
  gtk_widget_show_all(win);
  gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
}

void MainWindow::append_hostaddr(GtkTable *table, guint row,
  const gchar *title, AddrWidgets *addr)
{
  addr->label = gtk_label_new(title);
  gtk_misc_set_alignment(GTK_MISC(addr->label), 0, 0.5);
  addr->host = gtk_entry_new();
  addr->port = gtk_spin_button_new_with_range(0, 65535, 1);
  // append row
  gtk_table_attach(table, addr->label, 0, 1, row, row + 1, GTK_FILL, GtkAttachOptions(0), 0, 0);
  gtk_table_attach(table, addr->host, 1, 2, row, row + 1, GtkAttachOptions(GTK_FILL | GTK_EXPAND), GtkAttachOptions(0), 2, 0);
  gtk_table_attach(table, addr->port, 2, 3, row, row + 1, GTK_FILL, GtkAttachOptions(0), 0, 0);
}

void MainWindow::mode_changed()
{
  gtk_widget_set_sensitive(this->manualbox,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(this->mode_manual))
  );
  gtk_widget_set_sensitive(this->autobox,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(this->mode_auto))
  );
}

void MainWindow::use_same_proxy_changed()
{
  gboolean enabled = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(this->use_same_proxy));
  AddrWidgets *arr[] = { &this->http, &this->https, &this->ftp, &this->socks };
  for (AddrWidgets *addr : arr)
  {
    gtk_widget_set_sensitive(addr->label, enabled);
    gtk_widget_set_sensitive(addr->host, enabled);
    gtk_widget_set_sensitive(addr->port, enabled);
  }
}

