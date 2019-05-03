#include "stm32f1xx_hal.h"
#include "fonts.h"
#include "cmsis_os.h"

/**
 * This Library is written and optimized by Olivier Van den Eede(4ilo) in 2016
 * for Stm32 Uc and HAL-i2c lib's.
 *
 * To use this library with ssd1306 oled display you will need to customize the defines below.
 *
 * This library uses 2 extra files (fonts.c/h).
 * In this files are 3 different fonts you can use:
 * 		- Font_7x10
 * 		- Font_11x18
 * 		- Font_16x26
 *
 */

#ifndef ssd1306
#define ssd1306

// I2c address
#define SSD1306_I2C_ADDR        0x78
// SSD1306 width in pixels
#define SSD1306_WIDTH           128
// SSD1306 LCD height in pixels
#define SSD1306_HEIGHT          64

#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7

#define SSD1306_ACTIVATE_SCROLL 0x2F
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A
#define SSD1306_Set_Brightness_Cmd    0x81


//
//  Enumeration for screen colors
//
typedef enum {
	Black = 0x00, // Black color, no pixel
	White = 0x01  //Pixel is set. Color depends on LCD
} SSD1306_COLOR;

//
//  Struct to store transformations
//
typedef struct {
	uint16_t CurrentX;
	uint16_t CurrentY;
	uint8_t Inverted;
	uint8_t Initialized;
} SSD1306_t;


class SSD1306_OLED
{
	public:
		uint8_t Init(I2C_HandleTypeDef *_hi2c);
		void Fill(SSD1306_COLOR color);
		void UpdateScreen(void);
		void DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color);
		char WriteChar(char ch, FontDef Font, SSD1306_COLOR color);
		char WriteString(char* str, FontDef Font, SSD1306_COLOR color);
		void SetCursor(uint8_t x, uint8_t y);
		void DrawBitmap(uint8_t *_Buffer);
		void InvertDisplay(bool _Command);
		void Startscrollright(uint8_t start, uint8_t stop);
		void Startscrollleft(uint8_t start, uint8_t stop);
		void Startscrolldiagright(uint8_t start, uint8_t stop);
		void Startscrolldiagleft(uint8_t start, uint8_t stop);
		void Stopscroll(void);
		void SetBrightness(uint8_t _brightness);
	private:
		void WriteCommand(uint8_t command);
		void WriteData(uint8_t Data);
	
		I2C_HandleTypeDef *hi2c;
		// Screenbuffer
		uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

		// Screen object
		SSD1306_t SSD1306;
};

#endif
