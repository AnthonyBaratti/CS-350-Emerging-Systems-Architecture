/*
 * Copyright (c) 2015-2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== gpiointerrupt.c ========
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>

/* Driver configuration */
#include "ti_drivers_config.h"

/* Timer header files*/
#include <ti/drivers/Timer.h>

// I2C header files
#include <ti/drivers/I2C.h>

// UART header files
#include <ti/drivers/UART.h>
#include <ti/display/Display.h>



/*
 * Display configuration
 *
 * ##FIXME I can't get this to work
 * Resource:
 * https://software-dl.ti.com/ecs/SIMPLELINK_CC32XX_SDK/2_40_01_01/exports/docs/tidrivers/doxygen/html/_display_8h.html
 */

/*
Display_Handle display;
Display_Params displayParams;
void initDisplay(void){
    Display_init();  
    Display_Params_init(&displayParams);  
    display = Display_open(Display_Type_UART, &displayParams);  

    if (display == NULL) {  
        while (1); // Handle initialization failure  
    }  
}*/


#define DISPLAY(x) UART_write(uart, &output, x);
// UART Global Variables
char output[64];
int bytesToSend;

/*
 *  UART configuration
 */
// Driver Handles - Global variables
UART_Handle uart;
UART_Params uartParams;
void initUART(void)
{

    // Init the driver
    UART_init();
    // Configure the driver
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_TEXT;
    uartParams.readDataMode = UART_DATA_TEXT;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.baudRate = 115200;
    // Open the driver
    uart = UART_open(CONFIG_UART_0, &uartParams);
    if (uart == NULL) {
        /* UART_open() failed */
        while (1);
    }

}



/*
 *  I2C Configuration
 *
 *
 *
 */
// I2C Global Variables
static const struct {
    uint8_t address;
    uint8_t resultReg;
    char *id;
}

sensors[3] = {
{ 0x48, 0x0000, "11X" },
{ 0x49, 0x0000, "116" },
{ 0x41, 0x0001, "006" }
};

uint8_t txBuffer[1];
uint8_t rxBuffer[2];
I2C_Transaction i2cTransaction;

// Driver Handles - Global variables
I2C_Handle i2c;

// Make sure you call initUART() before calling this function.
void initI2C(void)
{
    int8_t i, found;
    I2C_Params i2cParams;
    DISPLAY(snprintf(output, 64, "Initializing I2C Driver - "));

    // Init the driver
    I2C_init();

    // Configure the driver
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;

    // Open the driver
    i2c = I2C_open(CONFIG_I2C_0, &i2cParams);
    if (i2c == NULL)
    {
        DISPLAY(snprintf(output, 64, "Failed\n\r"));
        while (1);
    }
    DISPLAY(snprintf(output, 32, "Passed\n\r"));
    // Boards were shipped with different sensors.
    // Welcome to the world of embedded systems.
    // Try to determine which sensor we have.
    // Scan through the possible sensor addresses
    /* Common I2C transaction setup */
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 0;
    found = false;
    for (i=0; i<3; ++i)
    {
        i2cTransaction.slaveAddress = sensors[i].address;
        txBuffer[0] = sensors[i].resultReg;
        DISPLAY(snprintf(output, 64, "Is this %s? ", sensors[i].id));
        if (I2C_transfer(i2c, &i2cTransaction))
        {
            DISPLAY(snprintf(output, 64, "Found\n\r"));
            found = true;
            break;
        }
        DISPLAY(snprintf(output, 64, "No\n\r"));
    }
    if(found)
    {
        DISPLAY(snprintf(output, 64, "Detected TMP%s I2C address: "
                "%x\n\r", sensors[i].id, i2cTransaction.slaveAddress));
    }
    else
    {
        DISPLAY(snprintf(output, 64, "Temperature sensor not found, contact professor\n\r"));
    }
}

int16_t readTemp(void)
{
    int j;
    int16_t temperature = 0;
    i2cTransaction.readCount = 2;
    if (I2C_transfer(i2c, &i2cTransaction))
    {
        /*
         * Extract degrees C from the received data;
         * see TMP sensor datasheet
         */
        temperature = (rxBuffer[0] << 8) | (rxBuffer[1]);
        temperature *= 0.0078125;
        /*
         * If the MSB is set '1', then we have a 2's complement
         * negative value which needs to be sign extended
         */
        if (rxBuffer[0] & 0x80)
        {
            temperature |= 0xF000;
        }
    }
    else
    {
        DISPLAY(snprintf(output, 64, "Error reading temperature sensor "
                "(%d)\n\r",i2cTransaction.status));
        DISPLAY(snprintf(output, 64, "Please power cycle your board by unplugging"
                " USB and plugging back in.\n\r"));
    }
    return temperature;
}

/*  Define a structure for all tasks shared variables
 *  ALL Tasks will have state, period, elapsed time, and their
 *  unique response function (SM)
 */
typedef struct task {
    int state;                  //Current State of task
    unsigned long period;       //Rate at which the task should tick
    unsigned long elapsedTime;  //Time since tasks previous tick
    int (*TickFct) (int);       //Call task tick function
} task;

// Set number of tasks, read temp, write display, button press, LED out
task tasks [4];
const unsigned char numTasks = 4;

/*
 *  These timer variables will be used to determine periods.
 *  The GCD will be set to the same as the timerCallBack function
 *  all times are represented as milliseconds (Based off the GCD
 *  and timerCallBack period). Each variable will be attached
 *  to their appropriate task within main.
 */
const unsigned int timerGCD = 100;          //GCD of all period checks (200, 500, 1000)
const unsigned int buttonTimer = 200;       //period for button check
const unsigned int tempTimer = 500;         //period for temp check
const unsigned int displayTimer = 1000;     //period for display output

//Initialize variables to be used for data manipulation
int setPoint = 0;      //default starting temp in Celsius (68 degrees F)
int heat = 0;           //return value of heat. 1 is on, 0 is off.
int temperature;        //current temperature value
int seconds = 0;        //time program has been running

//Initialize button flags
int setTempDown= 0;
int setTempUp = 0;

/*
 *  This Task Manager is the heart of the task scheduler
 *  It is designed to iterate through all of the tasks in round robin, checking
 *  each period to the elapsed time. If the elapsed time has passed the period check,
 *  the state changes based on the SMs for each task.
 *
 *  The task manager will be called by the timerCallBack function every GCD (100 ms)
 */
void TaskManager(){
    unsigned char i;
    for (i = 0; i < numTasks; ++i){   //Cycles through each task every GCD
        if (tasks[i].elapsedTime >= tasks[i].period) {  //Checks elapsed time
            tasks[i].state = tasks[i].TickFct(tasks[i].state); //sets state if time has elapsed
            tasks[i].elapsedTime = 0; //reset elapsed time
        }
        tasks[i].elapsedTime += timerGCD;
    }
}

/*
 *  Button States and Button SM declarations
 */
enum Button_States {Btn_Wait, Btn_Increase, Btn_Decrease} Button_State;
int TickFct_Button(int state);

/*
 *  This SM drives the button checks. The Wait state determines the transition.
 *  If the setDownTemp is activated with left board button, the state changes to
 *  Btn_Decrease.
 *  If the setUpTemp is actived with the right board button, the state changes
 *  to Btn_Increase.
 *  Then the state is reset to Btn_Wait & the button variable
 *   for the timer to check for the next button press.
 *  This SM will be called every time the elapsedTime surpasses the buttonTimer (200ms)
 *  It will also adjust the setPoint variable to display the current setting of the
 *  thermostat through the UART/I2C
 *
 * @params: current Button_State
 * @return: updated Button_State
 *
 */
int TickFct_Button(int state) {
    switch(state){ //Button State Start
        case Btn_Wait:
            //no state action for wait, continue to wait for task manager
            break;
        case Btn_Increase:
            setPoint += 1;      //raise setPoint degree by 1
            setTempUp = 0;      //reset button flag
            state = Btn_Wait;   //wait for next task manager call
            break;
        case Btn_Decrease:
            setPoint -= 1;      //lower setPoint degree by 1
            setTempDown = 0;    //reset button flag
            state = Btn_Wait;   //wait for next task manager call
            break;
        default:
            break;
    }//Button State end

    switch(state){ //Button Transitions start
        case Btn_Wait:
            if (setTempUp == 1) {
                state = Btn_Increase;
            }
            else if (setTempDown == 1){
                state = Btn_Decrease;
            }
            else {
                state = Btn_Wait; //if no buttons pressed, remain in wait state
            }
            break;
        default:
            break;
    } //Button transitions end
    return state;
}

/*
 *  LED States and SM function declarations
 */
enum LED_States {LED_On, LED_Off} LED_State;
int TickFct_LED(int state);


/*
 *  This SM is designed to operate the LED. It will be called
 *  everytime the elapsedTime is greater than the displayTimer.
 *  The transition states compare the current temperature
 *  with the setPoint, and turns the LED on or off according
 *  to the transitions.
 *
 * @params: current LED_State
 * @return: updated LED_State
 */
int TickFct_LED(int state) {
    temperature = readTemp();
    switch(state) { //LED States start
        case LED_On:
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);  //heater on
            break;
        case LED_Off:
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF); //heater off
            break;
        default:
            break;
    }//LED States end

    switch(state) { //LED transitions start
        case LED_On:
            if (temperature > setPoint){    //Actual temp is higher than set temp
                state = LED_Off;            //turn heater off
            }
            break;
        case LED_Off:
            if (temperature < setPoint){    //Actual temp is lower than set temp
                state = LED_On;              //turn heater on
            }
            break;
    } //LED transitions end
    return state;
}


//Display message SM and Declarations
enum Message_States {Msg_Wait, Msg_Display} Msg_State;
int TickFct_Display(int state);

/*
 *  This SM is designed to print the display readings
 *  of temperature, setPoint, LED, and operating time.
 *  It will be called from the task manager every time
 *  the elapsedTime is greater than displayTimer (1 second)
 *
 * @params: current Msg_State
 * @return: updated Msg_State
 */
int TickFct_Display(int state){
    switch(state) { //Display State Start
        case Msg_Wait:
            //No state actions for wait
            break;
        case Msg_Display:
            seconds += 1;   //tracking operation runtime, increase with each task manager call

            //Displays output using UART
            DISPLAY(snprintf(output, 64, "<%02d,%02d,%d,%04d>\n\r", temperature, setPoint, heat, seconds))
            break;
    } //Display State End

    switch(state) { //Display Transition Start
        case Msg_Wait:
            state = Msg_Display;
            break;
        case Msg_Display:
            state = Msg_Wait; //Waits to be called again by task manager.
            break;
    } //Display Transition End
    return state;
}

//Temperature States and SM Declarations
enum Temperature_States {Tmp_Wait, Tmp_Read} Tmp_State;
int TickFct_Temp(int state);

/*
 * This SM is designed to read the temperature and set
 * the temperature variable to the current temperature of the room.
 * This variable will be used in the display, and as a check against
 * setPoint in the LED (heater) SM
 *
 * @params: current Tmp_State
 * @return: updated Tmp_State
 */
int TickFct_Temp(int state){
    switch(state){ //Temperature State start
        case Tmp_Wait:
            //No state for wait
            break;
        case Tmp_Read:
            temperature = readTemp(); //reads temperature from sensor
            break;
        default:
            break;
    } //Temperature State end

    switch(state){ //Temperature Transitions start
        case Tmp_Wait:
            state = Tmp_Read;
            break;
        case Tmp_Read:
            state = Tmp_Wait; //waits to be called again by task manager
            break;
        default:
            break;
    } //Temp transitions end
    return state;
}


volatile unsigned char timerFlag = 0;
void timerCallback(Timer_Handle myHandle, int_fast16_t status)
{
    timerFlag = 1;
}

/*
 * Timer Interrupt for GPIO
 * The timerCallBack will be called every 100 milliseconds (GCD of all timers)
 * It will call the task manager, which will iterate through all of the tasks
 * (read temp, write display, button check, LED(heat)).
 */

Timer_Handle timer0; //Global timer variable
void initTimer(void)
{
    Timer_Params params;
    // Init the driver
    Timer_init();
    // Configure the driver
    Timer_Params_init(&params);
    params.period = 100000; //task GCD
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = timerCallback;
    // Open the driver
    timer0 = Timer_open(CONFIG_TIMER_0, &params);
    if (timer0 == NULL) {
        /* Failed to initialized timer */
        while (1) {}
    }
    if (Timer_start(timer0) == Timer_STATUS_ERROR) {
        /* Failed to start timer */
        while (1) {}
    }
}


/*
 *  ======== gpioButtonFxn0 ========
 *  Callback function for the GPIO interrupt on CONFIG_GPIO_BUTTON_0.
 *
 *  Note: GPIO interrupts are cleared prior to invoking callbacks.
 */


//Reduce set temperature
void gpioButtonFxn0(uint_least8_t index)
{
    setTempDown = 1;
}

/*
 *  ======== gpioButtonFxn1 ========
 *  Callback function for the GPIO interrupt on CONFIG_GPIO_BUTTON_1.
 *  This may not be used for all boards.
 *
 *  Note: GPIO interrupts are cleared prior to invoking callbacks.
 */

//Increase set temperature
void gpioButtonFxn1(uint_least8_t index)
{
    char messageOn[]="Button 1 press";
    UART_write(uart, messageOn, sizeof(messageOn));
    setTempUp = 1;
}


/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    /* Call driver init functions */

    initUART();
    initI2C();
    //initDisplay();
    GPIO_init();

    /*
     *  Configure the LED and button pins
     */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_BUTTON_0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);
    GPIO_setConfig(CONFIG_GPIO_BUTTON_1, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

    /* Install Button callback */
    GPIO_setCallback(CONFIG_GPIO_BUTTON_0, gpioButtonFxn0);;
    GPIO_setCallback(CONFIG_GPIO_BUTTON_1, gpioButtonFxn1);;
    /* Enable interrupts */
    GPIO_enableInt(CONFIG_GPIO_BUTTON_0);
    GPIO_enableInt(CONFIG_GPIO_BUTTON_1);

    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);
    /*
     *  These sets of variables construct each task for
     *  the task manager to use. each task will iterate with every
     *  call of the callBackTimer (100ms). The TaskManager function will
     *  then check each period to the elapsed time, and change the state of
     *  each task if the elapsed time is geq to the tasks specified period.
     *  The task will then perform is designated action, then
     *  reset its elapsed time if it was geq, or
     *  increment the elapsed time of the task by the GCD
     */
    unsigned char i = 0;
    //Task 1: Button Function
    tasks[i].state = Btn_Wait; //Default until first read
    tasks[i].period = buttonTimer;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFct_Button;
    ++i;
    //Task 2: LED (heater) control
    tasks[i].state = LED_Off; //Default until first read
    tasks[i].period = displayTimer;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFct_LED;
    ++i;
    //Task 3: Display message
    tasks[i].state = Msg_Wait; //Default until first read
    tasks[i].period = displayTimer;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFct_Display;
    ++i;
    //Task 4: Temperature reading
    tasks[i].state = Tmp_Wait; //Default until first read
    tasks[i].period = tempTimer;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFct_Temp;

    initTimer();
    while(1){

        if(timerFlag){
            TaskManager();
            timerFlag=0;
        }
    }
}
