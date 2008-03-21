/* $Id: gucharmap-block-chapters.h 919 2005-09-08 13:35:59Z behdad $ */
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

/* block means unicode block */

#ifndef GUCHARMAP_BLOCK_CHAPTERS_MODEL_H
#define GUCHARMAP_BLOCK_CHAPTERS_MODEL_H

#include <gucharmap/gucharmap-chapters-model.h>

G_BEGIN_DECLS

#define GUCHARMAP_BLOCK_CHAPTERS_MODEL(obj) \
            (G_TYPE_CHECK_INSTANCE_CAST ((obj), gucharmap_block_chapters_model_get_type (), GucharmapBlockChaptersModel))

#define GUCHARMAP_BLOCK_CHAPTERS_MODEL_CLASS(clazz) \
            (G_TYPE_CHECK_CLASS_CAST ((clazz), gucharmap_block_chapters_model_get_type (), GucharmapBlockChaptersModelClass))

#define GUCHARMAP_IS_BLOCK_CHAPTERS_MODEL(obj) \
            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), gucharmap_block_chapters_model_get_type ()))

#define GUCHARMAP_BLOCK_CHAPTERS_MODEL_GET_CLASS(obj) \
            (G_TYPE_INSTANCE_GET_CLASS ((obj), gucharmap_block_chapters_model_get_type (), GucharmapBlockChaptersModelClass))

typedef struct _GucharmapBlockChaptersModel GucharmapBlockChaptersModel;
typedef struct _GucharmapBlockChaptersModelClass GucharmapBlockChaptersModelClass;

struct _GucharmapBlockChaptersModel
{
  GucharmapChaptersModel parent;
};

struct _GucharmapBlockChaptersModelClass
{
  GucharmapChaptersModelClass parent_class;
};

GType                   gucharmap_block_chapters_model_get_type (void);
GucharmapChaptersModel* gucharmap_block_chapters_model_new      (void);

G_END_DECLS

#endif /* #ifndef GUCHARMAP_BLOCK_CHAPTERS_MODEL_H */
