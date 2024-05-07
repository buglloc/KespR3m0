/*******************************************************************************
 * Size: 14 px
 * Bpp: 4
 * Opts: --bpp 4 --size 14 --no-compress --font materialdesignicons-webfont.woff --range 984489,983308,984887,984878 --format lvgl -o ui_font_icons.c
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif

#ifndef UI_FONT_ICONS
#define UI_FONT_ICONS 1
#endif

#if UI_FONT_ICONS

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+F010C "󰄌" */
    0x0, 0x5b, 0xbb, 0x0, 0x5b, 0xbb, 0x0, 0x0,
    0x8f, 0xff, 0x0, 0x7f, 0xff, 0x0, 0x3f, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xfb, 0x3f, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x8c, 0x3e, 0x0, 0x0, 0x0,
    0x9, 0x20, 0x6c, 0x3e, 0x6, 0x77, 0x40, 0x7f,
    0x93, 0x6c, 0x3e, 0xa, 0xbb, 0x60, 0xbf, 0xc4,
    0x6c, 0x3e, 0x0, 0x0, 0x0, 0xc, 0x30, 0x6c,
    0x3e, 0x0, 0x0, 0x0, 0x0, 0x0, 0x6c, 0x3f,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x3, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x32,

    /* U+F05A9 "󰖩" */
    0x0, 0x0, 0x47, 0x99, 0x74, 0x0, 0x0, 0x1,
    0x8f, 0xff, 0xff, 0xff, 0xe8, 0x10, 0x1e, 0xfe,
    0x84, 0x22, 0x48, 0xef, 0xe1, 0x9, 0x70, 0x0,
    0x0, 0x0, 0x8, 0x90, 0x0, 0x1, 0x8c, 0xff,
    0xc8, 0x10, 0x0, 0x0, 0xe, 0xff, 0xcc, 0xff,
    0xe0, 0x0, 0x0, 0x5, 0x60, 0x0, 0x6, 0x50,
    0x0, 0x0, 0x0, 0x2, 0x66, 0x20, 0x0, 0x0,
    0x0, 0x0, 0xd, 0xff, 0xd0, 0x0, 0x0, 0x0,
    0x0, 0x2, 0xff, 0x20, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x66, 0x0, 0x0, 0x0,

    /* U+F072E "󰜮" */
    0x0, 0xd, 0xee, 0x80, 0x0, 0x0, 0xe, 0xff,
    0x90, 0x0, 0x0, 0xe, 0xff, 0x90, 0x0, 0x0,
    0xe, 0xff, 0x90, 0x0, 0x24, 0x4f, 0xff, 0xa4,
    0x41, 0x1d, 0xff, 0xff, 0xff, 0xa0, 0x1, 0xdf,
    0xff, 0xfa, 0x0, 0x0, 0x1d, 0xff, 0xa0, 0x0,
    0x0, 0x1, 0xda, 0x0, 0x0, 0x0, 0x0, 0x10,
    0x0, 0x0,

    /* U+F0737 "󰜷" */
    0x0, 0x0, 0x73, 0x0, 0x0, 0x0, 0x7, 0xfe,
    0x30, 0x0, 0x0, 0x7f, 0xff, 0xe3, 0x0, 0x7,
    0xff, 0xff, 0xfe, 0x30, 0x4b, 0xcf, 0xff, 0xec,
    0xa1, 0x0, 0xe, 0xff, 0x90, 0x0, 0x0, 0xe,
    0xff, 0x90, 0x0, 0x0, 0xe, 0xff, 0x90, 0x0,
    0x0, 0xe, 0xff, 0x90, 0x0, 0x0, 0x6, 0x66,
    0x30, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 224, .box_w = 14, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 77, .adv_w = 224, .box_w = 14, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 224, .box_w = 10, .box_h = 10, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 204, .adv_w = 224, .box_w = 10, .box_h = 10, .ofs_x = 2, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x49d, 0x622, 0x62b
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 983308, .range_length = 1580, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 4, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 4,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t ui_font_icons = {
#else
lv_font_t ui_font_icons = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 11,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = 0,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if UI_FONT_ICONS*/
