#include "main.h"
#include "driverlib/driverlib.h"
#include "hal_LCD.h"

void VENTILATIONA(void);
void VENTILATIONB(void);
void IRRIGATIONA(void);
void IRRIGATIONB(void);
void uartTransmit(void);
void uartDisplay(uint8_t *sendText, uint8_t length);

char ADCState = 0;
int16_t ADCResult = 0;

static _Bool adcBusy = false;
static _Bool adc_done = false;

static _Bool zone = false;
static _Bool pressed = false;

long MUXRESULT;
int muxselect = 1;

int sensor;

static uint8_t cliBuffer[15];
static uint8_t cliIndex = 0;

volatile static _Bool lightsensor = true;

static uint8_t TEMPERATUREA = 0;
static uint8_t TEMPERATUREB = 0;
static uint8_t MOISTUREA = 0;
static uint8_t MOISTUREB = 0;

static uint8_t TEMPERATUREATHRES = 99;
static uint8_t TEMPERATUREBTHRES = 99;
static uint8_t MOISTUREATHRES = 0;
static uint8_t MOISTUREBTHRES = 0;

volatile static _Bool VmotorA = true;
volatile static _Bool VmotorB = true;
volatile static _Bool ImotorA = true;
volatile static _Bool ImotorB = true;

volatile static _Bool UARTSIGNAL = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void main(void)
{
    char buttonState = 0; //Current button press state (to allow edge detection)

    /*
     * Functions with two underscores in front are called compiler intrinsics.
     * They are documented in the compiler user guide, not the IDE or MCU guides.
     * They are a shortcut to insert some assembly code that is not really
     * expressible in plain C/C++. Google "MSP430 Optimizing C/C++ Compiler
     * v18.12.0.LTS" and search for the word "intrinsic" if you want to know
     * more.
     * */

    //Turn off interrupts during initialization
    __disable_interrupt();

    //Stop watchdog timer unless you plan on using it
    WDT_A_hold(WDT_A_BASE);

    // Initializations - see functions for more detail
    Init_GPIO();    //Sets all pins to output low as a default
    Init_PWM();     //Sets up a PWM output
    Init_ADC();     //Sets up the ADC to sample
    Init_Clock();   //Sets up the necessary system clocks
    Init_UART();    //Sets up an echo over a COM port
    Init_LCD();     //Sets up the LaunchPad LCD display

     /*
     * The MSP430 MCUs have a variety of low power modes. They can be almost
     * completely off and turn back on only when an interrupt occurs. You can
     * look up the power modes in the Family User Guide under the Power Management
     * Module (PMM) section. You can see the available API calls in the DriverLib
     * user guide, or see "pmm.h" in the driverlib directory. Unless you
     * purposefully want to play with the power modes, just leave this command in.
     */
    PMM_unlockLPM5(); //Disable the GPIO power-on default high-impedance mode to activate previously configured port settings

    //All done initializations - turn interrupts back on.
    __enable_interrupt();


    uint8_t cliWelcome[100];
    int cliIndex;
    for (cliIndex = 0 ; cliIndex < 100 ; cliIndex++)
        cliWelcome[cliIndex]= 0; /* initialize welcome message */

    strcpy((char*) cliWelcome, "               ECE 298 - Group 1              ");
    uartDisplay(cliWelcome, strlen((char*) cliWelcome));

   GPIO_setAsInputPinWithPullDownResistor(LIGHT_PORT, LIGHT_PIN);

   displayScrollText("ECE 298 GROUP 1");

   GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_PIN1);

   P2DIR |= 0x01; //Set P1 to output

   P2OUT = 0x00;



    while(1){

        if (GPIO_getInputPinValue(LIGHT_PORT, LIGHT_PIN)==0){
            lightsensor = false;
        }

        else {
            lightsensor = true;
        }

        if (! adcBusy){
            adcBusy = true;

            if (muxselect == 1){

                muxselect = 2;

                GPIO_setOutputHighOnPin(MUXIN1_PORT, MUXIN1_PIN);
                GPIO_setOutputLowOnPin(MUXIN0_PORT, MUXIN0_PIN);

                sensor = 1;
            }

            else if (muxselect == 2){

                muxselect = 3;

                GPIO_setOutputLowOnPin(MUXIN1_PORT, MUXIN1_PIN);
                GPIO_setOutputLowOnPin(MUXIN0_PORT, MUXIN0_PIN);

                sensor = 2;

            }

            else if (muxselect == 3){

                muxselect = 4;

                GPIO_setOutputLowOnPin(MUXIN1_PORT, MUXIN1_PIN);
                GPIO_setOutputHighOnPin(MUXIN0_PORT, MUXIN0_PIN);

                sensor = 3;

            }

            else {

                muxselect = 1;

                GPIO_setOutputHighOnPin(MUXIN1_PORT, MUXIN1_PIN);
                GPIO_setOutputHighOnPin(MUXIN0_PORT, MUXIN0_PIN);

                sensor = 4;

            }

            ADC_startConversion(ADC_BASE, ADC_SINGLECHANNEL);
        }

        if (1){

            adc_done = false;

            if (sensor == 1){                                   //Temperature sensor A

                TEMPERATUREA = ADCResult/4;

                if (TEMPERATUREA > 99){
                    TEMPERATUREA = 99;
                }

            }

            else if (sensor == 2){                              //Temperature sensor B

                TEMPERATUREB = ADCResult/4;

                if (TEMPERATUREB > 99){
                    TEMPERATUREB = 99;
                }

            }

            else if (sensor == 3){                              //Moisture A

                MOISTUREA = ADCResult/4;

                if (MOISTUREA > 99){
                    MOISTUREA = 99;
                }

            }

            else {                                              // moisture B

                MOISTUREB = ADCResult/4;

                if (MOISTUREB > 99){
                    MOISTUREB = 99;
                }


            }

        }

        if (GPIO_getInputPinValue(SW1_PORT, SW1_PIN)){
            showChar('T', pos1);
            showChar('M', pos4);
            showChar(((TEMPERATUREA/10)+'0'), pos2);
            showChar(((TEMPERATUREA%10)+'0'), pos3);
            showChar(((MOISTUREA/10)+'0'), pos5);
            showChar(((MOISTUREA%10)+'0'), pos6);
            __delay_cycles(10000);

        }

        else {
            showChar('T', pos1);
            showChar('M', pos4);
            showChar(((TEMPERATUREB/10)+'0'), pos2);
            showChar(((TEMPERATUREB%10)+'0'), pos3);
            showChar(((MOISTUREB/10)+'0'), pos5);
            showChar(((MOISTUREB%10)+'0'), pos6);
            __delay_cycles(10000);
        }

     if(UARTSIGNAL){
         uartTransmit();
     }

     if (lightsensor == true && TEMPERATUREA > TEMPERATUREATHRES && VmotorA == true){
         VENTILATIONA();
     }

     if (lightsensor == true && TEMPERATUREB > TEMPERATUREBTHRES && VmotorB == true){
         VENTILATIONB();
     }

     if (lightsensor == false && MOISTUREA < MOISTUREATHRES && ImotorA == true){
         IRRIGATIONA();
     }

     if (lightsensor == false && MOISTUREB < MOISTUREBTHRES && ImotorB == true){
         IRRIGATIONB();
     }

    }

}




void IRRIGATIONA(void) {

    P1DIR |= 0x01; //Set P1 to output

    P1OUT = 0x00; // Reset P1

    long motortimer = 25000;

    while (motortimer > 0){

        P1OUT = 0x10;

        motortimer--;

    }

    P1OUT = 0x00; // Reset P1

    return;
}

void IRRIGATIONB(void) {

    P1DIR |= 0x01; //Set P1 to output

    P1OUT = 0x00; // Reset P1

    long motortimer = 25000;

    while (motortimer > 0){

        P1OUT = 0x20;

        motortimer--;

    }

    P1OUT = 0x00; // Reset P1

    return;
}

void VENTILATIONA(void) {

    P1DIR |= 0x01; //Set P1 to output

    P1OUT = 0x00; // Reset P1

    long motortimer = 25000;

    while (motortimer > 0){

        P1OUT = 0x18;

        motortimer--;

    }

    P1OUT = 0x00; // Reset P1

    return;
}

void VENTILATIONB(void) {

    P1DIR |= 0x01; //Set P1 to output

    P1OUT = 0x00; // Reset P1

    long motortimer = 25000;

    while (motortimer > 0){

        P1OUT = 0x28;

        motortimer--;

    }

    P1OUT = 0x00; // Reset P1

    return;
}


void uartDisplay(uint8_t *sendText, uint8_t length)
{
    EUSCI_A_UART_transmitData(EUSCI_A0_BASE, 13);
    while(EUSCI_A_UART_queryStatusFlags(EUSCI_A0_BASE, EUSCI_A_UART_BUSY)) {}
    EUSCI_A_UART_transmitData(EUSCI_A0_BASE, 10);
    while(EUSCI_A_UART_queryStatusFlags(EUSCI_A0_BASE, EUSCI_A_UART_BUSY)) {}

    int i;
    for (i = 0 ; i < length ; i++)
    {
        EUSCI_A_UART_transmitData(EUSCI_A0_BASE, sendText[i]);
        while(EUSCI_A_UART_queryStatusFlags(EUSCI_A0_BASE, EUSCI_A_UART_BUSY)) {}
    }

    if ((sendText[0] != '>') && (sendText[0] != '#') && (sendText[0] != ' '))
    {
        EUSCI_A_UART_transmitData(EUSCI_A0_BASE, 13);
        while(EUSCI_A_UART_queryStatusFlags(EUSCI_A0_BASE, EUSCI_A_UART_BUSY)) {}
        EUSCI_A_UART_transmitData(EUSCI_A0_BASE, 10);
        while(EUSCI_A_UART_queryStatusFlags(EUSCI_A0_BASE, EUSCI_A_UART_BUSY)) {}
        EUSCI_A_UART_transmitData(EUSCI_A0_BASE, 10);
        while(EUSCI_A_UART_queryStatusFlags(EUSCI_A0_BASE, EUSCI_A_UART_BUSY)) {}
        EUSCI_A_UART_transmitData(EUSCI_A0_BASE, '>');
        while(EUSCI_A_UART_queryStatusFlags(EUSCI_A0_BASE, EUSCI_A_UART_BUSY)) {}
        EUSCI_A_UART_transmitData(EUSCI_A0_BASE, 32);
    }
}

void Init_GPIO(void)
{
    // Set all GPIO pins to output low to prevent floating input and reduce power consumption
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);

    //Set LaunchPad switches as inputs - they are active low, meaning '1' until pressed
    GPIO_setAsInputPinWithPullUpResistor(SW1_PORT, SW1_PIN);
    GPIO_setAsInputPinWithPullUpResistor(SW2_PORT, SW2_PIN);

    //Set LED1 and LED2 as outputs
    //GPIO_setAsOutputPin(LED1_PORT, LED1_PIN); //Comment if using UART
    GPIO_setAsOutputPin(LED2_PORT, LED2_PIN);
}

/* Clock System Initialization */
void Init_Clock(void)
{
    /*
     * The MSP430 has a number of different on-chip clocks. You can read about it in
     * the section of the Family User Guide regarding the Clock System ('cs.h' in the
     * driverlib).
     */

    /*
     * On the LaunchPad, there is a 32.768 kHz crystal oscillator used as a
     * Real Time Clock (RTC). It is a quartz crystal connected to a circuit that
     * resonates it. Since the frequency is a power of two, you can use the signal
     * to drive a counter, and you know that the bits represent binary fractions
     * of one second. You can then have the RTC module throw an interrupt based
     * on a 'real time'. E.g., you could have your system sleep until every
     * 100 ms when it wakes up and checks the status of a sensor. Or, you could
     * sample the ADC once per second.
     */
    //Set P4.1 and P4.2 as Primary Module Function Input, XT_LF
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN1 + GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);

    // Set external clock frequency to 32.768 KHz
    CS_setExternalClockSource(32768);
    // Set ACLK = XT1
    CS_initClockSignal(CS_ACLK, CS_XT1CLK_SELECT, CS_CLOCK_DIVIDER_1);
    // Initializes the XT1 crystal oscillator
    CS_turnOnXT1LF(CS_XT1_DRIVE_1);
    // Set SMCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_SMCLK, CS_DCOCLKDIV_SELECT, CS_CLOCK_DIVIDER_1);
    // Set MCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_MCLK, CS_DCOCLKDIV_SELECT, CS_CLOCK_DIVIDER_1);
}

/* UART Initialization */
void Init_UART(void)
{
    /* UART: It configures P1.0 and P1.1 to be connected internally to the
     * eSCSI module, which is a serial communications module, and places it
     * in UART mode. This let's you communicate with the PC via a software
     * COM port over the USB cable. You can use a console program, like PuTTY,
     * to type to your LaunchPad. The code in this sample just echos back
     * whatever character was received.
     */

    //Configure UART pins, which maps them to a COM port over the USB cable
    //Set P1.0 and P1.1 as Secondary Module Function Input.
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN1, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1, GPIO_PIN0, GPIO_PRIMARY_MODULE_FUNCTION);

    /*
     * UART Configuration Parameter. These are the configuration parameters to
     * make the eUSCI A UART module to operate with a 9600 baud rate. These
     * values were calculated using the online calculator that TI provides at:
     * http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
     */

    //SMCLK = 1MHz, Baudrate = 9600
    //UCBRx = 6, UCBRFx = 8, UCBRSx = 17, UCOS16 = 1
    EUSCI_A_UART_initParam param = {0};
        param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
        param.clockPrescalar    = 6;
        param.firstModReg       = 8;
        param.secondModReg      = 17;
        param.parity            = EUSCI_A_UART_NO_PARITY;
        param.msborLsbFirst     = EUSCI_A_UART_LSB_FIRST;
        param.numberofStopBits  = EUSCI_A_UART_ONE_STOP_BIT;
        param.uartMode          = EUSCI_A_UART_MODE;
        param.overSampling      = 1;

    if(STATUS_FAIL == EUSCI_A_UART_init(EUSCI_A0_BASE, &param))
    {
        return;
    }

    EUSCI_A_UART_enable(EUSCI_A0_BASE);

    EUSCI_A_UART_clearInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);

    // Enable EUSCI_A0 RX interrupt
    EUSCI_A_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
}

/* EUSCI A0 UART ISR - Echoes data back to PC host */
#pragma vector=USCI_A0_VECTOR
__interrupt
void EUSCIA0_ISR(void)
{
    uint8_t RxStatus = EUSCI_A_UART_getInterruptStatus(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);
    EUSCI_A_UART_clearInterrupt(EUSCI_A0_BASE, RxStatus);
    uint8_t received_data = EUSCI_A_UART_receiveData(EUSCI_A0_BASE);

    if ((RxStatus) && !((received_data == 0x7F) && (cliIndex == 0))) /* received correct package, and not a backspace in the begining */
    {
        EUSCI_A_UART_transmitData(EUSCI_A0_BASE, received_data); /* echo */
    }

   if (received_data == '\r') /* enter key */
        UARTSIGNAL = true;

   else if (received_data == 0x7F) /* backspace key */
   {
       if (cliIndex > 0) /* if buffer not empty */
       {
           cliIndex--;
           if (cliIndex < 15) /* within buffer range */
               cliBuffer[cliIndex] = 0; /* remove last char from buffer */
       }
   }

   else if (cliIndex < 15) /* other keys */
   {
       if ((isalpha(tolower(received_data))) || (isdigit(received_data)) || (received_data == ' ') || (received_data == '?')) /* legal keys */
           cliBuffer[cliIndex] = tolower(received_data); /* store key */
       cliIndex++;
   }
   else
       cliIndex++;
}

/* PWM Initialization */
void Init_PWM(void)
{
    /*
     * The internal timers (TIMER_A) can auto-generate a PWM signal without needing to
     * flip an output bit every cycle in software. The catch is that it limits which
     * pins you can use to output the signal, whereas manually flipping an output bit
     * means it can be on any GPIO. This function populates a data structure that tells
     * the API to use the timer as a hardware-generated PWM source.
     *
     */
    //Generate PWM - Timer runs in Up-Down mode
    param.clockSource           = TIMER_A_CLOCKSOURCE_SMCLK;
    param.clockSourceDivider    = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    param.timerPeriod           = TIMER_A_PERIOD; //Defined in main.h
    param.compareRegister       = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    param.compareOutputMode     = TIMER_A_OUTPUTMODE_RESET_SET;
    param.dutyCycle             = HIGH_COUNT; //Defined in main.h

    //PWM_PORT PWM_PIN (defined in main.h) as PWM output
    GPIO_setAsPeripheralModuleFunctionOutputPin(PWM_PORT, PWM_PIN, GPIO_PRIMARY_MODULE_FUNCTION);
}

void Init_ADC(void)
{
    /*
     * To use the ADC, you need to tell a physical pin to be an analog input instead
     * of a GPIO, then you need to tell the ADC to use that analog input. Defined
     * these in main.h for A9 on P8.1.
     */

    //Set ADC_IN to input direction
    GPIO_setAsPeripheralModuleFunctionInputPin(ADC_IN_PORT, ADC_IN_PIN, GPIO_PRIMARY_MODULE_FUNCTION);

    //Initialize the ADC Module
    /*
     * Base Address for the ADC Module
     * Use internal ADC bit as sample/hold signal to start conversion
     * USE MODOSC 5MHZ Digital Oscillator as clock source
     * Use default clock divider of 1
     */
    ADC_init(ADC_BASE,
             ADC_SAMPLEHOLDSOURCE_SC,
             ADC_CLOCKSOURCE_ADCOSC,
             ADC_CLOCKDIVIDER_1);

    ADC_enable(ADC_BASE);

    /*
     * Base Address for the ADC Module
     * Sample/hold for 16 clock cycles
     * Do not enable Multiple Sampling
     */
    ADC_setupSamplingTimer(ADC_BASE,
                           ADC_CYCLEHOLD_16_CYCLES,
                           ADC_MULTIPLESAMPLESDISABLE);

    //Configure Memory Buffer
    /*
     * Base Address for the ADC Module
     * Use input ADC_IN_CHANNEL
     * Use positive reference of AVcc
     * Use negative reference of AVss
     */
    ADC_configureMemory(ADC_BASE,
                        ADC_IN_CHANNEL,
                        ADC_VREFPOS_AVCC,
                        ADC_VREFNEG_AVSS);

    ADC_clearInterrupt(ADC_BASE,
                       ADC_COMPLETED_INTERRUPT);

    //Enable Memory Buffer interrupt
    ADC_enableInterrupt(ADC_BASE,
                        ADC_COMPLETED_INTERRUPT);
}

//ADC interrupt service routine
#pragma vector=ADC_VECTOR
__interrupt
void ADC_ISR(void)
{
    uint8_t ADCStatus = ADC_getInterruptStatus(ADC_BASE, ADC_COMPLETED_INTERRUPT_FLAG);

    ADC_clearInterrupt(ADC_BASE, ADCStatus);

    if (ADCStatus)
    {
        adcBusy = false;
        ADCState = 0; //Not busy anymore
        ADCResult = ADC_getResults(ADC_BASE);
        adc_done = true;

    }
}

void uartTransmit(void)
{
    UARTSIGNAL = false;
    uint8_t txMsg[100];

    int i;
    for (i = 0 ; i < 50 ; i++)
        txMsg[i] = 0;

    if (! strcmp((char*)cliBuffer, ""))
    {
        strcpy((char*) txMsg, "> ");
        uartDisplay(txMsg, strlen((char*) txMsg));
    }

    else if ((cliBuffer[0] == 's') && (cliBuffer[1] == 'e') && (cliBuffer[2] == 't'))
    {
        int newThreshold = -1;
        if (isdigit(cliBuffer[9]))
            newThreshold = (cliBuffer[7] - 48)*100 + (cliBuffer[8] - 48)*10 + (cliBuffer[9] - 48);
        else if (isdigit(cliBuffer[8]))
            newThreshold = (cliBuffer[7] - 48)*10 + (cliBuffer[8] - 48);
        else if (isdigit(cliBuffer[7]))
            newThreshold = (cliBuffer[7] - 48);

        if ((newThreshold >= 0) && (newThreshold <= 100))
        {
            if ((cliBuffer[4] == 't') && (cliBuffer[5] == '1'))
            {
                TEMPERATUREATHRES = newThreshold;
                strcpy((char*) txMsg, "Temperature Zone A threshold set");
                uartDisplay(txMsg, strlen((char*) txMsg));
            }
            else if ((cliBuffer[4] == 'm') && (cliBuffer[5] == '1'))
            {
                MOISTUREATHRES = newThreshold;
                strcpy((char*) txMsg, "Moisture Zone A threshold set");
                uartDisplay(txMsg, strlen((char*) txMsg));
            }
            else if ((cliBuffer[4] == 't') && (cliBuffer[5] == '2'))
            {
                TEMPERATUREBTHRES = newThreshold;
                strcpy((char*) txMsg, "Temperature Zone B threshold set");
                uartDisplay(txMsg, strlen((char*) txMsg));
            }
            else if ((cliBuffer[4] == 'm') && (cliBuffer[5] == '2'))
            {
                MOISTUREBTHRES = newThreshold;
                strcpy((char*) txMsg, "Moisture Zone B threshold set");
                uartDisplay(txMsg, strlen((char*) txMsg));
            }
            else
            {
                strcpy((char*) txMsg, "ERROR");
                uartDisplay(txMsg, strlen((char*) txMsg));
            }
        }
        else
        {
            strcpy((char*) txMsg, "ERROR");
            uartDisplay(txMsg, strlen((char*) txMsg));
        }
    }

    else if (! strcmp((char*)cliBuffer, "motor t1 on"))
    {
        VmotorA = true;
        strcpy((char*) txMsg, "Temperature Zone A is enabled");
        uartDisplay(txMsg, strlen((char*) txMsg));
    }

    else if (! strcmp((char*)cliBuffer, "motor t1 off"))
    {
        VmotorA = false;
        strcpy((char*) txMsg, "Temperature Zone A is disabled");
        uartDisplay(txMsg, strlen((char*) txMsg));
    }

    else if (! strcmp((char*)cliBuffer, "motor m1 on"))
    {
        ImotorA = true;
        strcpy((char*) txMsg, "Moisture Zone A is enabled");
        uartDisplay(txMsg, strlen((char*) txMsg));
    }

    else if (! strcmp((char*)cliBuffer, "motor m1 off"))
    {
        ImotorA = false;
        strcpy((char*) txMsg, "Moisture Zone A is disabled");
        uartDisplay(txMsg, strlen((char*) txMsg));
    }

    else if (! strcmp((char*)cliBuffer, "motor t2 on"))
    {
        VmotorB = true;
        strcpy((char*) txMsg, "Temperature Zone B is enabled");
        uartDisplay(txMsg, strlen((char*) txMsg));
    }

    else if (! strcmp((char*)cliBuffer, "motor t2 off"))
    {
        VmotorB = false;
        strcpy((char*) txMsg, "Temperature Zone B is disabled");
        uartDisplay(txMsg, strlen((char*) txMsg));
    }

    else if (! strcmp((char*)cliBuffer, "motor m2 on"))
    {
        ImotorB = true;
        strcpy((char*) txMsg, "Moisture Zone B is enabled");
        uartDisplay(txMsg, strlen((char*) txMsg));
    }
    else if (! strcmp((char*)cliBuffer, "motor m2 off"))
    {
        ImotorB = false;
        strcpy((char*) txMsg, "Moisture Zone B is disabled");
        uartDisplay(txMsg, strlen((char*) txMsg));
    }

    else
    {
        strcpy((char*) txMsg, "ERROR");
        uartDisplay(txMsg, strlen((char*) txMsg));
    }

    cliIndex = 0;
    for (i = 0 ; i < 15 ; i++)
        cliBuffer[i] = 0;
}
2
