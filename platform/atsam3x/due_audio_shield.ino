#define NUM_CHANNELS 2

int g_recieve[NUM_CHANNELS];
int g_send[NUM_CHANNELS];

// test
int g_rcv_buff[NUM_CHANNELS][1024];
unsigned int g_rcv_buff_index;
unsigned char g_channel_index;
unsigned char g_phase_cnt;
unsigned char g_volumes[NUM_CHANNELS];
unsigned char g_volume_cnt;

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
  
  // set the clock divider to provide 4.66MHz
  SSC->SSC_CMR = 9;
  
  SSC->SSC_TCMR = SSC_TCMR_START_CONTINUOUS | SSC_TCMR_CKI | SSC_TCMR_CKG_CONTINUOUS | SSC_TCMR_CKO_TRANSFER | SSC_TCMR_PERIOD(1);
  SSC->SSC_RCMR = SSC_RCMR_START_TRANSMIT | SSC_RCMR_CKI | SSC_RCMR_CKS_TK | SSC_RCMR_PERIOD(1);
  SSC->SSC_TFMR = SSC_TFMR_DATLEN(23) | SSC_TFMR_MSBF | SSC_TFMR_FSOS_TOGGLING;
  SSC->SSC_RFMR = SSC_RFMR_DATLEN(23) | SSC_RFMR_MSBF | SSC_RFMR_FSOS_TOGGLING;
  
  Serial.begin(115200);
  
  g_rcv_buff_index = 0;
  g_channel_index = 0;
  g_volume_cnt = 0;
  g_phase_cnt = 0;
  
  for (int i=0; i<NUM_CHANNELS; i++) {
    g_volumes[i] = 0;
  }
  
  // enable the SSC send/receive
  SSC->SSC_CR = SSC_CR_RXEN;
  SSC->SSC_CR = SSC_CR_TXEN;
  
  // and start the timer
  TC_Start(TC0, 0);
}

void loop()
{
  int mx[NUM_CHANNELS] = {-0x1000000, -0x1000000};
  int mn[NUM_CHANNELS] = {0x1000000, 0x1000000};
  int avg[NUM_CHANNELS] = {0,0};
  int i;
  
  for (i=0; i<1024; i++) {
    if (g_rcv_buff[0][i] > mx[0]) mx[0] = g_rcv_buff[0][i];
    if (g_rcv_buff[1][i] > mx[1]) mx[1] = g_rcv_buff[1][i];
    if (g_rcv_buff[0][i] < mn[0]) mn[0] = g_rcv_buff[0][i];
    if (g_rcv_buff[1][i] < mn[1]) mn[1] = g_rcv_buff[1][i];
    avg[0] += g_rcv_buff[0][i] / 256;
    avg[1] += g_rcv_buff[1][i] / 256;
  }
  Serial.println(mx[0], HEX);
  Serial.println(mn[0], HEX);
  Serial.println(mx[1], HEX);
  Serial.println(mn[1], HEX);
  Serial.println(avg[0]/4, HEX);
  Serial.println(avg[1]/4, HEX);
  Serial.println(SSC->SSC_SR, HEX);
  Serial.println(" ");
  delay(500);
}

void TC0_Handler()
{
  int vals[NUM_CHANNELS] = {0,0};
  
  // clear the interrupt bit
  TC_GetStatus(TC0, 0);
  
  // prepare the values for each channel
  vals[0] = 0;
  vals[1] = 0;
  
  // just a test to see we're in business
  if (g_phase_cnt & 0x20) {
    vals[0] = 0x1ffff;
    vals[1] = 0x1ffff;
  }
  
  g_volumes[0] = g_volume_cnt;
  g_volumes[1] = 0xff - g_volumes[0];
  
  g_phase_cnt++;
  if (g_phase_cnt == 0) g_volume_cnt++;
  
  vals[0] = vals[0] * g_volumes[0] / 256;
  vals[1] = vals[1] * g_volumes[1] / 256;
  
  // write the value twice (once for each channel)
  while ((SSC->SSC_SR & SSC_SR_TXRDY) == 0);
  //SSC->SSC_THR = vals[0];
  SSC->SSC_THR = g_rcv_buff[0][g_rcv_buff_index];
  while ((SSC->SSC_SR & SSC_SR_RXRDY) == 0);
  g_rcv_buff[0][g_rcv_buff_index] = SSC->SSC_RHR;
  while ((SSC->SSC_SR & SSC_SR_TXRDY) == 0);
  //SSC->SSC_THR = vals[1];
  SSC->SSC_THR = g_rcv_buff[1][g_rcv_buff_index];
  while ((SSC->SSC_SR & SSC_SR_RXRDY) == 0);
  g_rcv_buff[1][g_rcv_buff_index++] = SSC->SSC_RHR;
  
  if (g_rcv_buff_index >= 1024) g_rcv_buff_index = 0;
}

