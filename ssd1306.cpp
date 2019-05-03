
#include"ssd1306.hpp"


//
//  Send a byte to the command register
//
void SSD1306_OLED::WriteCommand(uint8_t command)
{
	//HAL_I2C_Mem_Write(hi2c,SSD1306_I2C_ADDR,0x00,1,&command,1,10);
	uint8_t data[2];
	data[0] = 0x80;
	data[1] = command;
	HAL_I2C_Master_Transmit(hi2c,SSD1306_I2C_ADDR,data,2,10);
}

void SSD1306_OLED::WriteData(uint8_t _Data)
{
	uint8_t data[2];
	data[0] = 0x40;
	data[1] = _Data;
	HAL_I2C_Master_Transmit(hi2c,SSD1306_I2C_ADDR,data,2,10);
}
	
void SSD1306_OLED::DrawBitmap(uint8_t *_Buffer)
{		
	uint8_t i;
	
	for (i = 0; i < 8; i++) 
	{
		WriteCommand(0xB0 + i);
		WriteCommand(0x00);
		WriteCommand(0x10);

		HAL_I2C_Mem_Write(hi2c,SSD1306_I2C_ADDR,0x40,1,&_Buffer[SSD1306_WIDTH * i],SSD1306_WIDTH,100);
	}
}

//
//	Initialize the oled screen
//
uint8_t SSD1306_OLED::Init(I2C_HandleTypeDef *_hi2c)
{	
	// Wait for the screen to boot
	hi2c = _hi2c;
	osDelay(100);
	
	/* Init LCD */
	WriteCommand(0xAE); //display off
	WriteCommand(0x20); //Set Memory Addressing Mode   
	WriteCommand(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCommand(0xB0); //Set Page Start Address for Page Addressing Mode,0-7
	WriteCommand(0xC8); //Set COM Output Scan Direction
	WriteCommand(0x00); //---set low column address
	WriteCommand(0x10); //---set high column address
	WriteCommand(0x40); //--set start line address
	WriteCommand(0x81); //--set contrast control register
	WriteCommand(0xFF);
	WriteCommand(0xA1); //--set segment re-map 0 to 127
	WriteCommand(0xA6); //--set normal display
	WriteCommand(0xA8); //--set multiplex ratio(1 to 64)
	WriteCommand(0x3F); //
	WriteCommand(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCommand(0xD3); //-set display offset
	WriteCommand(0x00); //-not offset
	WriteCommand(0xD5); //--set display clock divide ratio/oscillator frequency
	WriteCommand(0xF0); //--set divide ratio
	WriteCommand(0xD9); //--set pre-charge period
	WriteCommand(0x22); //
	WriteCommand(0xDA); //--set com pins hardware configuration
	WriteCommand(0x12);
	WriteCommand(0xDB); //--set vcomh
	WriteCommand(0x20); //0x20,0.77xVcc
	WriteCommand(0x8D); //--set DC-DC enable
	WriteCommand(0x14); //
	WriteCommand(0xAF); //--turn on SSD1306 panel
	
	// Clear screen
	Fill(Black);
	
	// Flush buffer to screen
	//UpdateScreen();
	
	// Set default values for screen object
	SSD1306.CurrentX = 0;
	SSD1306.CurrentY = 0;
	
	SSD1306.Initialized = 1;
	
	return 1;
}

//
//  Fill the whole screen with the given color
//
void SSD1306_OLED::Fill(SSD1306_COLOR color) 
{
	/* Set memory */
	uint32_t i;

	for(i = 0; i < sizeof(SSD1306_Buffer); i++)
	{
		//SSD1306_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
		WriteData((color == Black) ? 0x00 : 0xFF);
	}
}

//
//  Write the screenbuffer with changed to the screen
//
void SSD1306_OLED::UpdateScreen(void) 
{
	uint8_t i;
	
	for (i = 0; i < 8; i++) {
		WriteCommand(0xB0 + i);
		WriteCommand(0x00);
		WriteCommand(0x10);

		HAL_I2C_Mem_Write(hi2c,SSD1306_I2C_ADDR,0x40,1,&SSD1306_Buffer[SSD1306_WIDTH * i],SSD1306_WIDTH,100);
	}
}

//
//	Draw one pixel in the screenbuffer
//	X => X Coordinate
//	Y => Y Coordinate
//	color => Pixel color
//
void SSD1306_OLED::DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color)
{
	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) 
	{
		// Don't write outside the buffer
		return;
	}
	
	// Check if pixel should be inverted
	if (SSD1306.Inverted) 
	{
		color = (SSD1306_COLOR)!color;
	}
	
	// Draw in the right color
	if (color == White)
	{
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
	} 
	else 
	{
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}

//
//  Draw 1 char to the screen buffer
//	ch 		=> char om weg te schrijven
//	Font 	=> Font waarmee we gaan schrijven
//	color 	=> Black or White
//
char SSD1306_OLED::WriteChar(char ch, FontDef Font, SSD1306_COLOR color)
{
	uint32_t i, b, j;
	
	// Check remaining space on current line
	if (SSD1306_WIDTH <= (SSD1306.CurrentX + Font.FontWidth) ||
		SSD1306_HEIGHT <= (SSD1306.CurrentY + Font.FontHeight))
	{
		// Not enough space on current line
		return 0;
	}
	
	// Use the font to write
	for (i = 0; i < Font.FontHeight; i++)
	{
		b = Font.data[(ch - 32) * Font.FontHeight + i];
		for (j = 0; j < Font.FontWidth; j++)
		{
			if ((b << j) & 0x8000) 
			{
				DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR) color);
			} 
			else 
			{
				DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)!color);
			}
		}
	}
	
	// The current space is now taken
	SSD1306.CurrentX += Font.FontWidth;
	
	// Return written char for validation
	return ch;
}

//
//  Write full string to screenbuffer
//
char SSD1306_OLED::WriteString(char* str, FontDef Font, SSD1306_COLOR color)
{
	// Write until null-byte
	while (*str) 
	{
		if (WriteChar(*str, Font, color) != *str)
		{
			// Char could not be written
			return *str;
		}
		
		// Next char
		str++;
	}
	
	// Everything ok
	return *str;
}

//
//	Position the cursor
//
void SSD1306_OLED::SetCursor(uint8_t x, uint8_t y) 
{
	SSD1306.CurrentX = x;
	SSD1306.CurrentY = y;
}

void SSD1306_OLED::InvertDisplay(bool _Command) 
{
  if (_Command) {
    WriteCommand(SSD1306_INVERTDISPLAY);
  } else {
    WriteCommand(SSD1306_NORMALDISPLAY);
  }
}

void SSD1306_OLED::Startscrollright(uint8_t start, uint8_t stop)
{
  WriteCommand(SSD1306_RIGHT_HORIZONTAL_SCROLL);
  WriteCommand(0X00);
  WriteCommand(start);
  WriteCommand(0X00);
  WriteCommand(stop);
  WriteCommand(0X00);
  WriteCommand(0XFF);
  WriteCommand(SSD1306_ACTIVATE_SCROLL);
}

void SSD1306_OLED::Startscrollleft(uint8_t start, uint8_t stop)
{
  WriteCommand(SSD1306_LEFT_HORIZONTAL_SCROLL);
  WriteCommand(0X00);
  WriteCommand(start);
  WriteCommand(0X00);
  WriteCommand(stop);
  WriteCommand(0X00);
  WriteCommand(0XFF);
  WriteCommand(SSD1306_ACTIVATE_SCROLL);
}

void SSD1306_OLED::Startscrolldiagright(uint8_t start, uint8_t stop)
{
  WriteCommand(SSD1306_SET_VERTICAL_SCROLL_AREA);
  WriteCommand(0X00);
  WriteCommand(SSD1306_HEIGHT);
  WriteCommand(SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
  WriteCommand(0X00);
  WriteCommand(start);
  WriteCommand(0X00);
  WriteCommand(stop);
  WriteCommand(0X01);
  WriteCommand(SSD1306_ACTIVATE_SCROLL);
}

void SSD1306_OLED::Startscrolldiagleft(uint8_t start, uint8_t stop)
{
  WriteCommand(SSD1306_SET_VERTICAL_SCROLL_AREA);
  WriteCommand(0X00);
  WriteCommand(SSD1306_HEIGHT);
  WriteCommand(SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
  WriteCommand(0X00);
  WriteCommand(start);
  WriteCommand(0X00);
  WriteCommand(stop);
  WriteCommand(0X01);
  WriteCommand(SSD1306_ACTIVATE_SCROLL);
}

void SSD1306_OLED::Stopscroll(void)
{
  WriteCommand(SSD1306_DEACTIVATE_SCROLL);
}

void SSD1306_OLED::SetBrightness(uint8_t _brightness)
{
	WriteCommand(SSD1306_Set_Brightness_Cmd);
	WriteCommand(_brightness);
}
