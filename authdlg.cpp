#include <glib/gi18n.h>
#include "authdlg.h"
#include "apputils.h"

AuthDialog::AuthDialog(GtkWindow *parent)
{
  // dialog
  this->win = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(this->win), _("Proxy authentication"));
  gtk_window_set_transient_for(GTK_WINDOW(this->win), GTK_WINDOW(parent));
  gtk_window_set_modal(GTK_WINDOW(this->win), TRUE);
  gtk_window_set_destroy_with_parent(GTK_WINDOW(this->win), TRUE);
  gtk_window_set_resizable(GTK_WINDOW(this->win), FALSE);
  gtk_dialog_add_button(GTK_DIALOG(this->win), GTK_STOCK_OK, GTK_RESPONSE_OK);
  gtk_dialog_add_button(GTK_DIALOG(this->win), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);

  // table layout
  GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(this->win));
  GtkWidget *table = gtk_table_new(3, 2, FALSE);
  gtk_container_set_border_width(GTK_CONTAINER(table), 5);
  gtk_table_set_row_spacings(GTK_TABLE(table), 2);
  gtk_container_add(GTK_CONTAINER(content), table);

  // checkbox
  this->use_auth_checkbox = gtk_check_button_new_with_label(_("Use authetication"));
  gtk_button_set_bold(GTK_BUTTON(this->use_auth_checkbox));
  g_signal_connect(this->use_auth_checkbox, "toggled", G_CALLBACK(on_use_auth_toggled), this);
  gtk_table_attach(GTK_TABLE(table), this->use_auth_checkbox, 0, 2, 0, 1, GTK_FILL, GtkAttachOptions(0), 0, 0);

  // username
  this->username_label = gtk_label_new(_("Username"));
  gtk_table_attach(GTK_TABLE(table), this->username_label, 0, 1, 1, 2, GTK_FILL, GtkAttachOptions(0), 0, 0);
  this->username_entry = gtk_entry_new();
  gtk_table_attach(GTK_TABLE(table), this->username_entry, 1, 2, 1, 2, GtkAttachOptions(GTK_FILL | GTK_EXPAND), GtkAttachOptions(0), 2, 0);

  // password
  this->password_label = gtk_label_new(_("Password"));
  gtk_table_attach(GTK_TABLE(table), this->password_label, 0, 1, 2, 3, GTK_FILL, GtkAttachOptions(0), 0, 0);
  this->password_entry = gtk_entry_new();
  gtk_table_attach(GTK_TABLE(table), this->password_entry, 1, 2, 2, 3, GtkAttachOptions(GTK_FILL | GTK_EXPAND), GtkAttachOptions(0), 2, 0);

  // pre initialize
  use_auth_changed();
}

AuthDialog::~AuthDialog()
{
  gtk_widget_destroy(this->win);
}

void AuthDialog::use_auth_changed()
{
  gboolean enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(this->use_auth_checkbox));
  GtkWidget *widgets[] = {
    this->username_label, this->username_entry,
    this->password_label, this->password_entry
  };
  for (GtkWidget *widget : widgets)
    gtk_widget_set_sensitive(widget, enabled);
}

gint AuthDialog::show_modal()
{
  gtk_widget_show_all(GTK_WIDGET(this->win));
  gtk_window_set_position(GTK_WINDOW(this->win), GTK_WIN_POS_CENTER_ON_PARENT);
  return gtk_dialog_run(GTK_DIALOG(this->win));
}

gboolean AuthDialog::get_use_auth()
{
  return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(this->use_auth_checkbox));
}

void AuthDialog::set_use_auth(gboolean use)
{
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->use_auth_checkbox), use);
}

void AuthDialog::get_auth(const gchar* &username, const gchar* &password)
{
  username = gtk_entry_get_text(GTK_ENTRY(this->username_entry));
  password = gtk_entry_get_text(GTK_ENTRY(this->password_entry));
}

void AuthDialog::set_auth(const gchar* username, const gchar* password)
{
  gtk_entry_set_text(GTK_ENTRY(this->username_entry), username);
  gtk_entry_set_text(GTK_ENTRY(this->password_entry), password);
}
