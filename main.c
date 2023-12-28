
#define F_CPU 16000000UL 

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>


#define set_bit(var, bit) (var |= _BV(bit))
#define clear_bit(var, bit) (var &= ~_BV(bit))
#define is_bit_set(var, bit) ((var & _BV(bit)) != 0)


#define  NCOL 8
#define  NROW 8

#define BLINK_DURATION_US 10

struct RegisterBit{
    volatile uint8_t* reg_addr;
    uint8_t bit;
};

struct Coordinate{
    uint8_t x;
    uint8_t y;
};

enum Direction {UP=0, RIGHT=1, DOWN=2, LEFT=3};
#define NDIR 4

struct State{
    struct Coordinate snake[NROW*NCOL];
    struct Coordinate food_loc;
    uint8_t is_left_pressed; 
    uint8_t is_right_pressed; 

    uint8_t is_left_released; 
    uint8_t is_right_released; 

    enum Direction direction;
}; 
struct Coordinate gen_next_food_pos(struct Coordinate* snake);


struct State initialise_state(void){

    struct State state; 
    state.snake[0].x = NCOL/2;
    state.snake[0].y = NROW/2;

    uint8_t i; 
    for (i=1; i<NROW*NCOL; i++){
        // out of bound: none 
        state.snake[i].x = NCOL;
        state.snake[i].y = NROW;
    }

    state.food_loc = gen_next_food_pos(state.snake);

    state.is_left_pressed = 0;
    state.is_right_pressed = 0;

    state.is_left_released = 0; 
    state.is_right_released = 0; 

    state.direction = UP;

    return state;
}



const struct RegisterBit ROW_PIN[NROW] = {{&PORTA, 1}, {&PORTA, 2}, {&PORTA, 3}, {&PORTA, 4}, {&PORTC, 6}, {&PORTC, 1}, {&PORTC, 0}, {&PORTD, 7} };
const struct RegisterBit COLUMN_PIN[NCOL] = {{&PORTB, 0}, {&PORTB, 1}, {&PORTB, 2}, {&PORTB, 3}, {&PORTB, 4}, {&PORTB, 5}, {&PORTB, 6}, {&PORTB, 7} };


const struct RegisterBit LEFT_BUTTON_DDR = {&DDRD, 2};
const struct RegisterBit RIGHT_BUTTON_DDR = {&DDRD, 5}; 

const struct RegisterBit LEFT_BUTTON_PORT = {&PORTD, 2};
const struct RegisterBit RIGHT_BUTTON_PORT = {&PORTD, 5}; 

const struct RegisterBit LEFT_BUTTON_PIN = {&PIND, 2};
const struct RegisterBit RIGHT_BUTTON_PIN = {&PIND, 5}; 

uint8_t pix[NROW][NCOL];

// void lit_all_pix(void){
//     uint8_t r, c;
//     for (r=0; r<NROW; r++){
//         for (c=0; c<NCOL; c++){
//             pix[r][c] = 1;
//         }
//     }
// }

void dead(); 
void clear_pix();

void set_one_pix(uint8_t r, uint8_t c, uint8_t on){
    struct RegisterBit row = ROW_PIN[r];
    struct RegisterBit col = COLUMN_PIN[c];

    // set_bit(*row.reg_addr, row.bit);
    // clear_bit(*col.reg_addr, col.bit);  

    if (on){
        clear_bit(*row.reg_addr, row.bit);
        set_bit(*col.reg_addr, col.bit);
    }
    _delay_us(BLINK_DURATION_US); 

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


float float_sum(float* arr, uint8_t size){
    uint8_t i;
    float result = 0; 
    for (i=0; i<size; i++){
        result += arr[i];
    }
    return result; 
}

uint8_t random_with_distribution(float* proportion, uint8_t proportion_size){

    // have to use int instead of uint8_t
    int rand_num = rand(); 

    float sum_proportion = float_sum(proportion, proportion_size);

    uint8_t i; 
    float cumsum = 0;
    for (i=0; i<proportion_size; i++){
        cumsum += (proportion[i]/sum_proportion) * RAND_MAX;
        if (cumsum > rand_num){
            return i; 
        }
    }
    // shouldn't reach this part 
    return i; 
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



    clear_bit(*RIGHT_BUTTON_DDR.reg_addr, RIGHT_BUTTON_PORT.bit);
    clear_bit(*LEFT_BUTTON_DDR.reg_addr, LEFT_BUTTON_PORT.bit);

    srand(*LEFT_BUTTON_PIN.reg_addr);

    set_bit(*RIGHT_BUTTON_PORT.reg_addr, RIGHT_BUTTON_PORT.bit);
    set_bit(*LEFT_BUTTON_PORT.reg_addr, LEFT_BUTTON_PORT.bit);
}



struct State check_if_button_pressed(struct State state){
    if (!is_bit_set(*LEFT_BUTTON_PIN.reg_addr, LEFT_BUTTON_PIN.bit)){
        if (state.is_left_released){
             state.is_left_pressed = 1;
        }
    } else {
        state.is_left_released = 1;
    }

    if (!is_bit_set(*RIGHT_BUTTON_PIN.reg_addr, RIGHT_BUTTON_PIN.bit)){
        if (state.is_right_released){
             state.is_right_pressed = 1;
        }        
    } else {
        state.is_right_released = 1;
    }   
    // set it to true when the button is pressed (but not to false when released)

    return state;
}

uint8_t get_snake_last_segment(struct Coordinate* snake){
    uint8_t i;
    for (i=0; i<NCOL*NROW; i++){
        if (snake[i].x == NCOL) break; 
    }
    return i-1;
}


uint8_t if_collided(struct Coordinate* snake, struct Coordinate head_coord){

    uint8_t i; 
    for (i=1; i<NCOL*NROW; i++){
        if (snake[i].x == NCOL){
            return 0;
        }
        if (snake[i].x != head_coord.x) continue;
        if (snake[i].y != head_coord.y) continue;
        return 1;
    }
    return 0;
}




struct Coordinate gen_next_food_pos(struct Coordinate* snake){

    // 1: select the column 
    uint8_t i;

    float x_proportion[NCOL]; 

    for (i=0; i<NCOL; i++){
        x_proportion[i] = NCOL; 
    }

    for (i=0; i<NCOL*NROW; i++){

        if (snake[i].x == NCOL){
            break;
        }
        x_proportion[snake[i].x] -= 1; 
    }
    uint8_t x_pos = random_with_distribution(x_proportion, NCOL); 


    // 2: select the row 
    float y_proportion[NCOL]; 

    for (i=0; i<NROW; i++){
        y_proportion[i] = 1; 
    }

    for (i=0; i<NCOL*NROW; i++){

        if (snake[i].x == NCOL){
            break;
        }
        if (snake[i].x == x_pos){
            y_proportion[snake[i].y] -= 1; 
        }        
    }

    uint8_t y_pos = random_with_distribution(y_proportion, NROW); 
    

    struct Coordinate result = {.x = x_pos, .y = y_pos};
    return result;
}


struct State get_next_state(struct State state){

    if (state.is_left_pressed){
        state.direction = (state.direction + NDIR - 1) % NDIR; 

    }
    if (state.is_right_pressed){
        state.direction = (state.direction+1) % NDIR; 

    }
    state.is_right_pressed = 0;
    state.is_left_pressed = 0;

    state.is_left_released = 0; 
    state.is_right_released = 0; 



    // move the body forward except the head
    uint8_t snake_last_seg_idx = get_snake_last_segment(state.snake);

    // remember the last body segment location (for eating)
    struct Coordinate last_body_seg =  state.snake[snake_last_seg_idx];

    // move the body forward except the head
    uint8_t i; 
    for (i=snake_last_seg_idx; i>0; i--){
        state.snake[i] = state.snake[i-1];
    }

    // move the head
    switch (state.direction)
    {
    case UP:
        state.snake[0].y -= 1;
        break;
    case LEFT:
        state.snake[0].x -= 1;
        break;
    case RIGHT:
        state.snake[0].x += 1;
        break;
    case DOWN:
        state.snake[0].y += 1;      
        break;
    default:
        break;
    }

    if ((state.snake[0].x == state.food_loc.x) && (state.snake[0].y==state.food_loc.y)){
        state.snake[snake_last_seg_idx+1] = last_body_seg;
        state.food_loc = gen_next_food_pos(state.snake);

    }

    if (if_collided(state.snake, state.snake[0])){
        dead();
        state = initialise_state();
    }

    if ((state.snake[0].x < 0) || (state.snake[0].y < 0) || (state.snake[0].x >= NCOL) || (state.snake[0].y >= NROW)){
        dead();
        state = initialise_state();
    }

    
    return state;
}

void set_all_pix(){
    uint8_t i, j; 
    for (i=0; i<NROW; i++){
        for (j=0; j<NCOL; j++){
            pix[i][j] = 1; 
        }
    }

}
void dead(){
    uint8_t i, j; 
    
    for (i=0; i<5; i++){

        clear_pix();
        for (j=0; j<10; j++){
            flash();
        } 
        set_all_pix();
        for (j=0; j<10; j++){
            flash();
        } 
     }

    
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

        for (i=0; i<20; i++){
            flash();
            state = check_if_button_pressed(state);
        }
    }
}