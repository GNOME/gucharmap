/* $Id$ */
/*
 * Copyright (c) 2004 Noah Levitt
 * Copyright (C) 2007 Christian Persch
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02110-1301  USA
 */

#include "config.h"

#include "gucharmap-script-chapters-model.h"
#include "gucharmap-script-chapters.h"

static void
gucharmap_script_chapters_init (GucharmapScriptChapters *chapters)
{
}

static void
gucharmap_script_chapters_class_init (GucharmapScriptChaptersClass *clazz)
{
}

G_DEFINE_TYPE (GucharmapScriptChapters, gucharmap_script_chapters, GUCHARMAP_TYPE_CHAPTERS)

GtkWidget * 
gucharmap_script_chapters_new (void)
{
  GucharmapChapters *chapters;
  GucharmapChaptersModel *model;
  
  model = gucharmap_script_chapters_model_new ();
  chapters = g_object_new (gucharmap_script_chapters_get_type (),
                           "hadjustment", NULL,
                           "vadjustment", NULL,
                           "chapters-model", model,
                           NULL);
  g_object_unref (model);

  return GTK_WIDGET (chapters);
}
