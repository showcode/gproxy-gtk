#ifndef AUTHDLG_H
#define AUTHDLG_H

#include <gtk/gtk.h>
#include <glib/gi18n.h>

class AuthDialog
{
  public:
  GtkWidget *win;
  GtkWidget *use_auth_checkbox;
  GtkWidget *username_label;
  GtkWidget *username_entry;
  GtkWidget *password_label;
  GtkWidget *password_entry;

  public:
    AuthDialog(GtkWindow *parent);
    virtual ~AuthDialog();
    gint show_modal();

  protected:
    static void on_use_auth_toggled(GtkWidget *sender, AuthDialog* ref) { ref->use_auth_changed(); };

  private:
    void use_auth_changed();
};

#endif // AUTHDLG_H
