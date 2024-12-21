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

/* Driver Header files */
#include <ti/drivers/GPIO.h>

/* Driver configuration */
#include "ti_drivers_config.h"

/* Timer header files*/
#include <ti/drivers/Timer.h>

/*
 * Two separate switch cases will keep track of the same 2 states.
 * The Message_State switch case will create functionality (output) for the message to the LEDs
 * The Button_Press switch case will control the transitions (input) between each Message_State
 *
 */
enum Message_States {Message_SOS, Message_OK} Button_Press, Message_State;

/*
 *  This allows the GPIO write to be pre-defined for the red and green LEDs
 *  When LED_State_SM() is called, it will be called for the current iteration of the
 *  pre-defined message array every 500ms until the array is finished.
 *
 *  (See LED_States sosMsgArray & LED_States okMsgArray)
 */
enum LED_States {Red_On, Green_On, RG_OFF} LED_State;

/*
 *  This state machine will be used by timerCallback function to light
 *  LEDs 500ms at a time
 */
void LED_State_SM(){
    switch (LED_State){
    case (Red_On):
        GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF); //Green off
        GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON); //red on
        break;
    case (Green_On):
        GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_ON); //Green on
        GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF); //red off
        break;
    case (RG_OFF):
        GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF); //Green off
        GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF); //red off
        break;
    default:
        break;
    }
}


/*
 *  This is an array of states designed to be read in order with each timerCallback (500 milliseconds)
 *  Will be iterated and output with LED_State_SM() method in the Message_SOS switch case
 *
 *  SOS Message (dot dot dot, dash dash dash, dot dot dot)
 */
enum LED_States sosMsgArray [] = {
    //dot dot dot (S) 1500ms pause
    Red_On, RG_OFF, Red_On, RG_OFF, Red_On, RG_OFF, RG_OFF, RG_OFF,

    //dash dash dash (O) 1500ms pause
    Green_On, Green_On, Green_On, RG_OFF,
    Green_On, Green_On, Green_On, RG_OFF,
    Green_On, Green_On, Green_On, RG_OFF,RG_OFF, RG_OFF,

    //dot dot dot (S) 1500ms pause
    Red_On, RG_OFF, Red_On, RG_OFF, Red_On, RG_OFF, RG_OFF, RG_OFF,

    //2000ms pause to fit 3500 ms per word gap
    RG_OFF, RG_OFF, RG_OFF, RG_OFF
};

/*
 *  This is an array designed to be read in order with each timerCallBack (500 milliseconds)
 *  Will be iterated and output with LED_State_SM() in the Message_OK switch case
 *
 *  OK message (dash dash dash, dot dash dot)
 */
enum LED_States okMsgArray [] = {
    //dash dash dash (O) 1500ms pause
    Green_On, Green_On, Green_On, RG_OFF,
    Green_On, Green_On, Green_On, RG_OFF,
    Green_On, Green_On, Green_On, RG_OFF, RG_OFF, RG_OFF,

    //dot dash dot (K) 1500ms pause
    Red_On, RG_OFF,
    Green_On, Green_On, Green_On, RG_OFF,
    Red_On, RG_OFF, RG_OFF, RG_OFF,

    //2000 ms pause to fit 3500ms per word gap
    RG_OFF, RG_OFF, RG_OFF, RG_OFF
};

/*
 *  This variable will be used to keep track of the array position with each timerCallBack
 *  Progressing the state read in the array by 1 until the end is reached (every 500 ms)
 */
unsigned int i = 0;



/*
 * Message_State_SM():
 *
 *  Each respective case will iterate through its respective MsgArray, using LED_State_SM() to call
 *     the LED that is suppose to be lit (red, green, or neither). Then it increments the iterator variable (i)
 *     and checks to see if the end of the Array has been reached before checking the
 *     transition state (if the button was pressed, the transition state changed
 *     /if the button was not pressed, transition to same state). If the array is finished
 *     the iterator variable is reset to zero (to start at the beginning of whichever array will be used)
 *     and the Message_State transitions to whichever Button_State is active. Using the if statement
 *     check at the end of each recursive call allows the message to be fully displayed before transitioning
 *     to a new (or the same) state.
 *
 *     Called in timerCallback() Method to execute every 500ms
 *     Functionality: Display SOS/OK Messages.
 *
 */

void Message_State_SM(){
    //STATES START
    switch (Message_State) {

        case (Message_SOS):
                //Set state from the array position
                LED_State = sosMsgArray[i];

                //execute the state of the ith position
                LED_State_SM();

                i += 1; //increment the position

                /*
                 *  To iterate to the end of the array, we increment size with each iteration
                 *  Best practice not to use for loops within embedded systems to prevent locking
                 *  so the increment is done within the state switch case and is replayed
                 *  by the timerCallBack function (recursive every 500ms)
                 *  C offers no length check of arrays, so this snippet of code was used from:
                 *
                 *  Lemonaki, K. (2022, December 5). How to find the size of an array
                 *      in C with the sizeof operator. Freecodecamp.org.
                 *      https://www.freecodecamp.org/news/how-to-find-the-size-of-an-array-in-c-with-the-sizeof-operator/
                 */

                if (i == (sizeof(sosMsgArray)/sizeof(sosMsgArray[0]))){
                    i = 0; //reset to read array from beginning

                    /*
                     *  This statement is held within the if statement to allow the entire message to
                     *  finish being displayed. Once the MsgArray has reached its end, the counter
                     *  gets reset to 0, and the Message_State can transition to a new message if
                     *  if the button was pressed. If the button was not pressed, repeat Message_State.
                     */

                    Message_State = Button_Press;  //loads Message_State with current Button_Press transition.
                }
            break;
        case (Message_OK):
                  //Set message array position
                  LED_State = okMsgArray[i];

                  //Use the state of the ith position
                  LED_State_SM();

                  i += 1; //increment the position

                  //i has reached the end of array (Lemonaki, 2022).
                  if (i == (sizeof(okMsgArray)/sizeof(okMsgArray[0]))){
                      //reset to read array from beginning.
                      i = 0;

                      //loads Message_State with current Button_Press transition
                      Message_State = Button_Press;
                   }
            break;
        default:
            break;
    } //STATES END
}



/*
 *  This switch case is designed as a transition state machine.
 *  Since transitions are based solely on input (button push) it is
 *  implemented within the gpioButtonFxn0 function.
 *
 *  The Message_State switch case only reads the Button_Press state after the
 *  MsgArray has completely finished displaying the current message. Then
 *  the Message_State transitions to the Button_Press (even if it did not change).
 *
 *  Called in:
 *  gpioButtonFxn0()
 *  gpioButtonFxn1()
 *
 *  Functionality: Manage transitions between Message_States
 */

void Button_State_SM() {

    //TRANSITIONS START
    switch (Button_Press) {
        case Message_SOS:
            Button_Press = Message_OK;
            break;
        case Message_OK:
            Button_Press = Message_SOS;
            break;
        default:
            break;
    } //TRANSITIONS END
}


/*
 *  Timer interrupt for GPIO
 *  Initialized to 500ms recursive.
 *  Operates a switch case (Message_State) to switch between the two messages (SOS and OK)
 *  The gpioButtonFxn0() function will determine which state is used (a press changes the state)
 *  This is simply the functionality (not the transitions).
 */
void timerCallBack (Timer_Handle myHandle, int_fast16_t status)
{
    //used to display message through LEDs
    Message_State_SM();
}

void initTimer(void)
{
    Timer_Handle timer0;
    Timer_Params params;

    Timer_init();
    Timer_Params_init(&params);
    params.period = 500000; //set to 500 milliseconds
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_CONTINUOUS_CALLBACK; //Non-blocking callback
    params.timerCallback = timerCallBack;

    timer0 = Timer_open (CONFIG_TIMER_0, &params);

    if (timer0 == NULL) {
        /*Failed to initialized timer*/
        while (1) {}
    }

    if (Timer_start(timer0) == Timer_STATUS_ERROR) {
        /* Failed to start timer*/
            while (1) {}
    }
}


/*
 *  ======== gpioButtonFxn0 ========
 *  Callback function for the GPIO interrupt on CONFIG_GPIO_BUTTON_0.
 *
 *  Note: GPIO interrupts are cleared prior to invoking callbacks.
 */
void gpioButtonFxn0(uint_least8_t index)
{
    //used to detect button input and determine transition
    Button_State_SM();
}

/*
 *  ======== gpioButtonFxn1 ========
 *  Callback function for the GPIO interrupt on CONFIG_GPIO_BUTTON_1.
 *  This may not be used for all boards.
 *
 *  Note: GPIO interrupts are cleared prior to invoking callbacks.
 */
void gpioButtonFxn1(uint_least8_t index)
{
    //used to detect button input and determine transition
    Button_State_SM();
}


/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    /* Call driver init functions */
    GPIO_init();
    initTimer();
    /* Configure the LED and button pins */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_LED_1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_BUTTON_0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

    /* Turn on user LED */
    //GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    /* Install Button callback */
    GPIO_setCallback(CONFIG_GPIO_BUTTON_0, gpioButtonFxn0);

    /* Enable interrupts */
    GPIO_enableInt(CONFIG_GPIO_BUTTON_0);

    //Initialize states.
    Message_State = Button_Press;

    /*
     *  If more than one input pin is available for your device, interrupts
     *  will be enabled on CONFIG_GPIO_BUTTON1.
     */
    if (CONFIG_GPIO_BUTTON_0 != CONFIG_GPIO_BUTTON_1) {
        /* Configure BUTTON1 pin */
        GPIO_setConfig(CONFIG_GPIO_BUTTON_1, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

        /* Install Button callback */
        GPIO_setCallback(CONFIG_GPIO_BUTTON_1, gpioButtonFxn1);
        GPIO_enableInt(CONFIG_GPIO_BUTTON_1);
    }

    return (NULL);
}
