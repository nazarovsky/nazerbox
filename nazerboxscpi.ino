//     NazerBox SCPI edition
//     20.04.2016 
// 

#include "Streaming.h" 
#include "scpiparser.h"
#include <Arduino.h>

const int gbPin = 11;    // the number of the green pushbutton pin
const int rbPin = 12;    // the number of the red pushbutton pin
const int genL = 9;      // the number of the left BNC pin
const int genR = 10;     // the number of the right BNC pin
const int greenLed = 2;  // the number of green LED pin
const int redLed = 3;    // the number of red LED pin
const int DEFAULT_DELAY=0;     // default delay in ms
const int DEFAULT_WIDTH=100;     // default delay in ms
const int DEFAULT_NUM=1;     // default delay in ms
const int POSLED=100;     // delay after impulse

uint32_t DELAY1; // Actual delay
uint32_t ADDON; // us compensation for quartz

uint32_t WIDTH; // impulse WIDTH
uint32_t NUM;   // impulse COUNT
uint32_t TEMPORARY; // temporary

#define portOfPin(P)\
  (((P)>=0&&(P)<8)?&PORTD:(((P)>7&&(P)<14)?&PORTB:&PORTC))
#define ddrOfPin(P)\
  (((P)>=0&&(P)<8)?&DDRD:(((P)>7&&(P)<14)?&DDRB:&DDRC))
#define pinOfPin(P)\
  (((P)>=0&&(P)<8)?&PIND:(((P)>7&&(P)<14)?&PINB:&PINC))
#define pinIndex(P)((uint8_t)(P>13?P-14:P&7))
#define pinMask(P)((uint8_t)(1<<pinIndex(P)))

#define pinAsInput(P) *(ddrOfPin(P))&=~pinMask(P)
#define pinAsInputPullUp(P) *(ddrOfPin(P))&=~pinMask(P);digitalHigh(P)
#define pinAsOutput(P) *(ddrOfPin(P))|=pinMask(P)
#define digitalLow(P) *(portOfPin(P))&=~pinMask(P)
#define digitalHigh(P) *(portOfPin(P))|=pinMask(P)
#define isHigh(P)((*(pinOfPin(P))& pinMask(P))>0)
#define isLow(P)((*(pinOfPin(P))& pinMask(P))==0)
#define digitalState(P)((uint8_t)isHigh(P))

struct scpi_parser_context ctx;

scpi_error_t identify(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t arm_pps(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t get_delay(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t set_delay(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t get_width(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t set_width(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t get_num(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t set_num(struct scpi_parser_context* context, struct scpi_token* command);

//**************Delay****************************
void delay_ms(uint16_t tic_ms)
{
    while(tic_ms)
    {
      delay_us(999);
      tic_ms--;
    }
}

void delay_us(uint16_t tic_us)
{
  tic_us *= 4; //1us = 4 цикла
  __asm__ volatile 
      ( 
      "1: sbiw %0,1" "\n\t" //; вычесть из регистра значение N
      "brne 1b"       
      : "=w" (tic_us)
      : "0" (tic_us)
      );
}

// calibrated delay in ms (from 1)
void delay_calibrated(uint16_t tic_ms)
{
  if (tic_ms)
  {
    ADDON=int(float(tic_ms*52/100)); // duration is 14 us
    tic_ms--;
    delay_us(996-14+ADDON);    
    while(tic_ms)
      {
        delay_us(999);
        tic_ms--;
      }
  }
}


void defaults(){
  pinMode(gbPin, INPUT);
  pinMode(rbPin, INPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(genL, INPUT);  // left channel is 1PPS input
  pinMode(genR, OUTPUT); // right channel is delayed impulse output 
  // set initial LED state
  digitalWrite(greenLed,1);
  digitalWrite(redLed,0);
  digitalWrite(genR,0);
  DELAY1=DEFAULT_DELAY;
  WIDTH =DEFAULT_WIDTH;
  NUM   =DEFAULT_NUM;

}

void setup()
{
  struct scpi_command* arm;
  struct scpi_command* dly;

  /* First, initialise the parser. */
  scpi_init(&ctx);

  /*
   * After initialising the parser, we set up the command tree.  Ours is
   *
   *  *IDN?         -> identify
   *  ARM           -> ARM device to fire on next PPS
   *  DELAY (?)     -> GET / SET delay (from 0 to 1000 ms)
   *  WIDTH (?)     -> GET / SET impulse width (from 1 to 1000 ms)
   *  NUM (?)       -> GET / SET impulse count (from 1 to 100)
      */
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*IDN?", 5, "*IDN?", 5, identify);
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*RST", 4, "*RST", 4, reset);
  arm = scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "ARM", 3, "ARM", 3, arm_pps);
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "DELAY", 5, "DLY", 3, set_delay);
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "DELAY?", 6, "DLY?", 4, get_delay);
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "WIDTH", 5, "WIDTH", 5, set_width);
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "WIDTH?", 6, "WIDTH?", 6, get_width);
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "NUM", 3, "NUM", 3, set_num);
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "NUM?", 4, "NUM?", 4, get_num);

  
  /*
   * Next, we set our outputs to some default value.
   */

  defaults();
  
  Serial.begin(115200);
}

void fire(){
    noInterrupts();

    digitalLow(greenLed);
    for (int i=0;i<NUM;i++){
       while (isHigh(genL)) {  // wait for PPS low
       }
       while (isLow(genL)) { // wait for PPS high
       }
       delay_calibrated(DELAY1);
       digitalHigh(redLed);
       PORTB=PORTB | B00000100; // set HIGH pin 10
       delay_calibrated(WIDTH);
       PORTB=PORTB & B11111011; // set LOW pin 10
       digitalLow(redLed);
       delay_ms(POSLED);
    }
    digitalHigh(greenLed);
  interrupts();
}

void loop()
{
  char line_buffer[256];
  unsigned char read_length;
  
  while(1)
  {
      /*------   PROCESS HARDWARE BUTTONS---------*/
      if (isLow(gbPin)) {
         TEMPORARY=WIDTH;     
         WIDTH=1;         
         fire();     
         WIDTH=TEMPORARY;
      }

      if (isLow(rbPin)) {
         TEMPORARY=WIDTH;     
         WIDTH=100;         
         fire();     
         WIDTH=TEMPORARY;
      }      

    /*------   PROCESS SERIAL ---------*/
    /* Read in a line and execute it. */
    if (Serial.available()>0){
       read_length = Serial.readBytesUntil('\n', line_buffer, 256);
       if(read_length > 0)
       {
         scpi_execute_command(&ctx, line_buffer, read_length);
       }
    }
    /*------  SCPI LOOP END ---------*/
  }
}


/*
 * Respond to *IDN?
 */
scpi_error_t identify(struct scpi_parser_context* context, struct scpi_token* command)
{
  scpi_free_tokens(command);

  Serial.println("NazerTechnologies,NazerBox 1.0 SCPI edition,0001,0.1");
  return SCPI_SUCCESS;
}


/*
 * Respond to *RST
 */
scpi_error_t reset(struct scpi_parser_context* context, struct scpi_token* command)
{
  scpi_free_tokens(command);
  defaults();
  return SCPI_SUCCESS;
}


scpi_error_t arm_pps(struct scpi_parser_context* context, struct scpi_token* command)
{
  scpi_free_tokens(command);
  
  fire(); 
  return SCPI_SUCCESS;
}


/**
 * Read the current delay
 */
scpi_error_t get_delay(struct scpi_parser_context* context, struct scpi_token* command)
{
  Serial.println(DELAY1);

  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

/**
 * Set the impulse delay
 */
 
scpi_error_t set_delay(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_token* args;
  struct scpi_numeric output_numeric;
  unsigned char output_value;

  args = command;

  while(args != NULL && args->type == 0)
  {
    args = args->next;
  }

  output_numeric = scpi_parse_numeric(args->value, args->length, 0, 0, 1000);  //defaut,min,max
  
  if(output_numeric.length == 0)
  {
    DELAY1=(unsigned long)constrain(output_numeric.value, 0, 1000);
    Serial.println(DELAY1);  
  }
  else
  {
    scpi_error error;
    error.id = -200;
    error.description = "Command error;Invalid unit";
    error.length = 26;
    
    scpi_queue_error(&ctx, error);
    scpi_free_tokens(command);
    return SCPI_SUCCESS;
  }

  scpi_free_tokens(command);

  return SCPI_SUCCESS;
}


/**
 * Read the current num
 */
scpi_error_t get_num(struct scpi_parser_context* context, struct scpi_token* command)
{
  Serial.println(NUM);

  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}


scpi_error_t set_num(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_token* args;
  struct scpi_numeric output_numeric;
  unsigned char output_value;

  args = command;

  while(args != NULL && args->type == 0)
  {
    args = args->next;
  }

  output_numeric = scpi_parse_numeric(args->value, args->length, 1, 1, 100);  //defaut,min,max
  
  if(output_numeric.length ==0) 
  {
    NUM=(unsigned long)constrain(output_numeric.value, 1, 100);
    Serial.println(NUM);  
  }
  else
  {
    scpi_error error;
    error.id = -200;
    error.description = "Command error;Invalid unit";
    error.length = 26;
    
    scpi_queue_error(&ctx, error);
    scpi_free_tokens(command);
    return SCPI_SUCCESS;
  }

  scpi_free_tokens(command);

  return SCPI_SUCCESS;
}



/**
 * Read the current width
 */
scpi_error_t get_width(struct scpi_parser_context* context, struct scpi_token* command)
{
  Serial.println(WIDTH);

  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}
/**
 * Set the current width
 */

scpi_error_t set_width(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_token* args;
  struct scpi_numeric output_numeric;
  unsigned char output_value;

  args = command;

  while(args != NULL && args->type == 0)
  {
    args = args->next;
  }

  output_numeric = scpi_parse_numeric(args->value, args->length, 1, 1, 1000);  //defaut,min,max
  
  if(output_numeric.length ==0) 
  {
    WIDTH=(unsigned long)constrain(output_numeric.value, 1, 1000);
    Serial.println(WIDTH);  
  }
  else
  {
    scpi_error error;
    error.id = -200;
    error.description = "Command error;Invalid unit";
    error.length = 26;
    
    scpi_queue_error(&ctx, error);
    scpi_free_tokens(command);
    return SCPI_SUCCESS;
  }

  scpi_free_tokens(command);

  return SCPI_SUCCESS;
}


