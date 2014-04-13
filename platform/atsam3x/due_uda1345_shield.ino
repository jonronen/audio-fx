#define NUM_CHANNELS 2

// test
static uint32_t g_buff[NUM_CHANNELS][1024];
static unsigned int g_rx_buff_index;
static unsigned int g_tx_buff_index;

// Make sure that data is first pin in the list.
const PinDescription SSCTXPins[]=
{
  { PIOA, PIO_PA16B_TD, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT, PIN_ATTR_DIGITAL, NO_ADC, NO_ADC, NOT_ON_PWM, NOT_ON_TIMER }, // A0
  { PIOA, PIO_PA15B_TF, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT, PIN_ATTR_DIGITAL, NO_ADC, NO_ADC, NOT_ON_PWM, NOT_ON_TIMER }, // PIN 24
  { PIOA, PIO_PA14B_TK, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT, PIN_ATTR_DIGITAL, NO_ADC, NO_ADC, NOT_ON_PWM, NOT_ON_TIMER }, // PIN 23
};

// Make sure that data is first pin in the list.
const PinDescription SSCRXPins[]=
{
  { PIOB, PIO_PB18A_RD, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT, PIN_ATTR_DIGITAL, NO_ADC, NO_ADC, NOT_ON_PWM, NOT_ON_TIMER }, // A9
  { PIOB, PIO_PB17A_RF, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT, PIN_ATTR_DIGITAL, NO_ADC, NO_ADC, NOT_ON_PWM, NOT_ON_TIMER }, // A8
  { PIOB, PIO_PB19A_RK, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT, PIN_ATTR_DIGITAL, NO_ADC, NO_ADC, NOT_ON_PWM, NOT_ON_TIMER }, // A10
};


// the pin we use for capturing the master clock (12.288 MHz)
const PinDescription TCLK5Pin = // DAC1
  { PIOB, PIO_PB16A_TCLK5, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT, PIN_ATTR_DIGITAL, NO_ADC, NO_ADC, NOT_ON_PWM, NOT_ON_TIMER };
//  { PIOB, PIO_PB26B_TCLK0, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT, PIN_ATTR_DIGITAL, NO_ADC, NO_ADC, NOT_ON_PWM, NOT_ON_TIMER };
//  { PIOA, PIO_PA4A_TCLK1, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT, PIN_ATTR_DIGITAL, NO_ADC, NO_ADC, NOT_ON_PWM, NOT_ON_TIMER };


void setup()
{
  // enable clocks for the peripherals we're using
  pmc_enable_periph_clk(ID_SSC);
  pmc_enable_periph_clk(ID_TC5);
  
  // set the relevant SSC pins for I2S (not analog or digital I/Os)
  for (int i=0; i<3; i++) {
    PIO_Configure(SSCTXPins[i].pPort,
      SSCTXPins[i].ulPinType,
      SSCTXPins[i].ulPin,
      SSCTXPins[i].ulPinConfiguration);
    PIO_Configure(SSCRXPins[i].pPort,
      SSCRXPins[i].ulPinType,
      SSCRXPins[i].ulPin,
      SSCRXPins[i].ulPinConfiguration);
  }
  
  // set the TCLK5 pin for capturing the master I2S clock
  PIO_Configure(TCLK5Pin.pPort,
      TCLK5Pin.ulPinType,
      TCLK5Pin.ulPin,
      TCLK5Pin.ulPinConfiguration);
  
  //
  // setup timer1 for an interrupt with external clock
  //

  // use TC1 with timer #0 and external clock #2,
  // which means base freq is determined by an external oscillator
  TC_Configure(TC1, 2, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_XC2);
  TC_SetRC(TC1, 2, 256); // assuming I2S master clock is 256f
  // Enable the RC Compare Interrupt...
  TC1->TC_CHANNEL[2].TC_IER=TC_IER_CPCS;
  // ... and disable all others.
  TC1->TC_CHANNEL[2].TC_IDR=~TC_IER_CPCS;

  // clear the interrupt
  NVIC_ClearPendingIRQ(TC5_IRQn);
  NVIC_EnableIRQ(TC5_IRQn);
  
  //
  // setup SSC for I2S master, 24-bit stereo, simultaneous send/receive
  //
  
  SSC->SSC_CR = SSC_CR_SWRST;
  SSC->SSC_CMR = 0;
  SSC->SSC_RCMR = 0;
  SSC->SSC_RFMR = 0;
  SSC->SSC_TCMR = 0;
  SSC->SSC_TFMR = 0;
  
  // set the clock divider to provide 4.66MHz
  SSC->SSC_CMR = 9;
  
  //
  // Special notes about the values in these registers:
  // 1. CKI bit is on for RX and off for TX.
  //    This is because data samples should be shifted out
  //    on the clock's falling edge and sampled on rising edge.
  // 2. Clocks are defined to be the same: both outputs
  //    (but one of them is ignored anyway)
  // 3. The word select line of TX is connected both to the
  //    codec and to the word select of RX. That's why TFMR
  //    is output and RFMR is input, and that's why RCMR is
  //    defined with START_RF_EDGE.
  //
  SSC->SSC_TCMR = SSC_TCMR_START_CONTINUOUS | SSC_TCMR_CKS_MCK | SSC_TCMR_CKO_CONTINUOUS | SSC_TCMR_CKG_CONTINUOUS;
  SSC->SSC_RCMR = SSC_RCMR_START_RF_EDGE | SSC_RCMR_CKI | SSC_RCMR_CKS_MCK | SSC_RCMR_CKO_CONTINUOUS | SSC_RCMR_CKG_CONTINUOUS | SSC_RCMR_STTDLY(1);
  SSC->SSC_TFMR = SSC_TFMR_DATLEN(23) | SSC_TFMR_FSOS_TOGGLING | SSC_TFMR_MSBF;
  SSC->SSC_RFMR = SSC_RFMR_DATLEN(23) | SSC_RFMR_MSBF;
  
  // clear the interrupt
  NVIC_ClearPendingIRQ(SSC_IRQn);
  
  Serial.begin(115200);
  
  g_rx_buff_index = 0;
  g_tx_buff_index = 0;
  
  // enable the SSC send/receive
  SSC->SSC_CR = SSC_CR_RXEN;
  SSC->SSC_CR = SSC_CR_TXEN;
  
  // and start the timer
  TC_Start(TC1, 2);
}

void loop()
{
  int mx[NUM_CHANNELS] = {0, 0};
  int mn[NUM_CHANNELS] = {0x7fffff, 0x7fffff};
  int avg[NUM_CHANNELS] = {0,0};
  int i;
  
  for (i=0; i<1024; i++) {
    if (g_buff[0][i] > mx[0]) mx[0] = g_buff[0][i];
    if (g_buff[1][i] > mx[1]) mx[1] = g_buff[1][i];
    if (g_buff[0][i] < mn[0]) mn[0] = g_buff[0][i];
    if (g_buff[1][i] < mn[1]) mn[1] = g_buff[1][i];
    avg[0] += g_buff[0][i] / 256;
    avg[1] += g_buff[1][i] / 256;
  }
  Serial.println(mx[0], HEX);
  Serial.println(mn[0], HEX);
  Serial.println(mx[1], HEX);
  Serial.println(mn[1], HEX);
  Serial.println(avg[0]/4, HEX);
  Serial.println(avg[1]/4, HEX);
  //Serial.println(SSC->SSC_SR, HEX);
  Serial.println(" ");
  delay(500);
}

void TC5_Handler()
{
  // clear the interrupt bit
  TC_GetStatus(TC1, 2);

  // write left channel
  SSC->SSC_THR = g_buff[0][(g_tx_buff_index+1024-16) % 1024];
  
  // read left channel
  g_buff[0][g_rx_buff_index] = SSC->SSC_RHR;
  
  while (ssc_is_tx_ready(SSC) != SSC_RC_YES);
  
  // write right channel
  SSC->SSC_THR = g_buff[1][(g_tx_buff_index+1024-16) % 1024];

  g_tx_buff_index++;
  if (g_tx_buff_index >= 1024) g_tx_buff_index = 0;
  
  // read right channel
  g_buff[1][g_rx_buff_index] = SSC->SSC_RHR;
  
  g_rx_buff_index++;
  if (g_rx_buff_index >= 1024) g_rx_buff_index = 0;
}

