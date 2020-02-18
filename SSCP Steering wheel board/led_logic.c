/*
* LED update task written for the Stanford Solar Car Project
*/

void LED_InitGpios(){
  
  for(LED_Index led = CruiseDown_LED; led <= Power_LED; led++)
    Pin_ConfigGpioPin(&(leds[led]),
                      GPIO_Mode_OUT,
                      GPIO_Speed_2MHz,
                      GPIO_OType_PP,
                      GPIO_PuPd_DOWN,
                      false);
  
  
  // Initialization, but for Hard Fault LED and State LED. 
  for(LED_Index led = HardFault_LED; led <= Debug_LED; led++)
    Pin_ConfigGpioPin(&(leds [ led ] ),
                      GPIO_Mode_OUT,
                      GPIO_Speed_2MHz,
                      GPIO_OType_PP,
                      GPIO_PuPd_DOWN,
                      false);
}

void LED_UpdateTask(void *data){
  portTickType xLastWakeTime = xTaskGetTickCount();
  LED_InitGpios();
  static const int NUM_BUT_LEDS = 9;
  SteerButtonState* sw_state = &message.sw_state.button_state;
  
  while(true)
  {
    Pin_SetHigh(&(leds[9])); // Turns on power button
    for(int a = 0; a < NUM_BUT_LEDS; a++)
    {
      if(sw_state->led_state & (1 << a))
      {
        Pin_SetHigh(&(leds[a]));
      }
      else 
      {
        Pin_SetLow(&(leds[a]));
      }              
    }
//    vTaskDelayUntil(&xLastWakeTime, 10);
    vTaskDelay(50);
  }
}