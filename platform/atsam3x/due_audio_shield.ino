unsigned int g_timer_cnt;
int g_rcv_buff[1024];
unsigned int g_rcv_buff_index;

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


void setup()
{
  // enable clocks for the peripherals we're using
  pmc_enable_periph_clk(ID_SSC);
  pmc_enable_periph_clk(ID_TC0);
  
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
  
  //
  // setup timer0 for an interrupt with 48KHz frequency
  //
  
  // use TC0 with timer #0 and clock #1, which means base freq is F_CPU/2
  TC_Configure(TC0, 0, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK1);
  TC_SetRC(TC0, 0, 875); // 84MHz / 48KHz / 2
  // Enable the RC Compare Interrupt...
  TC0->TC_CHANNEL[0].TC_IER=TC_IER_CPCS;
  // ... and disable all others.
  TC0->TC_CHANNEL[0].TC_IDR=~TC_IER_CPCS;
  
  // clear the interrupt
  NVIC_ClearPendingIRQ(TC0_IRQn);
  NVIC_EnableIRQ(TC0_IRQn);
  
  //
  // setup SSC for I2S master, 24-bit stereo, simultaneous send/receive
  //
  
  SSC->SSC_CR = SSC_CR_SWRST;
  SSC->SSC_CMR = 0;
  SSC->SSC_RCMR = 0;
  SSC->SSC_RFMR = 0;
  SSC->SSC_TCMR = 0;
  SSC->SSC_TFMR = 0;
  
  // set the clock divider to provide ~1MHz
  SSC->SSC_CMR = 10;
  
  SSC->SSC_TCMR = SSC_TCMR_STTDLY(1) | SSC_TCMR_START_CONTINUOUS | SSC_TCMR_CKG_CONTINUOUS | SSC_TCMR_CKO_TRANSFER | SSC_TCMR_PERIOD(23);
  SSC->SSC_RCMR = SSC_RCMR_STTDLY(1) | SSC_RCMR_START_TRANSMIT | SSC_RCMR_CKS_TK | SSC_RCMR_CKO_NONE | SSC_RCMR_PERIOD(23);
  SSC->SSC_TFMR = SSC_TFMR_DATLEN(23) | SSC_TFMR_MSBF | SSC_TFMR_DATNB(1) | SSC_TFMR_FSOS_TOGGLING;
  SSC->SSC_RFMR = SSC_RFMR_DATLEN(23) | SSC_RFMR_MSBF | SSC_RFMR_DATNB(1) | SSC_RFMR_FSOS_TOGGLING;
  
  Serial.begin(115200);
  
  g_rcv_buff_index = 0;
  
  // enable the SSC send/receive
  SSC->SSC_CR = SSC_CR_RXEN;
  SSC->SSC_CR = SSC_CR_TXEN;
  
  // and start the timer
  TC_Start(TC0, 0);
}

void loop()
{
  int mx = -0x7ffff;
  int mn = 0x7ffff;
  int i;
  
  for (i=0; i<1024; i++) {
    if (g_rcv_buff[i] > mx) mx = g_rcv_buff[i];
    if (g_rcv_buff[i] < mn) mn = g_rcv_buff[i];
  }
  Serial.println(mx, HEX);
  Serial.println(mn, HEX);
  Serial.println(" ");
  delay(500);
}

void TC0_Handler()
{
  int val = 0;
  
  // clear the interrupt bit
  TC_GetStatus(TC0, 0);
  
  // just a test to see we're in business
  g_timer_cnt++;
  if (g_timer_cnt & 0x20) {
    val = 0x3ffff;
  }
  
  // write the value twice
  while ((SSC->SSC_SR & SSC_SR_TXRDY) == 0);
  SSC->SSC_THR = val;
  g_rcv_buff[g_rcv_buff_index++] = SSC->SSC_RHR;
  while ((SSC->SSC_SR & SSC_SR_TXRDY) == 0);
  SSC->SSC_THR = val;
  g_rcv_buff[g_rcv_buff_index++] = SSC->SSC_RHR;
  
  if (g_rcv_buff_index >= 1024) g_rcv_buff_index = 0;
}

