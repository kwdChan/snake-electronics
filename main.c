
#define F_CPU 16000000UL 

#include <avr/io.h>
#include <util/delay.h>


#define set_bit(var, bit) (var |= _BV(bit))
#define clear_bit(var, bit) (var &= ~_BV(bit))
#define is_bit_set(var, bit) ((var & _BV(bit)) != 0)


#define  NCOL 8
#define  NROW 8

#define BLINK_DURATION_US 0

struct RegisterBit{
    volatile uint8_t* reg_addr;
    uint8_t bit;
};

struct Coordinate{
    uint8_t x;
    uint8_t y;
};

struct State{
    struct Coordinate snake[NROW*NCOL];
    struct Coordinate food_loc;
    uint8_t is_left_pressed; 
    uint8_t is_right_pressed; 
}; 


struct State initialise_state(void){

    struct State state; 
    state.snake[0].x = 0;
    state.snake[0].y = 0;

    uint8_t i; 
    for (i=1; i<NROW*NCOL; i++){
        // out of bound: none 
        state.snake[i].x = NCOL;
        state.snake[i].y = NROW;
    }

    state.food_loc.x=4;
    state.food_loc.y=5;

    state.is_left_pressed = 0;
    state.is_right_pressed = 0;

    return state;
}



const struct RegisterBit ROW_PIN[NROW] = {{&PORTA, 1}, {&PORTA, 2}, {&PORTA, 3}, {&PORTA, 4}, {&PORTC, 6}, {&PORTC, 1}, {&PORTC, 0}, {&PORTD, 7} };
const struct RegisterBit COLUMN_PIN[NCOL] = {{&PORTB, 0}, {&PORTB, 1}, {&PORTB, 2}, {&PORTB, 3}, {&PORTB, 4}, {&PORTB, 5}, {&PORTB, 6}, {&PORTB, 7} };

const struct RegisterBit LEFT_BUTTON = {&PORTD, 2};
const struct RegisterBit RIGHT_BUTTON = {&PORTD, 5}; 

uint8_t pix[NROW][NCOL];


// void lit_all_pix(void){
//     uint8_t r, c;
//     for (r=0; r<NROW; r++){
//         for (c=0; c<NCOL; c++){
//             pix[r][c] = 1;
//         }
//     }
// }



void set_one_pix(uint8_t r, uint8_t c, uint8_t on){
    struct RegisterBit row = ROW_PIN[r];
    struct RegisterBit col = COLUMN_PIN[c];

    // set_bit(*row.reg_addr, row.bit);
    // clear_bit(*col.reg_addr, col.bit);  

    if (on){
        clear_bit(*row.reg_addr, row.bit);
        set_bit(*col.reg_addr, col.bit);
    }
    //_delay_us(BLINK_DURATION_US); 

    set_bit(*row.reg_addr, row.bit);
    clear_bit(*col.reg_addr, col.bit);    
}


void flash(){
    uint8_t r, c;

    for (r=0; r<NROW; r++){
        for (c=0; c<NCOL; c++){
            set_one_pix(r, c, pix[r][c]);
        }
    }
}
void init(void){
    DDRA = 0xFF; 
    DDRB = 0xFF; 
    DDRC = 0xFF; 
    DDRD = 0xFF; 


    PORTA = 0x00; 
    PORTB = 0x00; 
    PORTC = 0x00; 
    PORTD = 0x00; 

}


struct State get_next_state(struct State state){
    
    state.snake[0].x += 1;
    state.snake[0].y += 1;
    
    return state;
}

void clear_pix(){
    uint8_t i, j; 
    for (i=0; i<NROW; i++){
        for (j=0; j<NCOL; j++){
            pix[i][j] = 0; 
        }
    }

}

void update_pix(struct State state){
    uint8_t i;

    clear_pix(); 
    
    for (i=0; i<NCOL*NROW; i++){
        if ((state.snake[i].x < NCOL) && (state.snake[i].y < NROW)){
            pix[state.snake[i].y][state.snake[i].x] = 1; 
            
        } else {
            break;
        }
    }

    pix[state.food_loc.y][state.food_loc.x] = 1; 
}




int main(void){

    clear_pix();
    struct State state;
    state = initialise_state();
    init();
    
    while (1)
    {   
        uint8_t i;
        state = get_next_state(state);
        update_pix(state);

        for (i=0; i<150; i++){
            flash();
        }
    }
}