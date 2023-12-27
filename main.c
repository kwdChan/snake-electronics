
#define F_CPU 8000000UL 

#include <avr/io.h>
#include <util/delay.h>


#define set_bit(var, bit) (var |= _BV(bit))
#define clear_bit(var, bit) (var &= ~_BV(bit))
#define is_bit_set(var, bit) ((var & _BV(bit)) != 0)


#define  NCOL 8
#define  NROW 8

struct RegisterBit{
    volatile uint8_t *reg_addr;
    uint8_t bit;
};

const struct RegisterBit ROW_PIN[NROW] = {{&PORTA, 1}, {&PORTA, 2}, {&PORTA, 3}, {&PORTA, 4}, {&PORTC, 6}, {&PORTC, 1}, {&PORTC, 0}, {&PORTD, 7} };
const struct RegisterBit COLUMN_PIN[NCOL] = {{&PORTB, 0}, {&PORTB, 1}, {&PORTB, 2}, {&PORTB, 3}, {&PORTB, 4}, {&PORTB, 5}, {&PORTB, 6}, {&PORTB, 7} };

uint8_t lit[NROW][NCOL];

void blink(uint8_t r, uint8_t c){
    struct RegisterBit row = ROW_PIN[r];
    struct RegisterBit col = COLUMN_PIN[c];

    clear_bit(*row.reg_addr, row.bit);
    set_bit(*col.reg_addr, col.bit);

    _delay_us(10);

    set_bit(*row.reg_addr, row.bit);
    clear_bit(*col.reg_addr, col.bit);    
}

void init_lit(void){
    uint8_t r, c;
    for (r=0; r<NROW; r++){
        for (c=0; c<NCOL; c++){
            lit[r][c] = 0;
        }
    }
}

void all_lit(void){
    uint8_t r, c;
    for (r=0; r<NROW; r++){
        for (c=0; c<NCOL; c++){
            lit[r][c] = 1;
        }
    }
}



void display(void){

    uint8_t r, c;
    for (r=0; r<NROW; r++){
        for (c=0; c<NCOL; c++){
            if (lit[r][c]){
                blink(r, c); 
            } 
        }
    }
}


int main(void){
    init_lit();
    all_lit();
    DDRA = 0xFF; 
    DDRB = 0xFF; 
    DDRC = 0xFF; 
    DDRD = 0xFF; 

    
    while (1)
    {
        display();

    }
    
}