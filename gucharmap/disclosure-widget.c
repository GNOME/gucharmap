/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * 19:27 < iain> noah; okay, I hope you're going to use that widget for it :)
 * 19:27 < noah> iain: what widget is that?
 * 19:28 < iain> its in gnome-panel/gnome-panel/disclosure-widget.[ch]
 * 19:28 < iain> just cut and paste it
 *
 *
 *  Authors: Iain Holmes <iain@ximian.com>
 *
 *  Copyright 2002 Iain Holmes
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtktogglebutton.h>

#include "gucharmap_intl.h"
#include "disclosure-widget.h"

static GtkCheckButtonClass *parent_class = NULL;

struct _CDDBDisclosurePrivate {
	GtkWidget *container;
	char *shown;
	char *hidden;
	
	guint32 expand_id;
	GtkExpanderStyle style;

	int expander_size;
	int direction;
};

static void
finalize (GObject *object)
{
	CDDBDisclosure *disclosure;

	disclosure = CDDB_DISCLOSURE (object);
	if (disclosure->priv == NULL) {
		return;
	}

	g_free (disclosure->priv->hidden);
	g_free (disclosure->priv->shown);

	if (disclosure->priv->container != NULL) {
		g_object_unref (G_OBJECT (disclosure->priv->container));
	}
	
	g_free (disclosure->priv);
	disclosure->priv = NULL;

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
get_x_y (CDDBDisclosure *disclosure,
	 int *x,
	 int *y,
	 GtkStateType *state_type)
{
	GtkCheckButton *check_button;
	GdkRectangle new_area, restrict_area;
	int indicator_size, indicator_spacing;
	int focus_width;
	int focus_pad;
	gboolean interior_focus;
	GtkWidget *widget = GTK_WIDGET (disclosure);
	GtkAllocation *area = &widget->allocation;
	GtkBin *bin = GTK_BIN (disclosure);
	GtkRequisition child_requisition;
	int width, height;
	
	if (GTK_WIDGET_VISIBLE (disclosure) &&
	    GTK_WIDGET_MAPPED (disclosure)) {
		check_button = GTK_CHECK_BUTTON (disclosure);
		
		gtk_widget_style_get (widget,
				      "interior_focus", &interior_focus,
				      "focus-line-width", &focus_width,
				      "focus-padding", &focus_pad,
				      NULL);
		
		*state_type = GTK_WIDGET_STATE (widget);
		if ((*state_type != GTK_STATE_NORMAL) &&
		    (*state_type != GTK_STATE_PRELIGHT)) {
			*state_type = GTK_STATE_NORMAL;
		}

		if (bin->child) {
			width = bin->child->allocation.x - widget->allocation.x - (2 * GTK_CONTAINER (widget)->border_width);
		} else {
			width = widget->allocation.width;
		}
		
		*x = widget->allocation.x + (width) / 2;
		*y = widget->allocation.y + widget->allocation.height / 2;

		if (interior_focus == FALSE) {
			*x += focus_width + focus_pad;
		}

		*state_type = GTK_WIDGET_STATE (widget) == GTK_STATE_ACTIVE ? GTK_STATE_NORMAL : GTK_WIDGET_STATE (widget);

		if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL) {
			*x = widget->allocation.x + widget->allocation.width - (indicator_size + *x - widget->allocation.x);
		}
	} else {
		*x = 0;
		*y = 0;
		*state_type = GTK_STATE_NORMAL;
	}
}

static gboolean
expand_collapse_timeout (gpointer data)
{
	GdkRectangle area;
	GtkWidget *widget = data;
	CDDBDisclosure *disclosure = data;
	GtkStateType state_type;
	int x, y;
	
	gdk_window_invalidate_rect (widget->window, &widget->allocation, TRUE);
	get_x_y (disclosure, &x, &y, &state_type);
	
	gtk_paint_expander (widget->style,
			    widget->window,
			    state_type,
			    &widget->allocation,
			    widget,
			    "disclosure",
			    x, y,
			    disclosure->priv->style);

	disclosure->priv->style += disclosure->priv->direction;
	if ((int) disclosure->priv->style > (int) GTK_EXPANDER_EXPANDED) {
		disclosure->priv->style = GTK_EXPANDER_EXPANDED;

		if (disclosure->priv->container != NULL) {
			gtk_widget_show (disclosure->priv->container);
		}

		g_object_set (G_OBJECT (disclosure),
			      "label", disclosure->priv->hidden,
			      NULL);
		return FALSE;
	} else if ((int) disclosure->priv->style < (int) GTK_EXPANDER_COLLAPSED) {
		disclosure->priv->style = GTK_EXPANDER_COLLAPSED;

		if (disclosure->priv->container != NULL) {
			gtk_widget_hide (disclosure->priv->container);
		}

		g_object_set (G_OBJECT (disclosure),
			      "label", disclosure->priv->shown,
			      NULL);

		return FALSE;
	} else {
		return TRUE;
	}
}

static void
do_animation (CDDBDisclosure *disclosure,
	      gboolean opening)
{
	if (disclosure->priv->expand_id > 0) {
		gtk_timeout_remove (disclosure->priv->expand_id);
	}

	disclosure->priv->direction = opening ? 1 : -1;
	disclosure->priv->expand_id = g_timeout_add (50, expand_collapse_timeout, disclosure);
}

static void
toggled (GtkToggleButton *tb)
{
	CDDBDisclosure *disclosure;

	disclosure = CDDB_DISCLOSURE (tb);
	do_animation (disclosure, gtk_toggle_button_get_active (tb));

	if (disclosure->priv->container == NULL) {
		return;
	}
}

static void
draw_indicator (GtkCheckButton *check,
		GdkRectangle *area)
{
	GtkWidget *widget = GTK_WIDGET (check);
	CDDBDisclosure *disclosure = CDDB_DISCLOSURE (check);
	GtkStateType state_type;
	int x, y;

	get_x_y (disclosure, &x, &y, &state_type);
	gtk_paint_expander (widget->style,
			    widget->window,
			    state_type,
			    area,
			    widget,
			    "treeview",
			    x, y,
			    disclosure->priv->style);
}

static void
class_init (CDDBDisclosureClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkCheckButtonClass *button_class;
	GtkToggleButtonClass *toggle_class;
	
	object_class = G_OBJECT_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);
	button_class = GTK_CHECK_BUTTON_CLASS (klass);
	toggle_class = GTK_TOGGLE_BUTTON_CLASS (klass);
	
	toggle_class->toggled = toggled;
	button_class->draw_indicator = draw_indicator;

	object_class->finalize = finalize;

	parent_class = g_type_class_peek_parent (klass);

	gtk_widget_class_install_style_property (widget_class,
						 g_param_spec_int ("expander_size",
								   _("Expander Size"),
								   _("Size of the expander arrow"),
								   0, G_MAXINT,
								   10, G_PARAM_READABLE));
}

static void
init (CDDBDisclosure *disclosure)
{
	disclosure->priv = g_new0 (CDDBDisclosurePrivate, 1);
	disclosure->priv->expander_size = 10;
}

GType
cddb_disclosure_get_type (void)
{
	static GType type = 0;

	if (type == 0) {
		GTypeInfo info = {
			sizeof (CDDBDisclosureClass),
			NULL, NULL, (GClassInitFunc) class_init, NULL, NULL,
			sizeof (CDDBDisclosure), 0, (GInstanceInitFunc) init
		};

		type = g_type_register_static (GTK_TYPE_CHECK_BUTTON, "CDDBDisclosure", &info, 0);
	}

	return type;
}

GtkWidget *
cddb_disclosure_new (GtkWidget *widget,
                     const gchar *shown,
		     const gchar *hidden)
{
	CDDBDisclosure *disclosure;

	disclosure = g_object_new (cddb_disclosure_get_type (), "label", shown, NULL);

	disclosure->priv->shown = g_strdup (shown);
	disclosure->priv->hidden = g_strdup (hidden);

	g_object_ref (G_OBJECT (widget));
	disclosure->priv->container = widget;

	return GTK_WIDGET (disclosure);
}

