/*
 * This config file is for the SDL application
 */

/* We don't run on hardware directly */
#define CONFIG_PLATFORM (PLATFORM_NATIVE)

/* For Rolo and boot loader */
#define MODEL_NUMBER 100

#define MODEL_NAME   "Rockbox"

/* define this if you have RTC RAM available for settings */
//#define HAVE_RTC_RAM

/* define this if you have a real-time clock */


/* The number of bytes reserved for loadable codecs */
#define CODEC_SIZE 0x100

/* The number of bytes reserved for loadable plugins */
#define PLUGIN_BUFFER_SIZE 0x100

#define AB_REPEAT_ENABLE

/* Define this if you do software codec */
#define CONFIG_CODEC SWCODEC

#define HAVE_SW_TONE_CONTROLS 

/* Define this to the CPU frequency */
/*
#define CPU_FREQ 48000000
*/

/* Offset ( in the firmware file's header ) to the file CRC */
#define FIRMWARE_OFFSET_FILE_CRC 0

/* Offset ( in the firmware file's header ) to the real data */
#define FIRMWARE_OFFSET_FILE_DATA 8


