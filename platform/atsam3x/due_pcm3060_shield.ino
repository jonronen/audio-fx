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


void setup()
{
  // enable clocks for the peripherals we're using
  pmc_enable_periph_clk(ID_SSC);
  
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
  // setup SSC for I2S slave, 24-bit stereo, simultaneous send/receive
  //
  
  SSC->SSC_CR = SSC_CR_SWRST;
  SSC->SSC_CMR = 0;
  SSC->SSC_RCMR = 0;
  SSC->SSC_RFMR = 0;
  SSC->SSC_TCMR = 0;
  SSC->SSC_TFMR = 0;
  
  //// set the clock divider to provide 4.66MHz
  //SSC->SSC_CMR = 9;
  // No clock divider is needed if the codec is working in master mode
  SSC->SSC_CMR = 0;
  
  //
  // Special notes about the values in these registers:
  // 1. CKI bit is on for RX and off for TX.
  //    This is because data samples should be shifted out
  //    on the clock's falling edge and sampled on rising edge.
  // 2. STTDLY is zero for TX and one for RX.
  //    This is because TX samples RF on the clock's rising edge
  //    and shifts out data on the clock's falling edge.
  //
  SSC->SSC_TCMR = SSC_TCMR_START_RF_EDGE | SSC_TCMR_CKS_RK | SSC_TCMR_STTDLY(1);
  SSC->SSC_RCMR = SSC_RCMR_START_RF_EDGE | SSC_RCMR_CKI | SSC_RCMR_CKS_RK | SSC_RCMR_STTDLY(1);
  SSC->SSC_TFMR = SSC_TFMR_DATLEN(23) | SSC_TFMR_MSBF;
  SSC->SSC_RFMR = SSC_RFMR_DATLEN(23) | SSC_RFMR_MSBF;
  
  // interrupt when RX is ready
  SSC->SSC_IER = SSC_IER_TXRDY | SSC_IER_RXRDY;
  // clear the interrupt
  NVIC_ClearPendingIRQ(SSC_IRQn);
  NVIC_EnableIRQ(SSC_IRQn);
  
  Serial.begin(115200);
  
  g_rx_buff_index = 0;
  g_tx_buff_index = 0;
  
  // enable the SSC send/receive
  SSC->SSC_CR = SSC_CR_RXEN;
  SSC->SSC_CR = SSC_CR_TXEN;
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

void SSC_Handler()
{
  // write the TX
  if (ssc_is_tx_ready(SSC) == SSC_RC_YES) {
    SSC->SSC_THR = g_buff[0][(g_tx_buff_index+1024-16) % 1024];
    g_tx_buff_index++;
    if (g_tx_buff_index >= 1024) g_tx_buff_index = 0;
  }
  
  // read the RX
  if (ssc_is_rx_ready(SSC) == SSC_RC_YES) {
    //ssc_read(SSC, &g_rcv_buff[0][g_rcv_buff_index]);
    g_buff[0][g_rx_buff_index] = SSC->SSC_RHR;
    g_rx_buff_index++;
    if (g_rx_buff_index >= 1024) g_rx_buff_index = 0;
  }
}

