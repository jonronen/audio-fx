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
  pmc_enable_periph_clk(ID_SSC);
  
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
  
  SSC->SSC_CR = SSC_CR_SWRST;
  SSC->SSC_CMR = 0;
  SSC->SSC_RCMR = 0;
  SSC->SSC_RFMR = 0;
  SSC->SSC_TCMR = 0;
  SSC->SSC_TFMR = 0;
  
  // set the clock divider to provide ~1MHz
  SSC->SSC_CMR = 40;
  
  SSC->SSC_TCMR = SSC_TCMR_STTDLY(1) | SSC_TCMR_START_CONTINUOUS | SSC_TCMR_CKG_CONTINUOUS | SSC_TCMR_CKO_TRANSFER | SSC_TCMR_PERIOD(23);
  SSC->SSC_RCMR = SSC_RCMR_STTDLY(1) | SSC_RCMR_START_TRANSMIT | SSC_RCMR_CKS_TK | SSC_RCMR_CKO_NONE | SSC_RCMR_PERIOD(23);
  SSC->SSC_TFMR = SSC_TFMR_DATLEN(23) | SSC_TFMR_MSBF | SSC_TFMR_DATNB(1) | SSC_TFMR_FSOS_TOGGLING;
  SSC->SSC_RFMR = SSC_RFMR_DATLEN(23) | SSC_RFMR_MSBF | SSC_RFMR_DATNB(1) | SSC_RFMR_FSOS_TOGGLING;
  
  Serial.begin(115200);
  
  SSC->SSC_CR = SSC_CR_RXEN;
  SSC->SSC_CR = SSC_CR_TXEN;
}

void loop()
{
  *((uint32_t*)&SSC->SSC_THR) = 0x55AA5A;
  while ((SSC->SSC_SR & SSC_SR_TXRDY) == 0);
  *((uint32_t*)&SSC->SSC_THR) = 0x123456;
  while ((SSC->SSC_SR & SSC_SR_TXRDY) == 0);
  *((uint32_t*)&SSC->SSC_THR) = 0x789ABC;
  while ((SSC->SSC_SR & SSC_SR_TXRDY) == 0);
  *((uint32_t*)&SSC->SSC_THR) = 0xFFFFFF;
  Serial.println((int)SSC->SSC_RHR);
  delay(500);
}

