#define F_CPU 1000000UL
#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "hd44780.h"
#include <util/delay.h>
#define LINE_LENGTH 16
#define lcd_go_to_first_line(); lcd_goto(0);
#define lcd_go_to_second_line(); lcd_goto(0x40);
#define TEXT_CHUNK_SIZE LINE_LENGTH
#define activate_interrupts(); sei();
#define SCREEN_DISPLAY_TIME 3
#define MODE_SIMPLE 0
#define MODE_FREQUENCY 1
#define MODE 1
#define BUTTON 0
#define EMG 1
#define CONTROLLER 1
#define EMG_THERESHOLD 900

/*
// for debug purposes only
char * str(uint16_t number)
{
    uint16_t n = 0;
    for(uint16_t number_copy = number; number_copy; number_copy /= 10)
    {
        n++;
    }

    if(!number)
        n = 1;

    char *numbers = calloc(n + 1, sizeof(char));
    for(uint16_t i = n; i != 0; i--, number /= 10)
    {
        numbers[i - 1] = (number % 10) + 48;
    }
	numbers[n] = '\0';
    return numbers;
}

uint16_t strlen(const char *string)
{
    register char c;
    register uint16_t i = 0;
    while((c=*string++))
        i++;
    return i;
}

inline void reset_chars(char **text, uint8_t length)
{
    while((length--))
    {
        *text[length] = '\0';
    }
}
*/

void _delay_s(uint16_t seconds)
{
	while(seconds--)
	{
		_delay_ms(1000);
	}
}

#define set_high(port, pin) ((port) |= (1 << pin))
#define set_low(port, pin) ((port) &= !(1 << pin))
#define set_out(ddr, pin) ((ddr) |= (1 << pin))

// skip the first cutom character: sorry but it cannot be printed conviniently
uint8_t custom_characters_count = 1;


uint8_t define_character(char *character_byte_lines, uint8_t identificator)
{
	/*
		Returns: character identificator

		You can use it inside char/string typing ascii number (escaped literal):
		print("My first custom character: \x01")
	*/
	lcd_command(_BV(LCD_CGRAM) + identificator * 8);
	for(uint8_t i = 0; i < 8; i++)
	{
		lcd_putc(*character_byte_lines);
		character_byte_lines += 1;
	}
	lcd_goto(0);
	return identificator;
}


uint8_t register_character(char *character_byte_lines)
{
	define_character(character_byte_lines, custom_characters_count);
	custom_characters_count += 1;
	return custom_characters_count - 1;
}

static volatile uint8_t button_pressed = 0;

#if (CONTROLLER==BUTTON)
    ISR(TIMER2_OVF_vect)
    {
    	if((PINB & (1 << PB0)) == 1)
    		button_pressed = 1;
    }
#endif

void print(const char *s)
{
	/* max length: 255 */
	lcd_clrscr();

	register char c = *s++;
	uint8_t char_count = 0;

	for(char_count = 0; c; c = *s++)
	{
		// skip printing long message if button presed
		if(button_pressed)
		{
			button_pressed = 0;
			return;
		}
		if(char_count % LINE_LENGTH == 0)
		{
			// we will start new line soon
			// let's trim leftmost spaces
			if(c == ' ')
			{
				continue;
			}
			if(char_count != 0)
			{
				if(char_count % (2 * LINE_LENGTH) == 0)
				{
					// reached end of second line,
					// give user time to read and go to first line
					_delay_s(SCREEN_DISPLAY_TIME);
					lcd_clrscr();
					lcd_go_to_first_line();
				}
				else
				{
					// we were in first line
					lcd_go_to_second_line();
				}
			}

		}
		lcd_putc(c);
		char_count += 1;
	}
	_delay_s(SCREEN_DISPLAY_TIME);
}


typedef struct Screen Screen, *ptr;
struct Screen
{
	uint8_t size;
	uint8_t show_separators;
	char (*choice_handler)(Screen *this, uint8_t choice); /* Returns char to be displayed or 0 */
	char *choices[];
};

Screen *active_screen;
Screen typing_screen;

char minor_screen_choice_handler(Screen *this, uint8_t choice)
{
	if(choice == this->size - 2)
	{
		return '\b';
	}
	if(choice == this->size - 1)
	{
		active_screen = &typing_screen;
		return 0;
	}
	else
	{
		return *(this->choices[choice]);
	}
}

Screen ai_screen = {
	.size = 11,
	.show_separators = 0,
	.choice_handler = &minor_screen_choice_handler,
    #if (MODE == MODE_SIMPLE)
	.choices = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "\x04", "\x03"}
    #else
	.choices = {"e", "a", "i", "h", "d", "c", "f", "g", "b", "\x04", "\x03"}
    #endif
};

Screen jr_screen = {
	.size = 11,
	.show_separators = 0,
	.choice_handler = &minor_screen_choice_handler,
    #if (MODE == MODE_SIMPLE)
	.choices = {"j", "k", "l", "m", "n", "o", "p", "q", "r", "\x04", "\x03"}
    #else
    .choices = {"o", "n", "r", "l", "m", "p", "k", "j", "q", "\x04", "\x03"}
    #endif
};

Screen sz_screen = {
	.size = 10,
	.show_separators = 0,
	.choice_handler = &minor_screen_choice_handler,
    #if (MODE == MODE_SIMPLE)
	.choices = {"s", "t", "u", "v", "w", "x", "y", "z", "\x04", "\x03"}
    #else
    .choices = {"t", "s", "u", "w", "y", "v", "x", "z", "\x04", "\x03"}
    #endif
};

Screen dot_screen = {
	.size = 12,
	.show_separators = 0,
	.choice_handler = &minor_screen_choice_handler,
	.choices = {".", "'", ",", "?", "!", "+", "-", "=", "\x01", "\x02", "\x04", "\x03"}
};


char typing_screen_choice_handler(Screen *this, uint8_t choice)
{
	switch(choice)
	{
		case 0:
			return ' ';
			break;
		case 1:
			active_screen = &ai_screen;
			break;
		case 2:
			active_screen = &jr_screen;
			break;
		case 3:
			active_screen = &sz_screen;
			break;
		case 4:
			active_screen = &dot_screen;
			break;
	}
	return 0;
}

Screen typing_screen = {
	.size = 5,
	.show_separators = 1,
	.choice_handler = &typing_screen_choice_handler,
	.choices = {"_", "a-i", "j-r", "s-z", "."}
};


void display(uint8_t cursor, const char *text[16])
{
	uint8_t highlight_on = 0;
	lcd_clrscr();
	uint8_t element = 0;
	// write upper choice options, determined by active screen
	for(uint8_t i = 0; i < active_screen->size; i++)
	{
		const char *s = active_screen->choices[i];
		if(i == cursor)
		{
			highlight_on = element + strlen(s) / 2;
			register char c;
			while((c=*s++))
			{
				lcd_putc(c);
			}
		}
		else
		{
			element += strlen(s);
			lcd_puts(s);
		}
		if((*active_screen).show_separators)
		{
			lcd_putc('|');
			element += 1;
		}
	}
	// write lower choice options or current text
	lcd_go_to_second_line();
	lcd_puts(text);

	lcd_goto(highlight_on);
}

uint16_t adc(uint8_t channel)
{
    // if I use the same ref all the time, I can move it to init
    ADMUX = 1 << REFS0 | (channel & 0x7); //  ref is AVCC; set channel

    ADCSRA |= (1 << ADSC);

   //Wait for conversion to complete
   while(!(ADCSRA & (1<<ADIF)));

   //Clear ADIF by writing one to it
   ADCSRA|=(1<<ADIF);

   return(ADC);

}

uint16_t adc_aref(uint8_t channel)
{
    // if I use the same ref all the time, I can move it to init
    ADMUX = 1 << REFS1 | (channel & 0x7); //  ref is AREF; set channel

    ADCSRA |= (1 << ADSC);

   //Wait for conversion to complete
   while(!(ADCSRA & (1<<ADIF)));

   //Clear ADIF by writing one to it
   ADCSRA|=(1<<ADIF);

   return(ADC);

}

int main()
{
	// configure led output
	set_out(DDRD, PD6);
	set_high(PORTD, PD6);

    #if (CONTROLLER==BUTTON)
    	// configure timer interrupts
    	TCCR2 = 1 << CS22 | 1 << CS21 | 1 << CS20;
    	TIMSK |= 1 << TOIE2;
    	activate_interrupts();
    #endif

    // enable ADC, preskaler: F_CPU / 128
    ADCSRA = 1 << ADEN | 1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0;

	// init lcd display
	lcd_init();
	lcd_command(_BV(LCD_DISPLAYMODE) | _BV(LCD_DISPLAYMODE_ON) | _BV(LCD_DISPLAYMODE_CURSOR));

	register_character((char []){0b00000, 0b01010, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000, 0b00000}); // heart
	register_character((char []){0b00000, 0b00000, 0b01010, 0b00000, 0b10001, 0b01110, 0b00000, 0b00000}); // smile
	register_character((char []){0b00000, 0b00000, 0b00100, 0b01110, 0b11111, 0b00000, 0b00000, 0b00000}); // go up
	register_character((char []){0b00000, 0b00010, 0b00110, 0b01110, 0b00110, 0b00010, 0b00000, 0b00000}); // go left

	char typed_text[TEXT_CHUNK_SIZE] = "";
	uint8_t cursor = 0;

	active_screen = &typing_screen;

	// initalization ended, turn of the led
	set_low(PORTD, PD6);

    print("Consecutive slices of the alphabet will be underlined one by one");
    print("Choose one and then a letter you want to type.");
    print("To remove a letter choose '\x04'. To go back, choose '\x03'.");

    register uint16_t factor;

    while(1)
    {
        #if (CONTROLLER==EMG)
            // get an avarage from 10 measurements
            uint16_t emg = 0;
            for(uint8_t i = 0; i < 10; i++)
                emg += adc(4) / 10;

            // set flag
            if(emg > EMG_THERESHOLD)
                button_pressed = 1;
        #endif

		if(button_pressed)
		{

			int8_t length = strlen(typed_text);
			if(length >= TEXT_CHUNK_SIZE)
			{
                //reset_chars(&typed_text, TEXT_CHUNK_SIZE);
				memset(typed_text, 0, TEXT_CHUNK_SIZE);
			}

			char result = active_screen->choice_handler(active_screen, cursor);

            // a letter was typed
			if(result)
			{
				if(result == '\b')
				{
					typed_text[length - 1] = '\0';
				}
				else if(result == ' ')
				{
					typed_text[length] = '_';
				}
				else
				{
					typed_text[length] = result;
				}
			}
            // the screen was  changed
            else
            {
    			cursor = 0;
            }

			button_pressed = 0;
		}

		if(++cursor > active_screen->size)
		{
			cursor = 0;
		}

        factor = adc(0);

        _delay_ms(200);
		display(cursor, &typed_text);
		_delay_s(10 * factor / 1023);
    }
}
