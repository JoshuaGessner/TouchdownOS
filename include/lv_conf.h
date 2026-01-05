/**
 * @file lv_conf.h
 * LVGL configuration for TouchdownOS
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/

/* Color depth: 16 (RGB565) for best performance on embedded displays */
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

/*=========================
   MEMORY SETTINGS
 *=========================*/

/* Memory pool size for LVGL's internal management */
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (128U * 1024U)  /* 128KB */

/* Use LVGL's built-in memory manager */
#define LV_MEM_ADR 0

/* Memory pool alignment */
#define LV_MEM_BUF_MAX_NUM 16

/*=========================
   HAL SETTINGS
 *=========================*/

/* DPI for proper sizing calculations (round 1.28" display) */
#define LV_DPI_DEF 240

/* Display resolution */
#define LV_HOR_RES_MAX 240
#define LV_VER_RES_MAX 240

/* Rendering configuration */
#define LV_DISP_DEF_REFR_PERIOD 33  /* 30 FPS */

/* Input device settings */
#define LV_INDEV_DEF_READ_PERIOD 30  /* 33 FPS input polling */

/*=========================
   FEATURE CONFIGURATION
 *=========================*/

/* Enable animations for smooth UI */
#define LV_USE_ANIMATION 1

/* Enable shadows for depth */
#define LV_USE_SHADOW 1
#define LV_SHADOW_CACHE_SIZE 0

/* Enable group for input device navigation */
#define LV_USE_GROUP 1

/* Enable file system (for loading assets) */
#define LV_USE_FS_POSIX 1
#define LV_FS_POSIX_LETTER 'A'
#define LV_FS_POSIX_PATH "/"

/* Disable unnecessary features */
#define LV_USE_GPU_ARM2D 0
#define LV_USE_GPU_STM32_DMA2D 0

/* Disable NEON/assembly optimizations to avoid cross-compile issues */
#define LV_USE_DRAW_SW_ASM LV_DRAW_SW_ASM_NONE

/* Enable draw complex for better gradients and effects */
#define LV_DRAW_COMPLEX 1

/*==================
 * FONT USAGE
 *==================*/

/* Enable built-in fonts */
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_28 1

/* Default font */
#define LV_FONT_DEFAULT &lv_font_montserrat_16

/*===================
 * THEME USAGE
 *==================*/

/* Enable themes */
#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 1

/*==================
 * WIDGETS
 *==================*/

/* Enable necessary widgets */
#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_BTN 1
#define LV_USE_BTNMATRIX 1
#define LV_USE_CANVAS 1
#define LV_USE_CHECKBOX 1
#define LV_USE_DROPDOWN 1
#define LV_USE_IMG 1
#define LV_USE_LABEL 1
#define LV_USE_LINE 1
#define LV_USE_LIST 1
#define LV_USE_ROLLER 1
#define LV_USE_SLIDER 1
#define LV_USE_SWITCH 1
#define LV_USE_TEXTAREA 1
#define LV_USE_TABLE 0

/*==================
 * LAYOUTS
 *==================*/

#define LV_USE_FLEX 1
#define LV_USE_GRID 1

/*===================
 * LOGGING
 *==================*/

#ifdef DEBUG_BUILD
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
#define LV_LOG_PRINTF 1
#else
#define LV_USE_LOG 0
#endif

/*===================
 * DEBUGGING
 *==================*/

#ifdef DEBUG_BUILD
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1
#define LV_USE_ASSERT_STYLE 1
#define LV_USE_ASSERT_MEM_INTEGRITY 1
#define LV_USE_ASSERT_OBJ 1
#else
#define LV_USE_ASSERT_NULL 0
#define LV_USE_ASSERT_MALLOC 0
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ 0
#endif

#endif /* LV_CONF_H */
