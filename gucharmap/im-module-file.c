#include <gtk/gtk.h>

gint
main (gint argc, gchar **argv)
{
  g_print ("%s\n", gtk_rc_get_im_module_file ());
  return 0;
}
