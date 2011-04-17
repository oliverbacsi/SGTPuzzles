/*
 * ps3menu.c : PS3 Menu drawing API
 *
 * Copyright (C) Youness Alaoui (KaKaRoTo)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#include <malloc.h>
#include <string.h>
#include <assert.h>

#include "ps3menu.h"

static void
_draw_text (Ps3Menu *menu, Ps3MenuItem *item, cairo_t *cr,
    int x, int y)
{
  cairo_font_extents_t fex;

  cairo_save (cr);
  cairo_select_font_face(cr,
      "sans-serif",
      CAIRO_FONT_SLANT_NORMAL,
      CAIRO_FONT_WEIGHT_BOLD);

  cairo_set_font_size(cr, item->text_size);

  cairo_font_extents (cr, &fex);

  y += fex.ascent + fex.descent;

  cairo_set_source_rgba (cr, item->text_color.red, item->text_color.green,
      item->text_color.blue, item->text_color.alpha);
  cairo_move_to(cr, x, y);
  cairo_show_text (cr, item->text);
  cairo_restore (cr);
}

static int
_draw_item (Ps3Menu *menu, Ps3MenuItem *item,
    int selected, cairo_t *cr, int x, int y, void *user_data)
{
  cairo_surface_t *bg;
  int width = item->width - (2 * item->ipad_x);
  int height = item->height - (2 * item->ipad_y);

  cairo_save (cr);

  cairo_rectangle (cr, x, y, item->width, item->height);
  cairo_clip (cr);

  if (selected)
    bg = item->bg_sel_image;
  else
    bg = item->bg_image;

  cairo_scale (cr, (float) item->width / cairo_image_surface_get_width (bg),
      (float) item->height / cairo_image_surface_get_height (bg));
  cairo_set_source_surface (cr, bg, x, y);

  /* Avoid getting the edge blended with 0 alpha */
  cairo_pattern_set_extend (cairo_get_source(cr), CAIRO_EXTEND_PAD);

  /* Replace the destination with the source instead of overlaying */
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

  cairo_paint (cr);
  cairo_identity_matrix (cr);

  cairo_rectangle (cr, x + item->ipad_x, y + item->ipad_y, width, height);
  cairo_clip (cr);
  _draw_text (menu, item, cr, x + item->ipad_x, y + item->ipad_y);
  cairo_restore (cr);

	return 0;
}

Ps3Menu *
ps3_menu_new (cairo_surface_t *surface, int rows, int columns,
    int default_item_width, int default_item_height)
{
  Ps3Menu *menu = NULL;
  cairo_pattern_t *linpat = NULL;
  cairo_t *grad_cr = NULL;

  /* Infinite scrolling horizontally and vertically is not practical,
   * return error  */
  if (rows == -1 && columns == -1)
    return NULL;

  menu = malloc (sizeof(Ps3Menu));

  memset (menu, 0, sizeof(Ps3Menu));
  menu->surface = cairo_surface_reference (surface);
  menu->rows = rows;
  menu->columns = columns;
  menu->default_item_width = default_item_width;
  menu->default_item_height = default_item_height;
  menu->nitems = 0;
  menu->items = NULL;
  menu->selection = 0; /* Select the first item by default */
  menu->pad_x = PS3_MENU_DEFAULT_PAD_X;
  menu->pad_y = PS3_MENU_DEFAULT_PAD_Y;
  menu->start_item = 0;

  menu->bg_image = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
      default_item_width, default_item_height);

  linpat = cairo_pattern_create_linear (default_item_width, 0,
        default_item_width, default_item_height);

  cairo_pattern_add_color_stop_rgb (linpat, 0.0, 0.0, 0.0, 0.0);
  cairo_pattern_add_color_stop_rgb (linpat, 0.3, 0.4, 0.4, 0.4);
  cairo_pattern_add_color_stop_rgb (linpat, 0.7, 0.3, 0.3, 0.3);
  cairo_pattern_add_color_stop_rgb (linpat, 1.0, 0.0, 0.0, 0.0);

  grad_cr = cairo_create (menu->bg_image);
  cairo_set_source (grad_cr, linpat);
  cairo_paint (grad_cr);
  cairo_destroy (grad_cr);
  cairo_pattern_destroy (linpat);

  menu->bg_sel_image = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
        default_item_width, default_item_height);

  linpat = cairo_pattern_create_linear (default_item_width, 0,
      default_item_width, default_item_height);
  cairo_pattern_add_color_stop_rgb (linpat, 0.0, 0.0, 0.8, 0.3);
  cairo_pattern_add_color_stop_rgb (linpat, 0.3, 0.0, 0.6, 0.3);
  cairo_pattern_add_color_stop_rgb (linpat, 0.7, 0.0, 0.7, 0.3);
  cairo_pattern_add_color_stop_rgb (linpat, 1.0, 0.0, 0.8, 0.3);

  grad_cr = cairo_create (menu->bg_sel_image);
  cairo_set_source (grad_cr, linpat);
  cairo_paint (grad_cr);
  cairo_destroy (grad_cr);
  cairo_pattern_destroy (linpat);

  return menu;
}

int
ps3_menu_add_item (Ps3Menu *menu, cairo_surface_t *image,
    const char *text, int text_size)
{
  Ps3MenuItem *item = NULL;

  /* If the menu has a fixed and has no room left, return an error */
  if (menu->rows != -1 && menu->columns != -1 &&
      menu->nitems >= (menu->rows * menu->columns))
    return -1;

  menu->nitems++;
  menu->items = realloc (menu->items, menu->nitems * sizeof(Ps3MenuItem));

  item = &menu->items[menu->nitems - 1];
  memset (item, 0, sizeof(Ps3MenuItem));

  item->index = menu->nitems - 1;
  item->image = image;
  item->image_position = PS3_MENU_IMAGE_POSITION_LEFT;
  item->text = strdup (text);
  item->text_size = text_size;
  item->text_color = (Ps3MenuColor) {1.0, 1.0, 1.0, 1.0};
  item->alignment = PS3_MENU_ALIGN_MIDDLE_LEFT;
  item->wrap = PS3_MENU_TEXT_WRAP_TRUNCATE;
  item->draw_cb = _draw_item;
  item->draw_data = NULL;
  item->width = menu->default_item_width;
  item->height = menu->default_item_height;
  item->ipad_x = PS3_MENU_DEFAULT_IPAD_X;
  item->ipad_y = PS3_MENU_DEFAULT_IPAD_Y;
  item->bg_image = cairo_surface_reference (menu->bg_image);
  item->bg_sel_image = cairo_surface_reference (menu->bg_sel_image);

  return item->index;
}

int
ps3_menu_handle_input (Ps3Menu *menu, Ps3MenuInput input,
    Ps3MenuRectangle *bbox)
{
  int row, new_row, start_row, max_rows, max_visible_rows;
  int column, new_column, start_column, max_columns, max_visible_columns;
  int width, height;

  width = cairo_image_surface_get_width (menu->surface);
  height = cairo_image_surface_get_height (menu->surface);

  /* TODO: Actually walk the items and calculate the real value depending on
     individual item's width/height */
  max_visible_rows = height / (menu->default_item_height + (2 * menu->pad_y));
  max_visible_columns = width / (menu->default_item_width + (2 * menu->pad_x));

  /* Define which row/column the selection is in */
  if (menu->columns != -1) {
    row = menu->selection / menu->columns;
    column = menu->selection % menu->columns;
    start_row = menu->start_item / menu->columns;
    start_column = menu->start_item % menu->columns;

    if (menu->nitems < menu->columns)
      max_columns = menu->nitems;
    else
      max_columns = menu->columns;

    if (menu->nitems == 0)
      max_rows = 0;
    else
      max_rows = ((menu->nitems - 1) / menu->columns) + 1;
  } else {
    column = menu->selection / menu->rows;
    row = menu->selection % menu->rows;
    start_row = menu->start_item % menu->rows;
    start_column = menu->start_item / menu->rows;

    if (menu->nitems < menu->rows)
      max_rows = menu->nitems;
    else
      max_rows = menu->rows;

    if (menu->nitems == 0)
      max_columns = 0;
    else
      max_columns = ((menu->nitems - 1) / menu->rows) + 1;
  }

  switch(input) {
    case PS3_MENU_INPUT_UP:
      if (row > 0) {
        if (menu->columns != -1) {
          menu->selection -= menu->columns;
        } else {
          menu->selection--;
        }
      }
      break;
    case PS3_MENU_INPUT_DOWN:
      if (row < max_rows - 1) {
        if (menu->columns != -1) {
          if (menu->selection + menu->columns < menu->nitems)
          menu->selection += menu->columns;
        } else {
          if (menu->selection + 1 < menu->nitems)
            menu->selection++;
        }
      }
      break;
    case PS3_MENU_INPUT_LEFT:
      if (column > 0) {
        if (menu->rows != -1) {
          menu->selection -= menu->rows;
        } else {
          menu->selection--;
        }
      }
      break;
    case PS3_MENU_INPUT_RIGHT:
      if (column < max_columns - 1) {
        if (menu->rows != -1) {
          if (menu->selection + menu->rows < menu->nitems)
            menu->selection += menu->rows;
        } else {
          if (menu->selection + 1 < menu->nitems)
            menu->selection++;
        }
      }
      break;
  }

  if (menu->columns != -1) {
    new_row = menu->selection / menu->columns;
    new_column = menu->selection % menu->columns;
  } else {
    new_column = menu->selection / menu->rows;
    new_row = menu->selection % menu->rows;
  }

  if (new_row < start_row || new_column < start_column) {
    /* We go left/up back to a non-displayed item */
    if (menu->columns != -1) {
      if (input == PS3_MENU_INPUT_LEFT)
        menu->start_item -= 1;
      else
        menu->start_item -= menu->columns;
    } else {
      if (input == PS3_MENU_INPUT_LEFT)
        menu->start_item -= menu->rows;
      else
        menu->start_item -= 1;
    }
    ps3_menu_redraw (menu);

    bbox->x = 0;
    bbox->y = 0;
    bbox->width = cairo_image_surface_get_width (menu->surface);
    bbox->height = cairo_image_surface_get_height (menu->surface);
  } else if (((new_row - start_row) >= max_visible_rows) ||
      ((new_column - start_column) >= max_visible_columns)) {
    /* We go right/down to a hidden item */
    if (menu->columns != -1) {
      /* If we go on the right, we only need to shift by one column.
         If we go down, we need to shift by own row */
      if (input == PS3_MENU_INPUT_RIGHT)
        menu->start_item += 1;
      else
        menu->start_item += menu->columns;
    } else {
      if (input == PS3_MENU_INPUT_RIGHT)
        menu->start_item += menu->rows;
      else
        menu->start_item += 1;
    }
    ps3_menu_redraw (menu);

    bbox->x = 0;
    bbox->y = 0;
    bbox->width = cairo_image_surface_get_width (menu->surface);
    bbox->height = cairo_image_surface_get_height (menu->surface);
  } else {
    /* TODO: Only redraw the part that we need, and set the right bbox */
    ps3_menu_redraw (menu);

    bbox->x = 0;
    bbox->y = 0;
    bbox->width = cairo_image_surface_get_width (menu->surface);
    bbox->height = cairo_image_surface_get_height (menu->surface);
  }

  return menu->selection;
}

void
ps3_menu_set_selection (Ps3Menu *menu, int id, Ps3MenuRectangle *bbox)
{
  /* TODO: Implement this some day... */
  assert ("Not implemented" == NULL);
}

void
ps3_menu_redraw (Ps3Menu *menu)
{
  int i;
  cairo_t *cr;
  int width, height;
  int x = 0;
  int y = 0;
  int row, start_row;
  int column, start_column;

  cr = cairo_create (menu->surface);

  /* Clear the whole surface before redrawing */
  cairo_save (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
  cairo_paint (cr);
  cairo_restore (cr);

  width = cairo_image_surface_get_width (menu->surface);
  height = cairo_image_surface_get_height (menu->surface);

  /* Define which row/column we're drawing if we're scrolled */
  if (menu->columns != -1) {
    row = menu->start_item / menu->columns;
    column = menu->start_item % menu->columns;
  } else {
    row = menu->start_item % menu->rows;
    column = menu->start_item / menu->rows;
  }
  start_row = row;
  start_column = column;

  for (i = menu->start_item; i < menu->nitems;) {
    Ps3MenuItem *item = &menu->items[i];

    x += menu->pad_x;
    y += menu->pad_y;
    cairo_rectangle (cr, x, y, item->width, item->height);
    cairo_clip (cr);

    /* No need to draw the items that are outside the visible area */
    if (x < width && y < height) {
      item->draw_cb (menu, item, (menu->selection == item->index), cr,
          x, y, item->draw_data);
    }

    /* Move to the next item position */
    if (menu->columns != -1) {
      /* Filling the rows from left to right */
      column++;
      if (column < menu->columns) {
        x += item->width + menu->pad_x;
        y -= menu->pad_y;
      } else {
        row++;
        column = start_column;
        x = 0;
        y += item->height + menu->pad_y;
      }
      i = (menu->columns * row) + column;
    } else {
      /* Filling the columns from top to bottom */
      row++;
      if (row < menu->rows) {
        x -= menu->pad_x;
        y += item->height + menu->pad_y;
      } else {
        column++;
        row = start_row;
        x += item->width + menu->pad_x;
        y = 0;
      }
      i = (menu->rows * column) + row;
    }

    cairo_reset_clip (cr);
  }
  cairo_destroy (cr);
}

cairo_surface_t *
ps3_menu_get_surface (Ps3Menu *menu)
{
  return cairo_surface_reference (menu->surface);
}

void
ps3_menu_free (Ps3Menu *menu)
{
  int i;

  if (menu->items) {
    for (i = 0; i < menu->nitems; i++) {
      Ps3MenuItem *item = &menu->items[i];
      if (item->image)
        cairo_surface_destroy (item->image);
      free (item->text);
      if (item->bg_image)
        cairo_surface_destroy (item->bg_image);
      if (item->bg_sel_image)
        cairo_surface_destroy (item->bg_sel_image);
    }
    free (menu->items);
  }

  cairo_surface_destroy (menu->surface);
  if (menu->bg_image)
    cairo_surface_destroy (menu->bg_image);
  if (menu->bg_sel_image)
    cairo_surface_destroy (menu->bg_sel_image);
  free (menu);
}
