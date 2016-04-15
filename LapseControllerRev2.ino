#include <MsTimer2.h>
#include <SPI.h>
#include <EEPROM.h>
#include "Ucglib.h"

// Current Version
#define FIRMWARE_VERSION              "Rev2.0"
#define FIRMWARE_RELEASE_DATE        __DATE__
#define FIRMWARE_BUILD               __TIME__
#define FIRMWARE_DEVELOPER1          "Mario Contreras"

//Pin definitions
Ucglib_ST7735_18x128x160_HWSPI ucg(/*cd=*/ 9 , /*cs=*/ 10, /*reset=*/ 8);

#define SHUTTER        2
#define SLEEP          5
#define STEP           6
#define DIR            7
#define FOCUS          8
#define BUTTON         A0
#define WHISKER2       A2
#define WHISKER1       A3
#define X_VALUE        A4
#define Y_VALUE        A5
#define VOLTAGE        A6

//Const definitions
const int k_joystick_down = -1;
const int k_joystick_up = 1;
const int k_joystick_left = -2;
const int k_joystick_right = 2;
const int k_analogbutton1_pressed = 3;
const int k_analogbutton2_pressed = 4;
const int k_analogbutton3_pressed = 5;
const int k_dummy_event_call = 6;
const int k_focus_delay = 10;

const int k_high_speed = 0;    //only for positioning purposes, may skip steps
const int k_normal_speed = 1;  //maximun tested speed that ensure no steps are skip

const int k_away_from_motor = 1;
const int k_toward_motor = 0;

const unsigned int k_min_idle_time = 250;
const unsigned int k_steps_per_cm = 1300;

//Const definition - Screen related
const int k_font_height = 8;
const int k_font_width = 6;
const int k_title_x = 2;
const int k_title_y = 42;
const int k_settings_titles_x = 4;
const int k_settings_titles_y = 56;
const int k_settings_values_x = 68;
const int k_settings_line_spacing = k_font_height + 2;
const int k_settings_values_y = k_settings_titles_y;
const int k_battery_x = 100;
const int k_battery_y = k_title_y;

//Const defintion for Screen Re-draw
const int k_redraw_voltage = 1;
const int k_redraw_all_screen = 2;
const int k_redraw_settings_values = 3;
const int k_hide_settings = 4;
const int k_show_settings = 5;

//All settings available & spare slots
enum Settings {
  //Manual positioning settings
  /* 0  */  k_manualpositioning_mode,
  /* 1  */  k_manualpositioning_maxspeed,
  /* 2  */  k_manualpositioning_ramp,
  /* 3  */  k_manualpositioning_spare3,
  /* 4  */  k_manualpositioning_spare4,
  /* 5  */  k_manualpositioning_spare5,
  /* 6  */  k_manualpositioning_spare6,
  /* 7  */  k_manualpositioning_spare7,
  /* 8  */  k_manualpositioning_spare8,
  /* 9  */  k_manualpositioning_spare9,
  /* 10 */  k_manualpositioning_spare10,
  /* 11 */  k_manualpositioning_spare11,
  /* 12 */  k_manualpositioning_spare12,
  /* 13 */  k_manualpositioning_spare13,
  /* 14 */  k_manualpositioning_spare14,

  //Timelapse settings
  /* 15 */  k_timelapse_mode,
  /* 16 */  k_timelapse_delay,
  /* 17 */  k_timelapse_interval,
  /* 18 */  k_timelapse_bulb,
  /* 19 */  k_timelapse_aftershot,
  /* 20 */  k_timelapse_shots,
  /* 21 */  k_timelapse_distance,
  /* 22 */  k_timelapse_ramping,
  /* 23 */  k_timelapse_repeat,
  /* 24 */  k_timelapse_reverse,
  /* 25 */  k_timelapse_spare10,
  /* 26 */  k_timelapse_spare11,
  /* 27 */  k_timelapse_spare12,
  /* 28 */  k_timelapse_spare13,
  /* 29 */  k_timelapse_spare14,

  //Sync settings
  /* 30 */  k_synch_mode,
  /* 31 */  k_synch_type,
  /* 32 */  k_synch_bulb,
  /* 33 */  k_synch_shots,
  /* 34 */  k_synch_distance,
  /* 35 */  k_synch_spare5,
  /* 36 */  k_synch_spare6,
  /* 37 */  k_synch_spare7,
  /* 38 */  k_synch_spare8,
  /* 39 */  k_synch_spare9,
  /* 40 */  k_synch_spare10,
  /* 41 */  k_synch_spare11,
  /* 42 */  k_synch_spare12,
  /* 43 */  k_synch_spare13,
  /* 44 */  k_synch_spare14,

  //Preferences settings
  /* 45 */  k_preferences_mode,
  /* 46 */  k_preferences_firmware,
  /* 47 */  k_preferences_units,
  /* 48 */  k_preferences_system_type,
  /* 49 */  k_preferences_steps_cal,
  /* 50 */  k_preferences_speed_cal,
  /* 51 */  k_preferences_framerate,
  /* 52 */  k_preferences_spare7,
  /* 53 */  k_preferences_spare8,
  /* 54 */  k_preferences_spare9,
  /* 55 */  k_preferences_spare10,
  /* 56 */  k_preferences_spare11,
  /* 57 */  k_preferences_spare12,
  /* 58 */  k_preferences_spare13,
  /* 59 */  k_preferences_spare14,

  /* 60 */  k_about_mode,
  /* 61 */  k_about_firmware,
  /* 62 */  k_about_releasedate,
  /* 63 */  k_about_developer1,
  /* 64 */  k_about_spare4,
  /* 65 */  k_about_spare5,
  /* 66 */  k_about_spare6,
  /* 67 */  k_about_spare7,
  /* 68 */  k_about_spare8,
  /* 69 */  k_about_spare9,
  /* 70 */  k_about_spare10,
  /* 71 */  k_about_spare11,
  /* 72 */  k_about_spare12,
  /* 73 */  k_about_spare13,
  /* 74 */  k_about_spare14,

};

enum Modes {
  k_mode_manualpositioning,
  k_mode_timelapse,
  k_mode_sync,
  k_mode_preferences,
  k_mode_about
};


// mode
const int size_of_settings = 15;
const int size_of_modes = 5;

char* mode_names[] = {"Manual/Cont", "Timelapse", "Synch Go", "Preferences", "About", "Run_status"};
char* settings[][size_of_settings] = {
  /* Manual positioning mode */ {"Mode", "Max speed", "Ramp", "" , "" , "", "" , "" , "", "", "" },
  /* Timelapse mode */          {"Mode", "Delay", "Interval", "Bulb", "Aftershot", "Shots", "Distance", "Ramping", "Rpt mode", "Rpt times", "Reverse"},
  /* Synch mode */              {"Mode", "Type"},
  /* Preferences mode */        {"Mode", "Units", "Rail type", "Steps cal", "Speed cal", },
  /* About */                   {"Mode", "Firmware", "Rel. date", "Developers"}
};


// Screen variables
boolean refresh_screen = true;
boolean redraw_screen = true;
boolean refresh_battery_status = true;
boolean redraw_setting_names_once = true;
boolean program_start = true;
int redraw_command = k_show_settings;

boolean temp_print_xy = false;


//State machine variables
int current_mode = 0;
int previous_setting = -1;
int current_setting = 0;
int current_settings_bottom_limit = 0;
int current_settings_upper_limit = 15; ////////////////////////////////////////////////////////////////////////////////// modificar para que sea referenciado a otra variable/inicializar correctamente

//Main variables
boolean carriage_is_moving = false;
unsigned long interval = 50;
unsigned long interval_index = 2;
unsigned long bulb_time = 0;
unsigned long bulb_index = 0;
unsigned long aftershot_time = 0;
unsigned long aftershot_index = 4;
unsigned long distance = 0;
unsigned long distance_index = 0;
unsigned long shots = 10;
unsigned long shots_index;
unsigned long idle_time = 40;
unsigned long delay_shots = 0;
unsigned long count = 1;
unsigned int repeat = 1;
unsigned int repeat_mode = 0;
unsigned int repeat_counter = 1;
unsigned int current_battery_adc_value = 0;
int ramp_porcentage = 0;
int ramp_index = -1;
int ramp_inflection = 0;
int mru = 0;
int mrua = 0;

// Other variables
boolean enable_joystick_event = true;

int x_position;
int y_position;


void setup() {
  // Pin definitions
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(SLEEP, OUTPUT);
  pinMode(FOCUS, OUTPUT);
  pinMode(SHUTTER, OUTPUT);
  pinMode(WHISKER1, INPUT_PULLUP);
  pinMode(WHISKER2, INPUT_PULLUP);
  pinMode(VOLTAGE, INPUT);
  pinMode(X_VALUE, INPUT);
  pinMode(Y_VALUE, INPUT);

  //Sent motor driver to sleep as it is not needed yet
  stepper_sleep();

  // STARTUP delay
  delay(250);

  // Screen setup
  ucg.begin(UCG_FONT_MODE_TRANSPARENT );
  ucg.clearScreen();
  //ucg.setRotate180();

  ucg.setColor(230, 115, 134);
  ucg.drawBox(0, 30, 128, k_font_height + 8);

  ucg.setColor(0, 0, 0);
  //ucg.setPrintPos(k_title_x, k_title_y);
  ucg.setPrintPos(k_title_x, 42);
  ucg.setFont(ucg_font_helvR08_hf);
  ucg.print("Lapse Controller");

  //Serial communication setup
  Serial.begin(115200);
  Serial.println("Chronos+");

  //Timer interrupt
  MsTimer2::set(10, heartbeat); // 10ms period
  MsTimer2::start();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  int event_id = 0;

  // Check for buttons & analog joystick status
  getJoystickPosition(&x_position, &y_position);
  getEvent(&event_id);
  if (event_id != 0) {
    update_state_machine(event_id);
    refresh_screen = true;
  }

  // Refresh screen only when neccesary
  if (refresh_screen) {
    refresh_screen = false;
    draw(redraw_command);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void update_state_machine (int event) {
  unsigned long test;
  static int nest;

  // Event while in Timelapse mode ------------------------------------------------------------------------
  if (event == k_analogbutton1_pressed && !carriage_is_moving && current_mode == k_mode_timelapse) {
    stepper_setdirection(k_away_from_motor);
    repeat_counter = repeat;
    carriage_is_moving = true;
  }

  if (event == k_analogbutton2_pressed && !carriage_is_moving && current_mode == k_mode_timelapse) {
    stepper_setdirection(k_toward_motor);
    repeat_counter = repeat;
    carriage_is_moving = true;
  }

  if (event == k_analogbutton3_pressed && current_mode == k_mode_timelapse) {
    count = 1;
    carriage_is_moving = false;
  }


  // Event while in Manual positioning mode ------------------------------------------------------------------------
  if (event == k_analogbutton1_pressed && !carriage_is_moving && current_mode == k_mode_manualpositioning) {
    carriage_is_moving = true;
  }

  if (event == k_analogbutton3_pressed && current_mode == k_mode_manualpositioning) {
    carriage_is_moving = false;
  }


  // Event while carriage is moving ------- ------------------------------------------------------------------------
  if (!carriage_is_moving) {
    if (event == k_joystick_up && current_setting >= current_settings_bottom_limit) {
      previous_setting = current_setting;
      if (current_setting == current_settings_bottom_limit) {
        current_setting = current_settings_upper_limit;
      }
      else {
        current_setting -= 1;
      }
    }

    if ((event == k_joystick_down) && (current_setting <= current_settings_upper_limit) ) {
      previous_setting = current_setting;
      if (current_setting == current_settings_upper_limit) {
        current_setting = current_settings_bottom_limit;
      }
      else {
        current_setting += 1;
      }
    }

    if (event == k_joystick_right ) {
      switch (current_setting) {
        case k_manualpositioning_mode:
        case k_timelapse_mode:
        case k_synch_mode:
        case k_preferences_mode:
        case k_about_mode:
          if (current_mode < size_of_modes - 1)  {
            current_mode++;
            current_settings_bottom_limit = current_mode * size_of_settings;
            current_settings_upper_limit = (current_mode + 1) * size_of_settings - 1;
            current_setting = current_settings_bottom_limit;
            //previous_setting = current_settings_bottom_limit;
            redraw_setting_names_once = true;
          }
          break;

        case k_timelapse_delay:
          if (delay_shots < 20)  delay_shots++;
          break;

        case k_timelapse_interval:
          if (interval_index < 134)      interval_index += 1;
          break;
        case k_timelapse_bulb:
          if (bulb_index < 50) {
            test = calc_bulb_value(bulb_index + 1);
            if (test < interval - aftershot_time - k_min_idle_time)    bulb_index += 1;
          }
          break;
        case k_timelapse_aftershot:
          if (aftershot_index < 19) {
            test = calc_aftershot_value(aftershot_index + 1);
            if (test < interval - bulb_time - k_min_idle_time)    aftershot_index += 1;
          }
          break;
        case k_timelapse_shots:
          if (shots_index < 66)     shots_index += 1;
          break;
        case k_timelapse_distance:
          if (distance_index < 39)  distance_index += 1;
          break;
        case k_timelapse_ramping:
          if (ramp_index < 10)       ramp_index++;
          break;
        case k_timelapse_repeat:
          if (repeat_mode < 1) repeat_mode++;
          break;

      }
    }

    if (event == k_joystick_left ) {
      switch (current_setting) {
        case k_manualpositioning_mode:
        case k_timelapse_mode:
        case k_synch_mode:
        case k_preferences_mode:
        case k_about_mode:
          if (current_mode > 0) {
            current_mode--;
            current_settings_bottom_limit = current_mode * size_of_settings;
            current_settings_upper_limit = (current_mode + 1) * size_of_settings - 1;
            current_setting = current_settings_bottom_limit;
            //previous_setting = current_settings_bottom_limit;
            redraw_setting_names_once = true;
          }
          break;

        case k_timelapse_delay:
          if (delay_shots > 0)  delay_shots--;
          break;

        case k_timelapse_interval:
          if (interval_index > 0) {
            test = calc_interval_value(interval_index - 1);
            if (test > bulb_time + aftershot_time + k_min_idle_time)      interval_index -= 1;
          }
          break;
        case k_timelapse_bulb:
          if (bulb_index > 0)        bulb_index -= 1;
          break;
        case k_timelapse_aftershot:
          if (aftershot_index > 0)   aftershot_index += -1;
          break;
        case k_timelapse_shots:
          if (shots_index > 0)       shots_index += -1;
          break;
        case k_timelapse_distance:
          if (distance_index > 0)    distance_index += -1;
          break;
        case k_timelapse_ramping:
          if (ramp_index >= 0)        ramp_index--;
          break;
        case k_timelapse_repeat:
          if (repeat_mode > 0) repeat_mode--;
          break;
      }
    }
  }
  // Update main variables
  interval = calc_interval_value(interval_index);
  bulb_time = calc_bulb_value(bulb_index);
  aftershot_time = calc_aftershot_value(aftershot_index);
  idle_time = interval - bulb_time - aftershot_time;

  mru = k_steps_per_cm * distance / shots ;
  mrua = mru / shots;

  ramp_porcentage = 10 * (ramp_index);
  ramp_inflection = ramp_porcentage * shots / 100;
  distance = 25 * (distance_index + 1);
  if (shots_index > 19)  shots = 200 * shots_index - 3200;
  else shots = 30 * (1 + shots_index);


  /*Serial.println("**************");
    Serial.print("Current mode=");
    Serial.println(current_mode);

    Serial.print("Current setting=");
    Serial.println(current_setting);

    Serial.print("Previous setting=");
    Serial.println(previous_setting);

    Serial.print("Current setting bottom limit=");
    Serial.println(current_settings_bottom_limit);

    Serial.print("Current setting upper limit=");
    Serial.println(current_settings_upper_limit);

    Serial.print("Interval=");
    Serial.println(interval);

    Serial.print("Idle time=");
    Serial.println(idle_time);

    Serial.print("Bulb time=");
    Serial.println(bulb_time);

    Serial.print("aftershot time");
    Serial.println(aftershot_time);

    Serial.print("distance=");
    Serial.println(distance);

    Serial.print("Shots=");
    Serial.println(shots);

    Serial.print("Ramp inflection=");
    Serial.println(ramp_inflection);

    Serial.print("mrua=");
    Serial.println(mrua);

    Serial.print("mru=");
    Serial.println(mru);*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void heartbeat() {
  static long beat = 0;
  static unsigned long battery_status_refresh_counter = 0;
  static unsigned int count_for_joystick_event = 0;
  static unsigned int current_timelapse_state = 0;
  static unsigned long temp_idle_time = 0;
  static unsigned long temp_bulb_time = 0;
  static unsigned long temp_aftershot_time = 0;
  static unsigned long temp_motor_time = 0;
  static int prev_reference = 0;
  static int reference = 0;
  static  int temp_speed = 0;
  unsigned long a;
  unsigned long b;
  unsigned long c;

  unsigned int delta_steps = 0;

  static int temp_counter = 0;

  interrupts();

  // Flag to periodically update battery status ---------------------------------------------------------------------------
  if (battery_status_refresh_counter < 1000) {
    battery_status_refresh_counter++;
  }
  else {
    battery_status_refresh_counter = 0;
    refresh_battery_status = true;
  }

  // Flag to trigger additional joystick events while keeping it in one position (left, right, up, down) ------------------
  if ((enable_joystick_event == false) && (count_for_joystick_event < 35)) {
    count_for_joystick_event = count_for_joystick_event + 1;
  }
  else {
    count_for_joystick_event = 0;
    enable_joystick_event = true;
  }


  // Reset variables when carriage is not moving --------------------------------------------------------------------------
  if (carriage_is_moving == false) {
    beat = 0;
    reference = 1;
    return;
  }

  // Sequence for Manual positioning Mode ---------------------------------------------------------------------------------
  if ((carriage_is_moving == true ) && (current_mode == k_mode_manualpositioning))  {
    prev_reference = reference;

    if (temp_speed == 0)  reference = 1;
    else                  reference = (beat) / (temp_speed);

    //Serial.print(reference);
    //Serial.print(" ");
    if (reference == 0) {
      stepper_move(1 , k_high_speed);
    }

    if (beat > 8) {
      beat = 0;
      //Serial.print(" - ");
      //Serial.println(temp_speed);
      if (x_position > 511)   {
        temp_speed = (x_position - 512) / 50;
        stepper_setdirection(k_toward_motor);
      }
      else        {
        temp_speed = (511 - x_position) / 50;
        stepper_setdirection(k_away_from_motor);
      }

    }
    else {
      beat++;
    }

    if (temp_counter > 50) {
      temp_print_xy = true;
      temp_counter = 0;
      //Serial.print(temp_speed);

    }
    else {
      temp_counter++;

    }
  }

  // Sequence for Timelapse Mode (3 states) -------------------------------------------------------------------------------
  if ((carriage_is_moving == true ) && (current_mode == k_mode_timelapse))  {
    if ((repeat_counter > 0) && (count <= shots)) {
      beat = beat + 10;

      if (beat < idle_time) {
        if (current_timelapse_state == -1) {
          a = temp_idle_time;
          temp_idle_time = millis();
          Serial.print("Cycle:");
          Serial.println(temp_idle_time - a);

          current_timelapse_state = 0;
        }
        //delay(k_focus_delay);
        return;
      }

      if (beat < idle_time + bulb_time) {
        if (current_timelapse_state == 0) {
          current_timelapse_state = 1;
          temp_bulb_time = millis();
          Serial.print("Idle time:");
          Serial.println(temp_bulb_time - temp_idle_time);

          digitalWrite(FOCUS, HIGH);
          delay(k_focus_delay);
          digitalWrite(SHUTTER, HIGH);
        }
        return;
      }

      if (beat < idle_time + bulb_time + aftershot_time) {
        if (current_timelapse_state == 1) {
          current_timelapse_state = 2;
          temp_aftershot_time = millis();
          Serial.print("Bulb time:");
          Serial.println(temp_aftershot_time - temp_bulb_time);
          digitalWrite(FOCUS, LOW);
          digitalWrite(SHUTTER, LOW);
        }

        return;
      }
      else {
        if (ramp_porcentage >= 0) {
          if (count <= ramp_inflection) {
            delta_steps = mrua * (count * count - (count - 1) * (count - 1)) / 10;
          }
          else {
            delta_steps = mrua * ((shots - (count - 1)) * (shots - (count - 1)) - (shots - count) * (shots - count)) / 10;
          }
        }
        else {
          delta_steps = mru / 10;
        }

        if (current_timelapse_state == 2) {
          current_timelapse_state = -1;
          temp_motor_time = millis();
          Serial.print("Aftershot:");
          Serial.println(temp_motor_time - temp_aftershot_time);
        }

        Serial.print ("Count=");
        Serial.print (count);
        Serial.print (", steps=");
        Serial.println (delta_steps);

        stepper_move(delta_steps, k_normal_speed);
        beat = 0;
        count++;
      }
    }
    else {
      count = 1;
      if (repeat_counter < 1) {
        carriage_is_moving = false;
      }
      else {
        carriage_is_moving = true;
        repeat_counter--;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void draw(int mode) {
  float current_battery_voltage = 0;
  /*
    //Serial.println(redraw);
    if (redraw == k_hide_settings) {
      redraw_setting_names_once = true;
      ucg.setColor(0, 0, 0);
      ucg.drawBox(0, k_settings_titles_y - k_font_height, 128, 120);
      return;
    }*/

  ucg.setFont(ucg_font_lucasfont_alternate_hf);
  if (redraw_setting_names_once) {
    redraw_setting_names_once = false;
    ucg.setColor(0, 0, 0);
    ucg.drawBox(0, k_settings_titles_y - k_font_height, 128, 120);
    ucg.setColor(255, 255, 255);
    for (int i = 0; i < size_of_settings; i++) {
      ucg.setPrintPos(k_settings_titles_x, k_settings_titles_y + k_settings_line_spacing * i);
      ucg.print(settings[current_mode][i]);
    }

    for (int i = 0; i < size_of_settings; i++) {
      ucg.setColor(0, 0, 0);
      ucg.drawBox(k_settings_values_x, k_settings_values_y + k_settings_line_spacing * i - k_font_height + 1, k_font_width * 10, k_font_height);
      ucg.setPrintPos(k_settings_values_x, k_settings_values_y + k_settings_line_spacing * i);
      ucg.setColor(255, 255, 255);
      draw_settings(size_of_settings * current_mode + i);
      redraw_screen = false;
    }
  }

  ucg.setColor(0, 0, 0);
  ucg.drawBox(k_settings_values_x, k_settings_values_y + k_settings_line_spacing * (current_setting % size_of_settings) - k_font_height + 1, k_font_width * 10, k_font_height);
  ucg.setPrintPos(k_settings_values_x, k_settings_values_y + k_settings_line_spacing * (current_setting % size_of_settings));
  ucg.setColor(0, 255, 255);
  draw_settings(current_setting);

  ucg.setColor(0, 0, 0);
  if (previous_setting >= current_settings_bottom_limit) {
    ucg.drawBox(k_settings_values_x, k_settings_values_y + k_settings_line_spacing * (previous_setting % size_of_settings) - k_font_height + 1, k_font_width * 10, k_font_height);
    ucg.setPrintPos(k_settings_values_x, k_settings_values_y + k_settings_line_spacing * (previous_setting % size_of_settings));
    ucg.setColor(255, 255, 255);
    draw_settings(previous_setting);
  }


  if (refresh_battery_status == true) {
    ucg.setColor(230, 115, 134);
    ucg.setFont(ucg_font_helvR08_hf);
    ucg.drawBox(k_battery_x - 1, k_battery_y - k_font_height + 1, 5 * k_font_width, k_font_height);
    ucg.setPrintPos(k_battery_x, k_battery_y);

    current_battery_adc_value = analogRead(VOLTAGE);
    current_battery_voltage = (float)current_battery_adc_value / 63;

    if (current_battery_adc_value > 900) {
      ucg.setColor(0, 0, 0);
    }
    else if (current_battery_adc_value > 300) {
      ucg.setColor(0, 0, 0);
    }
    else {
      ucg.setColor(0, 0, 255);
    }

    ucg.print(current_battery_voltage, 1);
    ucg.print("v");
    refresh_battery_status = false;
    ucg.setColor(255, 255, 255);
  }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void draw_refreshbattery() {

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void draw_settings(int setting) {
  switch (setting) {
    ///* 1  */  k_manualpositioning_maxspeed,
    ///* 2  */  k_manualpositioning_ramp,

    case k_manualpositioning_mode:
    case k_timelapse_mode: //Position
    case k_synch_mode:
    case k_preferences_mode:
    case k_about_mode:
      ucg.print(mode_names[current_mode]);
      break;

    case k_timelapse_delay: //Delay
      ucg.print(delay_shots);
      ucg.print(" shots");
      break;

    case k_timelapse_interval: //Interval
      if (interval < 60000) {
        ucg.print(interval / 1000);
        ucg.print(" seg     ");
      }
      else if (interval < 3600000) {
        ucg.print(interval / 60000);
        ucg.print(" min    ");
      }
      else {
        ucg.print(interval / 3600000);
        ucg.print(" hr:     ");
      }
      break;

    case k_timelapse_bulb: //Bulb
      ucg.print(bulb_time);
      ucg.print(" ms       ");
      break;

    case k_timelapse_aftershot: //AfterShot
      ucg.print(aftershot_time);
      ucg.print(" ms       ");
      break;

    case k_timelapse_shots: //Shots
      ucg.print(shots);
      ucg.print(" shots     ");
      break;

    case k_timelapse_distance: //Distance
      ucg.print(distance / 10);
      ucg.print(" cm");
      break;

    case k_timelapse_ramping: //Ramping
      if (ramp_porcentage < 0) {
        ucg.print("Off");
      }
      else {
        ucg.print(ramp_porcentage);
        ucg.print(" %");
      }
      break;

    case k_timelapse_repeat: //Repeat
      if (repeat_mode == 0) {
        ucg.print("Fwd");
      }
      else if (repeat_mode == 1) {
        ucg.print("Start");
      }
      break;

    case k_timelapse_reverse: //Reverse
      break;

    case k_about_firmware: //Firmware version
      ucg.print(FIRMWARE_VERSION);
      break;

    case k_about_releasedate: //Firmware version
      ucg.print(FIRMWARE_RELEASE_DATE);
      break;

    case k_about_developer1: //Firmware version
      ucg.print(FIRMWARE_DEVELOPER1);
      break;

  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void stepper_move(int number_of_steps, int speed_option) {
  unsigned int a_delay, b_delay;
  boolean temp_whisker1;
  boolean temp_whisker2;

  stepper_wakeup();
  if (speed_option == k_normal_speed) {
    a_delay = 3;/*5*/
    b_delay = 2;/*2*/
  }
  else {
    a_delay = 4; /*1*/
    b_delay = 1;/*1*/
  }

  // Avoid crushes when motor gets to the end
  //stepper_setdirection(k_away_from_motor);

  for (int n = 0; n < number_of_steps; n++) {
    temp_whisker1 = digitalRead(WHISKER1);
    temp_whisker2 = digitalRead(WHISKER2);

    if (!((temp_whisker1 == HIGH && digitalRead(DIR) == k_away_from_motor ) || (temp_whisker2 == HIGH && digitalRead(DIR) == k_toward_motor))) {
      digitalWrite(STEP, HIGH);
      delay(a_delay);
      digitalWrite(STEP, LOW);
      delay(b_delay);
    }
    else
    {
      update_state_machine(k_analogbutton3_pressed);
      Serial.println("switch de emergencia");
      return;
    }
  }
  stepper_sleep();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void stepper_setdirection(int d) {
  digitalWrite(DIR, d);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void stepper_sleep(void) {
  digitalWrite(SLEEP, HIGH);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void stepper_wakeup(void) {
  digitalWrite(SLEEP, LOW);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getJoystickPosition(int *x, int *y) {
  const int size_of_array = 20;
  static int x_array[size_of_array] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  static int y_array[size_of_array] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  static unsigned int index = 0;
  int x_temp = 0;
  int y_temp = 0;


  // Update analog joystick value ---------------------------
  x_array[index] = analogRead(X_VALUE);
  y_array[index] = analogRead(Y_VALUE);

  if (index < size_of_array - 1) index++;
  else            index = 0;

  for (int i = 0; i < size_of_array; i++) {
    x_temp +=   x_array[i];
    y_temp +=   y_array[i];
  }
  x_temp = x_temp / size_of_array;
  y_temp = y_temp / size_of_array;

  *x = x_temp;
  *y = y_temp;
}

unsigned int getEvent(int *event_identifier) {
  static boolean wait_for_joystick_release = false;
  static boolean wait_for_analogbutton_release = false;
  unsigned int analogbutton_value = 0;
  unsigned int x_temp = 0;
  unsigned int y_temp = 0;
  //int identifier = 0;

  // Dummy call event at program start
  if (program_start == true) {
    *event_identifier = k_dummy_event_call;
    program_start = false;
  }

  // Update analog joystick value ---------------------------
  x_temp = analogRead(X_VALUE);
  y_temp = analogRead(Y_VALUE);

  // Check for analog joystick event ------------------------
  if (enable_joystick_event == true) {
    if (y_temp < 300) {
      //Serial.println ("Up button pressed: ");
      enable_joystick_event = false;
      *event_identifier = k_joystick_up;
    }
    else if (y_temp > 800) {
      //Serial.println ("Down button pressed: ");
      enable_joystick_event = false;
      *event_identifier = k_joystick_down;
    }
    else if (x_temp < 300) {
      //Serial.println ("Right button pressed: ");
      enable_joystick_event = false;
      *event_identifier = k_joystick_right;
    }
    else if (x_temp > 800) {
      //Serial.println ("Left button pressed: ");
      enable_joystick_event = false;
      *event_identifier = k_joystick_left;
    }
  }

  // Check for analog button event ------------------------
  analogbutton_value = analogRead(BUTTON);
  if ((analogbutton_value > 50) && (wait_for_analogbutton_release == false)) {
    wait_for_analogbutton_release = true;

    if (analogbutton_value > 750 && analogbutton_value < 950 ) {
      Serial.print ("Button #3 pressed: ");
      *event_identifier = 5;
    }
    else if (analogbutton_value > 400 && analogbutton_value < 600 ) {
      Serial.print ("Button #2 pressed: ");
      *event_identifier = 4;
    }
    else if (analogbutton_value > 200 && analogbutton_value < 400 ) {
      Serial.print ("Button #1 pressed: ");
      *event_identifier = 3;
    }
    Serial.println(analogbutton_value);
  }
  else if ((analogbutton_value < 10) && (wait_for_analogbutton_release == true)) {
    wait_for_analogbutton_release = false;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long calc_interval_value (unsigned long index) {
  unsigned long value;
  if (index < 50)                        value = 1000 * (index + 1);
  else if (index >= 50 && index < 110)   value = 60000 * (index - 50);
  else if (index >= 110)                 value = 3600000 * (index - 110);

  return value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long calc_bulb_value (unsigned long index) {
  unsigned long value;
  if (index < 9)   value = 100 * (index + 1);
  else value = 1000 * (index - 8);

  return value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long calc_aftershot_value (unsigned long index) {
  unsigned long value;
  value = 50 * (index + 1);

  return value;
}



