#include <gtk/gtk.h>

gint
main ()
{
  gchar *im_module_file;

  im_module_file = gtk_rc_get_im_module_file ();
  g_print ("%s\n", im_module_file);
  g_free (im_module_file);

  return 0;
}
