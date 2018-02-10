#include <glib/gi18n.h>
#include "mainwnd.h"
#include "apputils.h"

MainWindow::MainWindow()
{
  /* Create the main window */
  this->win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(win), 8);
  gchar* caption = g_strdup_printf(_("Proxy properties (%s)"), g_get_user_name());
  gtk_window_set_title(GTK_WINDOW(win), caption);
  g_free(caption);
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

  /* Main vertical box */
  GtkWidget *mainbox = gtk_vbox_new(FALSE, 5);
  gtk_container_add(GTK_CONTAINER(win), mainbox);

  /* Notebook and tabs widgets */
  this->notebook = gtk_notebook_new();
  gtk_notebook_set_scrollable(GTK_NOTEBOOK(this->notebook), TRUE);
  gtk_box_pack_start(GTK_BOX(mainbox), this->notebook, TRUE, TRUE, 0);
  create_gs_tab(this->notebook);
  create_env_tab(this->notebook);

  /* Button panel */
  GtkWidget* hline = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(mainbox), hline, FALSE, FALSE, 2);
  create_button_box(mainbox);

  // preinitialize state
  set_gs_mode(MODE_NONE);
  set_gs_single_proxy(false);

  // emulate events
  mode_changed();
  single_proxy_changed();
}

void MainWindow::create_gs_tab(GtkWidget *notebook)
{
  GSList *radiogroup = NULL;

  GtkWidget *tabbox1 = gtk_vbox_new(FALSE, 5);
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tabbox1, gtk_label_new(_("GSettings")));

  /* No proxy group */
  this->mode_none = gtk_radio_button_new_with_label(radiogroup, _("Direct connect"));

  radiogroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(this->mode_none));
  g_signal_connect(this->mode_none, "toggled", G_CALLBACK(on_mode_changed), this);
  gtk_box_pack_start(GTK_BOX(tabbox1), this->mode_none, FALSE, FALSE, 0);

  /* Manual proxy group */
  this->mode_manual = gtk_radio_button_new_with_label(radiogroup, _("Manual settings"));
  radiogroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(this->mode_manual));
  g_signal_connect(this->mode_manual, "toggled", G_CALLBACK(this->on_mode_changed), this);
  gtk_box_pack_start(GTK_BOX(tabbox1), this->mode_manual, FALSE, FALSE, 0);

  this->manualbox = gtk_vbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(tabbox1), this->manualbox, FALSE, FALSE, 0);

  // table layout
  GtkWidget *table = gtk_table_new(5, 3, FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(table), 2);
  gtk_box_pack_start(GTK_BOX(manualbox), table, TRUE, TRUE, 0);

  // same_proxy checkbox
  this->single_proxy = gtk_check_button_new_with_label(_("Use same proxy for all proto"));
  g_signal_connect(this->single_proxy, "toggled", G_CALLBACK(this->on_single_proxy_toggled), this);
  gtk_table_attach_defaults(GTK_TABLE(table), this->single_proxy, 1, 2, 1, 2);
  // auth button
  this->auth_button = gtk_button_new_with_label(_("More"));
  gtk_table_attach(GTK_TABLE(table), this->auth_button, 2, 3, 1, 2, GTK_FILL, GtkAttachOptions(0), 0, 0);

  append_gs_row(GTK_TABLE(table), 0, _("HTTP"), &this->gs_http);
  append_gs_row(GTK_TABLE(table), 2, _("HTTPS"), &this->gs_https);
  append_gs_row(GTK_TABLE(table), 3, _("FTP"), &this->gs_ftp);
  append_gs_row(GTK_TABLE(table), 4, _("SOCKS"), &this->gs_socks);

  // ignore hosts label
  GtkWidget *label1 = gtk_label_new(_("Ignore proxy for this hosts"));
  gtk_misc_set_alignment(GTK_MISC(label1), 0, 0.5);
  gtk_box_pack_start(GTK_BOX(manualbox), label1, FALSE, FALSE, 0);
  // textbox with scroll
  GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll), GTK_SHADOW_IN);
  gtk_box_pack_start(GTK_BOX(manualbox), scroll, TRUE, TRUE, 0);
  this->gs_noproxy = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(this->gs_noproxy), GTK_WRAP_WORD_CHAR);
  gtk_text_view_set_accepts_tab(GTK_TEXT_VIEW(this->gs_noproxy), FALSE);
  gtk_container_add(GTK_CONTAINER(scroll), this->gs_noproxy);

  /* Autoconfig proxy group */
  this->mode_auto = gtk_radio_button_new_with_label(radiogroup, _("Autoconfig url"));
  radiogroup = gtk_radio_button_get_group(GTK_RADIO_BUTTON(this->mode_auto));
  g_signal_connect(this->mode_auto, "toggled", G_CALLBACK(this->on_mode_changed), this);
  gtk_box_pack_start(GTK_BOX(tabbox1), this->mode_auto, FALSE, FALSE, 0);

  this->autobox = gtk_hbox_new(FALSE, 2);
  gtk_box_pack_start(GTK_BOX(tabbox1), autobox, FALSE, FALSE, 0);

  GtkWidget *label2 = gtk_label_new(_("Url"));
  gtk_box_pack_start(GTK_BOX(autobox), label2, FALSE, FALSE, 0);

  this->autoconfig_url = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(autobox), this->autoconfig_url, TRUE, TRUE, 0);

  /* set bold text for radio's */
  gtk_button_set_bold(GTK_BUTTON(this->mode_none));
  gtk_button_set_bold(GTK_BUTTON(this->mode_manual));
  gtk_button_set_bold(GTK_BUTTON(this->mode_auto));
}

void MainWindow::create_env_tab(GtkWidget *notebook)
{
  GtkWidget *tabbox2 = gtk_vbox_new(FALSE, 15);
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tabbox2, gtk_label_new(_("Environment")));

  /* source selector */

  GtkWidget *hbox1 = gtk_hbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(tabbox2), hbox1, FALSE, TRUE, 0);

  // widgets for noproxy
  GtkWidget *label1 = gtk_label_new(_("Source"));
  gtk_misc_set_alignment(GTK_MISC(label1), 0, 0.5);
  gtk_box_pack_start(GTK_BOX(hbox1), label1, FALSE, FALSE, 0);
  this->env_source = gtk_combo_box_new_text();
  gtk_box_pack_start(GTK_BOX(hbox1), this->env_source, TRUE, TRUE, 0);

  /* proxy adresses group */

  this->envbox = gtk_vbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(tabbox2), this->envbox, FALSE, FALSE, 0);

  // table layout
  GtkWidget *table = gtk_table_new(4, 3, FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(table), 2);
  gtk_box_pack_start(GTK_BOX(this->envbox), table, FALSE, FALSE, 0);

  append_env_row(GTK_TABLE(table), 0, _("HTTP"), &this->env_http);
  append_env_row(GTK_TABLE(table), 1, _("HTTPS"), &this->env_https);
  append_env_row(GTK_TABLE(table), 2, _("FTP"), &this->env_ftp);
  append_env_row(GTK_TABLE(table), 3, _("SOCKS"), &this->env_socks);

  // widgets for noproxy
  RowWidgets* w = &this->env_noproxy;
  w->checkbox = gtk_check_button_new_with_label(_("Ignore proxy for this hosts"));
  w->label = gtk_bin_get_child(GTK_BIN(w->checkbox));
  gtk_misc_set_alignment(GTK_MISC(w->label), 0, 0.5);
  gtk_box_pack_start(GTK_BOX(this->envbox), w->checkbox, FALSE, FALSE, 0);
  w->host = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(this->envbox), w->host, FALSE, TRUE, 0);
  g_signal_connect(w->checkbox, "toggled", G_CALLBACK(this->on_env_checkbox_toggled), w);
  this->on_env_checkbox_toggled(w->checkbox, w);
}

void MainWindow::create_button_box(GtkWidget *container)
{
  GtkWidget *buttonbox = gtk_hbutton_box_new();
  gtk_hbutton_box_set_layout_default(GTK_BUTTONBOX_END);
  gtk_box_pack_end(GTK_BOX(container), buttonbox, FALSE, FALSE, 0);

  this->reset_button = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
  gtk_box_pack_start(GTK_BOX(buttonbox), this->reset_button, FALSE, FALSE, 0);

  this->apply_button = gtk_button_new_from_stock(GTK_STOCK_APPLY);
  gtk_box_pack_start(GTK_BOX(buttonbox), this->apply_button, FALSE, FALSE, 0);

  this->cancel_button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
  gtk_box_pack_start(GTK_BOX(buttonbox), this->cancel_button, FALSE, FALSE, 0);
}

void MainWindow::append_gs_row(GtkTable *table, guint row,
  const gchar *title, RowWidgets* addr)
{
  addr->label = gtk_label_new(title);
  gtk_misc_set_alignment(GTK_MISC(addr->label), 0, 0.5);
  addr->host = gtk_entry_new();
  addr->port = gtk_spin_button_new_with_range(0, 65535, 1);
  // append row
  gtk_table_attach(table, addr->label, 0, 1, row, row + 1, GtkAttachOptions(GTK_FILL), GtkAttachOptions(0), 0, 0);
  gtk_table_attach(table, addr->host, 1, 2, row, row + 1, GtkAttachOptions(GTK_FILL | GTK_EXPAND), GtkAttachOptions(0), 2, 0);
  gtk_table_attach(table, addr->port, 2, 3, row, row + 1, GtkAttachOptions(GTK_FILL), GtkAttachOptions(0), 0, 0);
}

void MainWindow::append_env_row(GtkTable *table, guint row,
  const gchar *title, RowWidgets* w)
{
  w->checkbox = gtk_check_button_new_with_label(title);
  w->label = gtk_bin_get_child(GTK_BIN(w->checkbox));
  gtk_misc_set_alignment(GTK_MISC(w->label), 0, 0.5);
  w->host = gtk_entry_new();
  // append row
  gtk_table_attach(table, w->checkbox, 0, 1, row, row + 1, GtkAttachOptions(GTK_FILL), GtkAttachOptions(0), 0, 0);
  gtk_table_attach(table, w->host, 1, 2, row, row + 1, GtkAttachOptions(GTK_FILL | GTK_EXPAND), GtkAttachOptions(0), 2, 0);
  g_signal_connect(w->checkbox, "toggled", G_CALLBACK(this->on_env_checkbox_toggled), w);
  this->on_env_checkbox_toggled(w->checkbox, w);
}

MainWindow::~MainWindow()
{
  gtk_widget_destroy(this->win);
}

void MainWindow::show()
{
  gtk_widget_show_all(win);
  gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
}

TabIndex MainWindow::get_tab_index()
{
  return (TabIndex)gtk_notebook_get_current_page(GTK_NOTEBOOK(this->notebook));
}

void MainWindow::set_gs_single_proxy(gboolean enabled)
{
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->single_proxy), enabled);
}

gboolean MainWindow::get_gs_single_proxy()
{
  return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(this->single_proxy));
}

ProxyMode MainWindow::get_mode()
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(this->mode_auto)))
    return MODE_AUTO;

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(this->mode_manual)))
    return MODE_MANUAL;

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(this->mode_none)))
    return MODE_NONE;

  return MODE_NOTSET;
}

void MainWindow::set_gs_mode(ProxyMode mode)
{
  if (mode == MODE_AUTO)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->mode_auto), TRUE);

  if (mode == MODE_MANUAL)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->mode_manual), TRUE);

  if (mode == MODE_NONE)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->mode_none), TRUE);
}

const gchar* MainWindow::get_gs_autoconfig_url()
{
  return gtk_entry_get_text(GTK_ENTRY(this->autoconfig_url));
}

void MainWindow::set_gs_autoconfig_url(const gchar* url)
{
  gtk_entry_set_text(GTK_ENTRY(this->autoconfig_url), url);
}

gchar** MainWindow::get_gs_noproxy()
{
  GtkTextBuffer *buff;
  GtkTextIter start, end;

  // getting hosts from text
  buff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(this->gs_noproxy));
  gtk_text_buffer_get_start_iter(buff, &start);
  gtk_text_buffer_get_end_iter(buff, &end);
  const gchar *text = gtk_text_iter_get_slice(&start, &end);

  gchar **items = g_strsplit_set(text, "\t\r\n, ;", -1);
  strtidy(items, "");
  return items;
}

void MainWindow::set_gs_noproxy(gchar** hosts)
{
   GtkTextBuffer *buff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(this->gs_noproxy));
   gchar* value = g_strjoinv("\r", hosts);
   gtk_text_buffer_set_text(buff, value, -1);
   g_free(value);
}

const gchar* MainWindow::get_gs_param(ProxyParam param, int* intval)
{
  const RowWidgets* ws[] = {
    &this->gs_http, &this->gs_https, &this->gs_ftp, &this->gs_socks };

  if (PARAM_HTTP <= param && param <= PARAM_SOCKS)
  {
    if (intval)
      *intval = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ws[param]->port));
    return gtk_entry_get_text(GTK_ENTRY(ws[param]->host));
  }
//  else if (param == PARAM_NOPROXY)
//  {
//    GtkTextBuffer *buff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(this->gs_noproxy));
//    GtkTextIter start, end;
//    gtk_text_buffer_get_bounds(buff, &start, &end);
//    return FUCK gtk_text_buffer_get_text(buff, &start, &end, FALSE);
//  }
  return NULL;
}

void MainWindow::set_gs_param(ProxyParam param, const gchar* value, int intval)
{
  const RowWidgets* ws[] = {
    &this->gs_http, &this->gs_https, &this->gs_ftp, &this->gs_socks };

  if (PARAM_HTTP <= param && param <= PARAM_SOCKS)
  {
    gtk_entry_set_text(GTK_ENTRY(ws[param]->host), value);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ws[param]->port), intval);
  }
//  else if (param == PARAM_NOPROXY)
//  {
//    GtkTextBuffer *buff = gtk_text_view_get_buffer(GTK_TEXT_VIEW(this->gs_noproxy));
//    gtk_text_buffer_set_text(buff, value, -1);
//  }
}

const gchar* MainWindow::get_env_param(ProxyParam param)
{
  const RowWidgets* ws[] = { &this->env_http, &this->env_https,
    &this->env_ftp, &this->env_socks, &this->env_noproxy };

  if (PARAM_HTTP <= param && param <= PARAM_NOPROXY)
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ws[param]->checkbox)))
      return gtk_entry_get_text(GTK_ENTRY(ws[param]->host));
  return NULL;
}

void MainWindow::set_env_param(ProxyParam param, const gchar* proxy)
{
  const RowWidgets* ws[] = { &this->env_http, &this->env_https,
    &this->env_ftp, &this->env_socks, &this->env_noproxy };

  if (PARAM_HTTP <= param && param <= PARAM_NOPROXY)
  {
    gtk_entry_set_text(GTK_ENTRY(ws[param]->host), !!proxy ? proxy : "");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ws[param]->checkbox), !!proxy);
  }
}

void MainWindow::mode_changed()
{
  ProxyMode mode = get_mode();
  gtk_widget_set_sensitive(this->manualbox, mode == MODE_MANUAL);
  gtk_widget_set_sensitive(this->autobox, mode == MODE_AUTO);
}

void MainWindow::single_proxy_changed()
{
  gboolean enabled = !get_gs_single_proxy();
  const RowWidgets *ws[] = {
    &this->gs_https, &this->gs_ftp, &this->gs_socks };
  for (const RowWidgets* w : ws)
  {
    gtk_widget_set_sensitive(w->label, enabled);
    gtk_widget_set_sensitive(w->host, enabled);
    gtk_widget_set_sensitive(w->port, enabled);
  }
}

void MainWindow::add_env_source_name(const gchar* filename)
{
  gtk_combo_box_append_text(GTK_COMBO_BOX(this->env_source), filename);
}

gchar* MainWindow::get_env_source_name()
{
  return gtk_combo_box_get_active_text(GTK_COMBO_BOX(this->env_source));
}

void MainWindow::set_env_source_index(gint index_)
{
  gtk_combo_box_set_active(GTK_COMBO_BOX(this->env_source), index_);
}

void MainWindow::set_env_editable(gboolean editable)
{
  gtk_widget_set_sensitive(GTK_WIDGET(this->envbox), editable);
}


