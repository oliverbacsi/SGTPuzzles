/*
 * ps3menu.c : PS3 Menu drawing API
 *
 * Copyright (C) Youness Alaoui (KaKaRoTo)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#include <cairo/cairo.h>


/**
 * Ps3MenuTextAlignment:
 * @PS3_MENU_ALIGN_TOP_LEFT: Align the text to the top left corner of the menu
 * item
 * @PS3_MENU_ALIGN_TOP_CENTER: Align the text to the top and the center of the
 * menu item
 * @PS3_MENU_ALIGN_TOP_RIGHT: Align the text to the top right corner of the
 * menu item
 * @PS3_MENU_ALIGN_MIDDLE_LEFT: Align the text to the center vertically, and
 * aligned to the left horizontally in the menu item
 * @PS3_MENU_ALIGN_MIDDLE_CENTER:Align the text to the center vertically, and
 * aligned to the center horizontally in the menu item
 * @PS3_MENU_ALIGN_MIDDLE_RIGHT:Align the text to the center vertically, and
 * aligned to the right horizontally menu item
 * @PS3_MENU_ALIGN_BOTTOM_LEFT: Align the text to the bottom left corner of
 * the menu item
 * @PS3_MENU_ALIGN_BOTTOM_CENTER: Align the text to the bottom and the center
 * of the menu item
 * @PS3_MENU_ALIGN_BOTTOM_RIGHT: Align the text to the bottom right corner of
 * the menu item
 *
 * Define how the text should be aligned and where to position it in a menu
 * item's button
 */
typedef enum {
  PS3_MENU_ALIGN_TOP_LEFT,
  PS3_MENU_ALIGN_TOP_CENTER,
  PS3_MENU_ALIGN_TOP_RIGHT,
  PS3_MENU_ALIGN_MIDDLE_LEFT,
  PS3_MENU_ALIGN_MIDDLE_CENTER,
  PS3_MENU_ALIGN_MIDDLE_RIGHT,
  PS3_MENU_ALIGN_BOTTOM_LEFT,
  PS3_MENU_ALIGN_BOTTOM_CENTER,
  PS3_MENU_ALIGN_BOTTOM_RIGHT,
} Ps3MenuTextAlignment;

/**
 * Ps3MenuTextWrap:
 * @PS3_MENU_TEXT_WRAP_NONE: No wrapping, just draw the text and let the
 * clipping do its work
 * @PS3_MENU_TEXT_WRAP_WORD: Wrap the text on word boundaries
 * @PS3_MENU_TEXT_WRAP_TRUNCATE: If text is too long, truncate and add elipsis
 *
 * Defines how to wrap a text in a menu item, if the text is too long to fit.
 */
typedef enum {
  PS3_MENU_TEXT_WRAP_NONE,
  PS3_MENU_TEXT_WRAP_WORD,
  PS3_MENU_TEXT_WRAP_TRUNCATE,
} Ps3MenuTextWrap;

/**
 * Ps3MenuImagePosition:
 * @PS3_MENU_IMAGE_POSITION_LEFT: Position the image on the left of the button
 * @PS3_MENU_IMAGE_POSITION_RIGHT: Position the image on the right of the
 * button
 * @PS3_MENU_IMAGE_POSITION_TOP: Position the image on the top of the button
 * @PS3_MENU_IMAGE_POSITION_BOTTOM: Position the image on the bottom of the
 * button
 *
 * Defines the position of an image on a menu item. The image will be scaled
 * so that it's height fits the menu item's height (for left/right positions)
 * or so that it's width fits the menu item's width (for top/bottom positions)
 */
typedef enum {
  PS3_MENU_IMAGE_POSITION_LEFT,
  PS3_MENU_IMAGE_POSITION_RIGHT,
  PS3_MENU_IMAGE_POSITION_TOP,
  PS3_MENU_IMAGE_POSITION_BOTTOM,
} Ps3MenuImagePosition;

/**
 * Ps3MenuInput:
 * @PS3_MENU_INPUT_UP: Move the selection one row up
 * @PS3_MENU_INPUT_DOWN: Move the selection one row down
 * @PS3_MENU_INPUT_LEFT: Move the selection one column to the left
 * @PS3_MENU_INPUT_RIGHT: Move the selection one column to the right
 *
 * Defines the four possible input that the menu can handle for changing the
 * selection
 */
typedef enum {
  PS3_MENU_INPUT_UP,
  PS3_MENU_INPUT_DOWN,
  PS3_MENU_INPUT_LEFT,
  PS3_MENU_INPUT_RIGHT,
} Ps3MenuInput;

/**
 * Ps3MenuColor:
 * @red: The red value
 * @green: The green value
 * @blue: The blue value
 * @alpha: The opacity
 *
 * A structure representing a single color, with values between 0.0 and 1.0
 */
typedef struct {
  float red;
  float green;
  float blue;
  float alpha;
} Ps3MenuColor;

/**
 * Ps3MenuRectangle:
 * @x: the x position of the rectangle
 * @x: the y position of the rectangle
 * @x: the width of the rectangle
 * @x: the height of the rectangle
 *
 * A structure representing a rectangle
 */
typedef struct {
  int x;
  int y;
  int width;
  int height;
} Ps3MenuRectangle;


/**
 * PS3_MENU_DEFAULT_IPAD_X:
 *
 * Default horizontal internal padding of a menu's item
 */
#define PS3_MENU_DEFAULT_IPAD_X 5

/**
 * PS3_MENU_DEFAULT_IPAD_Y:
 *
 * Default vertical internal padding of a menu's item
 */
#define PS3_MENU_DEFAULT_IPAD_Y 2

/**
 * PS3_MENU_DEFAULT_PAD_X:
 *
 * Default horizontal padding of a menu's item
 */
#define PS3_MENU_DEFAULT_PAD_X 3

/**
 * PS3_MENU_DEFAULT_PAD_Y:
 *
 * Default vertical padding of a menu's item
 */
#define PS3_MENU_DEFAULT_PAD_Y 2

typedef struct _Ps3MenuItem Ps3MenuItem;
typedef struct _Ps3Menu Ps3Menu;

/**
 * Ps3MenuDrawItemCb:
 * @menu: The #Ps3Menu being drawn
 * @item: The #Ps3MenuItem being drawn
 * @selected: #TRUE if this item is currently selected, #FALSE otherwise
 * @cr: The Cairo context to draw into
 * @x: The X position where to draw in the Cairo surface.
 * @y: The Y position where to draw in the Cairo surface.
 * @user_data: Any user provided data for the callback
 *
 * This callback is called whenever the #Ps3Menu needs to draw a menu item.
 * The button's clipping area will be already set, and you need to draw on
 * the (@x, @y) coordinate, up to(@x + @item->width - @item->ipad_x,
 * @y + @item->height - @item->ipad_y).
 *
 */
typedef int (*Ps3MenuDrawItemCb) (Ps3Menu *menu, Ps3MenuItem *item,
    int selected, cairo_t *cr, int x, int y, void *user_data);

/**
 * Ps3MenuItem:
 * @index: The index of this item in the #Ps3Menu. DO NOT modify this value.
 * @image: An image to draw in the menu, or #NULL
 * @image_position: Where to place the image on the menu
 * @text: The text to show
 * @text_size: The font size of the text to show
 * @text_color: The color to use for the menu item's text
 * @alignment: The alignment of the text
 * @wrap: How to wrap the text if too long to fit
 * @draw_cb: The drawing callback when the item needs to be drawn
 * @draw_data: User data for the drawing callback
 * @width: Item width
 * @height: Item height
 * @ipad_x: Internal horizontal padding
 * @ipad_y: Internal vertical padding
 * @bg_image: Background image for non-selected item
 * @bg_sel_image: Background image for selected item
 *
 * A structure representing a menu item, each attribute can be configured by
 * modifying the structure.
 */
struct _Ps3MenuItem {
  int index;
  cairo_surface_t *image;
  Ps3MenuImagePosition image_position;
  char *text;
  int text_size;
  Ps3MenuColor text_color;
  Ps3MenuTextAlignment alignment;
  Ps3MenuTextWrap wrap;
  Ps3MenuDrawItemCb draw_cb;
  void *draw_data;
  int width;
  int height;
  int ipad_x;
  int ipad_y;
  cairo_surface_t *bg_image;
  cairo_surface_t *bg_sel_image;
};

/**
 * Ps3Menu:
 * @surface: The surface where the menu gets drawn
 * @rows: Total rows to show in the menu
 * @columns: Total columns in the menu
 * @default_item_width: The default width of each menu item
 * @default_item_height: The default height of each menu item
 * @nitems: Number of items in the menu
 * @items: The items in the menu
 * @selection: Currently selected item index
 * @pad_x: The horizontal padding between items
 * @pad_y: The vertical padding between items
 * @bg_image: The default background image for non selected items
 * @bg_sel_image: The default background image for selected items
 * @start_item: The first item to be drawn (!= 0 if scrolled)
 */
struct _Ps3Menu {
  cairo_surface_t *surface;
  int rows;
  int columns;
  int default_item_width;
  int default_item_height;
  int nitems;
  Ps3MenuItem *items;
  int selection;
  int pad_x;
  int pad_y;
  cairo_surface_t *bg_image;
  cairo_surface_t *bg_sel_image;
  /* Private */
  int start_item;
};

/**
 * ps3_menu_new:
 * @surface: The surface associated with the menu, where it will get drawn. This
 * surface will be referenced so if you do not want to hold a reference to it,
 * then call cairo_surface_destroy() on it
 * @rows: The number of rows to show in the menu, or -1 for infinite
 * @columns: The number of columns to show in the menu, or -1 for infinite
 * @default_item_width: The default width of each menu item
 * @default_item_height: The default height of each menu item
 *
 * Create a new #Ps3Menu with default settings and configured to show a
 * specific number of rows or columns.
 * If a row or column is set to -1 (infinite), then the menu will be scrollable
 * in that direction.
 * Both @rows and @columns cannot be set to -1 at the same time or an error is
 * returned.
 *
 <note>
   <para>
   An item's width and height can always be modified after adding the item to
   the menu, however, if the width and height of each item is different and the
   menu has more than one row or column, then the behavior is undetermined.
   It is best to use the same width/height for all the items in the menu.
   </para>
 </note>
 *
 * Returns: the newly created #Ps3Menu or #NULL in case of an error.
 */
Ps3Menu *ps3_menu_new (cairo_surface_t *surface, int rows, int columns,
    int default_item_width, int default_item_height);

/**
 * ps3_menu_add_item:
 * @menu: The #Ps3Menu to add the item to
 * @image: An image to left align to the item or #NULL
 * @text: The middle-left aligned text to show in the item
 * @text_size: the size of the font for showing the text, or -1 for automatic
 *
 * Append a new item to the @menu.
 * If the number of rows is infinite or if both the number of rows and columns
 * are finite, the items are populated from left to right on each row,
 * then subsequent rows are filled.
 * If the number of columns is infinite, the items are populated from top to
 * bottom on each column, then subsequent columns are filled.
 *
 * Returns: A unique identifier (the item's index) representing this item or
 * -1 in case of an error (no room left).
 */
int ps3_menu_add_item (Ps3Menu *menu, cairo_surface_t *image,
    const char *text, int text_size);

/**
 * ps3_menu_handle_input:
 * @menu: The menu to change its selection
 * @input: The input received by the Pad
 * @bbox: The dirty rectangle of the surface
 *
 * Handle a controller input, causing a possible redraw of the
 * surface. If a drawing operation is needed, the @bbox rectangle will be
 * updated with the area that is marked dirty.
 *
 * Returns: The id of the currently selected menu item
 */
int ps3_menu_handle_input (Ps3Menu *menu, Ps3MenuInput input,
    Ps3MenuRectangle *bbox);

/**
 * ps3_menu_set_selection:
 * @menu: The menu to change its selection
 * @id: The id (as returned by ps3_menu_add_item()) to set as selected
 * @bbox: The dirty rectangle of the surface
 *
 * Changes the current selection of the menu, causing a possible redraw of the
 * surface. If a drawing operation is needed, the @bbox rectangle will be
 * updated with the area that is marked dirty
 */
void ps3_menu_set_selection (Ps3Menu *menu, int id, Ps3MenuRectangle *bbox);

/**
 * ps3_menu_redraw:
 * @menu: The menu to draw
 *
 * Force a redraw of the entire menu on the entire surface given.
 */
void ps3_menu_redraw (Ps3Menu *menu);

/**
 * ps3_menu_get_surface:
 * @menu: The menu
 *
 * Get the Cairo surface where the menu is drawn. This increases the reference
 * count of the surface, so you must call cairo_surface_destroy() once you are
 * done using it.
 *
 * Returns: The Cairo surface where the menu is drawn with its reference count
 * incremented.
 */
cairo_surface_t *ps3_menu_get_surface (Ps3Menu *menu);

/**
 * ps3_menu_free:
 * @menu: The menu to free
 *
 * Frees and destroys any memory used by the menu.
 * If any variable was modified in the #Ps3Menu or one of its #Ps3MenuItem then
 * it will be freed, so make sure that its reference count has been increased or
 * that it is not used afterwards.
 */
void ps3_menu_free (Ps3Menu *menu);