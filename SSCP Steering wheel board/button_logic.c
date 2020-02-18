/*
* Button update task written for the Stanford Solar Car Team. My work includes
* 1. ReadButtonState - Reads driver button input and populates Protobuff
* 2. UpdateSWButtonState - Updates the button pressed field for each of the buttons in our car
*/

/*Initialize buttons*/
void BUT_InitGpios(){
  for(int button_address = 0; button_address < STEER_NUMBUTTONS; button_address++) {
    Pin_ConfigGpioPin(buttons + button_address,
		      GPIO_Mode_IN,
		      GPIO_Speed_2MHz,
		      GPIO_OType_OD,
		      GPIO_PuPd_NOPULL,
		      false);
  }
}

static uint32_t ReadButtonState()
{
  uint32_t button_state = 0;
  for(int bit = 0; bit < STEER_NUMBUTTONS; bit++)
  {
    bool button_on = Pin_ReadValue(buttons + bit); // TRUE if pin is high.
    
    // Note that we use a pull-up resistor on our buttons. In practice, this means that,
    // while our buttons are not pressed, we have a HIGH input. Therefore, we have a
    // LOW pin read value when the button is pressed.
    /* The end result is "If the button is pressed -> TRUE". */
    uint32_t read_button_value = (button_on ? 0 : 1) << bit;
    
    // Update the button by bitwise-OR'ing them.
    button_state = button_state | read_button_value;
  }
  
  return button_state;
}

void BUT_UpdateTask(void *data) 
{  
  portTickType xLastWakeTime = xTaskGetTickCount(); //Useful for indicator
  //BUT_InitGpios();  
  ButtonState state = {.current_button = 0,
  .previous_button = 0,
  .buttons_changed = 0,
  .buttons_changed_on = 0,
  .buttons_changed_off = 0};
  
  SteerButtonState local_button_status = SteerButtonState_init_default;
  while(true)
  {
    state.current_button = ReadButtonState();
    
    /*Check changed button state*/
    UpdateSWButtonState(&local_button_status, state.current_button); 
    
    //lock?
    bool locked = LOCK_SWState(&protected_message);
    if(!locked)
      continue; 
    
    /* Reassign button state. */
    message.sw_state.button_state = local_button_status;
    /* Updates LED state */
    message.sw_state.button_state.led_state = state.current_button;
    
    UNLOCK_SWState(&protected_message);
    
    /* Update state for next iteration. */
    state.previous_button = state.current_button;
    
  }
}

void UpdateSWButtonState(SteerButtonState* button_status, 
                         uint32_t pressed_buttons)
{   
  button_status->cruise_down_on = ButtonEQ(pressed_buttons, BUT_CRUISE_DOWN);
  button_status->cruise_up_on = ButtonEQ(pressed_buttons, BUT_CRUISE_UP);
  
  button_status->cruise_resume_on =  ButtonEQ(pressed_buttons, BUT_CRUISE_RESUME);
  button_status->rearview_on = ButtonEQ(pressed_buttons, BUT_Rearview);
  button_status->power_save_on = false; //:) //ButtonEQ(pressed_buttons, BUT_Powersave);
  button_status->left_turn_on = ButtonEQ(pressed_buttons, BUT_LeftTurn);
  button_status->right_turn_on = ButtonEQ(pressed_buttons, BUT_RightTurn);
  
  /* Note on Sunrise vs Sundae compatibility. In the Sundae board we use to test
  software, the headlights and reverse button are flipped, which lead to hard-
  to-debug logic errors. This uses the SUNRISE_SW and SUNDAE_SW #defines to 
  correctly choose which button configuration we're using. */

#if SUNDAE_SW
  button_status->reverse_on    = ButtonEQ(pressed_buttons, BUT_Reverse);
  button_status->headlights_on = ButtonEQ(pressed_buttons, BUT_Headlight);
#elif SUNRISE_SW
  button_status->reverse_on    = ButtonEQ(pressed_buttons, BUT_Headlight);
  button_status->headlights_on = ButtonEQ(pressed_buttons, BUT_Reverse);
#endif
}

bool ButtonEQ(Button input, ButtonId comparison)
{
  // Each bit represents one button. Therefore, if we bitwise AND
  // the current button state with the corresponding bit for a particular
  // button, we get zero if that button isn't on and a high bit otherwise. 
  return (input & (1 << comparison)) != 0;
}