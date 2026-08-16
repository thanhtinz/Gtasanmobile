/**
 * Palette.
 *
 * COLOR_* values go to SendClientMessage and friends (RGBA).
 * EMBED_* values are the inline `{RRGGBB}` codes for mid-string colouring.
 */

#if defined _gtasan_colors_included
	#endinput
#endif
#define _gtasan_colors_included

#define COLOR_WHITE             0xFFFFFFFF
#define COLOR_GREY              0xAFAFAFFF
#define COLOR_DARKGREY          0x6E6E6EFF
#define COLOR_BLACK             0x000000FF

#define COLOR_ERROR             0xE05C4AFF
#define COLOR_SUCCESS           0x6FCF57FF
#define COLOR_INFO              0x62B3E0FF
#define COLOR_WARNING           0xE8B33AFF

#define COLOR_MONEY             0x86C24BFF
#define COLOR_LEVEL             0xE0A93CFF
#define COLOR_JOB               0x4FB3D9FF
#define COLOR_ADMIN             0xD46FD4FF
#define COLOR_CREW              0xC98A3AFF
#define COLOR_ANNOUNCE          0xF2C14EFF
#define COLOR_CRIME             0x8E44ADFF

// Faction markers
#define COLOR_CIVILIAN          0xCFCFCFFF
#define COLOR_POLICE            0x3F7FD6FF
#define COLOR_MEDIC             0xD65050FF

#define EMBED_WHITE             "{FFFFFF}"
#define EMBED_GREY              "{AFAFAF}"
#define EMBED_ERROR             "{E05C4A}"
#define EMBED_SUCCESS           "{6FCF57}"
#define EMBED_INFO              "{62B3E0}"
#define EMBED_MONEY             "{86C24B}"
#define EMBED_LEVEL             "{E0A93C}"
#define EMBED_JOB               "{4FB3D9}"
#define EMBED_CRIME             "{8E44AD}"
