
// With some code derived from:
// https://build.particle.io/shared_apps/5a1d9e5310c40e5c02001232

// Hardware list - BOM
//
// https://store.particle.io/products/boron-lte Paticle Boron
//
// https://www.adafruit.com/product/938 Monochrome 1.3" 128x64 OLED graphic display
//		- Solder jumpers to put it in i2c mode
//		- Connect to i2c bus and DISPLAY_RESET
//
// https://www.adafruit.com/product/2652 BME280 I2C or SPI Temperature Humidity Pressure Sensor[ID:2652]
//		- Connect to SPI pins
//
// https://www.adafruit.com/product/3263 Universal Thermocouple Amplifier MAX31856
//		- Connect to i2c bus
//



#include <Adafruit_MAX31856.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DiagnosticsHelperRK.h>

#define ALARM_NO_PIN A0
#define ALARM_NC_PIN A1
#define SPI_SS_MAX31856 A2
#define SPI_SCLK A3
#define SPI_MISO A4
#define SPI_MOSI A5

#define I2C_SDA D0
#define I2C_SCL D1
#define DISPLAY_RESET D2
#define BUTTON_DISPLAY D3
#define BUTTON_FUNCTION D4
#define ONE_WIRE D5
#define LED_A D6
#define LED_B D7

// Enable system threading
SYSTEM_THREAD(ENABLED);

// public variables
const unsigned long UPDATE_PERIOD_MS = 5000;
unsigned long last_update = 0;
int led_on_state = 0;

// Define board type
// 0 Unknown
// 1 Boron
// 2 Not Boron
int board_type = 0;
char board_name[21];

double temp_tc = 0;
double temp_tc_cj = 0;
double temp_amb = 0;
double humid_amb = 0;
bool equip_alarm = FALSE;
bool equip_alarm_last = FALSE;
FuelGauge fuel;
double batt_percent = 0;

bool usb_power_last = FALSE;
bool usb_power = TRUE;

bool equip_spec = TRUE;
double alarm_temp_min;
double alarm_temp_max;
bool low_t_alarm = FALSE;
bool low_t_alarm_last = FALSE;
bool high_t_alarm = FALSE;
bool high_t_alarm_last = FALSE;

float alarm_ambient_t_max = 30.0;
float alarm_ambient_h_max = 75.0;
bool amb_t_alarm = FALSE;
bool amb_t_alarm_last = FALSE;
bool amb_h_alarm = FALSE;
bool amb_h_alarm_last = FALSE;

bool fault_bme = FALSE;
uint8_t fault_thermocouple = 0;
bool BMEsensorReady = FALSE;

// Use software SPI: CS, DI, DO, CLK
// Pin labels are CS, SDI, SDO, SCK
// formal labels: SS, MISO, MOSI, SCLK
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(
	SPI_SS_MAX31856,
	SPI_MISO,
	SPI_MOSI,
	SPI_SCLK );

Adafruit_BME280 bme; // I2C

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (1 if sharing Arduino reset pin)
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,OLED_RESET);
Adafruit_SSD1306 display(128, 64, DISPLAY_RESET);
//char buf[64];

// EEPROM addresses
int alarm_temp_min_address = 1;
int alarm_temp_max_address = 5;

void selectExternalMeshAntenna() {
	// Per https://community.particle.io/t/xenon-mesh-external-antenna-enable/51424
	#if (PLATFORM_ID == PLATFORM_ARGON)
		digitalWrite(ANTSW1, 1);
		digitalWrite(ANTSW2, 0);
	#elif (PLATFORM_ID == PLATFORM_BORON)
		digitalWrite(ANTSW1, 0);
	#else
		digitalWrite(ANTSW1, 0);
	  digitalWrite(ANTSW2, 1);
	#endif
}

void name_handler(const char *topic, const char *data) {
	strncpy(board_name, data, sizeof(board_name));
}

void setup() {

	selectExternalMeshAntenna();
	Particle.subscribe( "particle/device/name", name_handler, MY_DEVICES);
	// wait so we know our name
	waitUntil(Particle.connected);
	Particle.publish("particle/device/name", PRIVATE);

	Particle.variable( "board_name", board_name , STRING );
	Particle.variable( "temp_tc", &temp_tc, DOUBLE );
	Particle.variable( "temp_tc_cj", &temp_tc_cj, DOUBLE );
	Particle.variable( "temp_amb", &temp_amb, DOUBLE );
	Particle.variable( "humid_amb", &humid_amb, DOUBLE );
	Particle.variable( "equip_alarm", &equip_alarm, BOOLEAN );
	Particle.variable( "batt_percent", &batt_percent, DOUBLE );
	Particle.variable( "usb_power", &usb_power, BOOLEAN );
	Particle.variable( "equip_spec", &equip_spec, BOOLEAN );
	Particle.variable( "low_t_alarm", &low_t_alarm, BOOLEAN );
	Particle.variable( "amb_t_alarm", &amb_t_alarm, BOOLEAN );
	Particle.variable( "amb_h_alarm", &amb_h_alarm, BOOLEAN );
	Particle.variable( "fault_bme", &fault_bme, BOOLEAN );
	Particle.variable( "alarm_t_min", &alarm_temp_min, DOUBLE );
	Particle.variable( "alarm_t_max", &alarm_temp_max, DOUBLE );

	Particle.function("clear_eeprom", clear_eeprom);
	Particle.function("SetAlarmTMax", write_alarm_temp_max);
	Particle.function("SetAlarmTMin", write_alarm_temp_min);

	pinMode( LED_A, OUTPUT );
	pinMode( LED_B, OUTPUT );

	pinMode( ALARM_NO_PIN, INPUT_PULLUP );
	pinMode( ALARM_NC_PIN, INPUT_PULLUP );

	alarm_temp_min = read_eeprom_temp( alarm_temp_min_address, -1000 );
	alarm_temp_max = read_eeprom_temp( alarm_temp_max_address, 1000 );

	// Start sensors
	maxthermo.begin();
	maxthermo.setThermocoupleType(MAX31856_TCTYPE_K);
	BMEsensorReady = bme.begin();

	if ( ! BMEsensorReady ){
		submit_json_message("FAULT_BME", "BME sensor is not ready");
		fault_bme = TRUE;
	}

	// Display
	display.begin(SSD1306_SWITCHCAPVCC, 0x3D);

	// Publish some hardware stuff
	#if (PLATFORM_ID == PLATFORM_BORON)

	submit_json_message("Power_stat", String(DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_POWER_SOURCE)));
	board_type = 1;

	#else
	pinMode(PWR, INPUT);
	submit_json_message("Power_CHG", String(digitalRead(CHG)));
	submit_json_message("Power_PWR", String(digitalRead(PWR)));
	board_type = 2;

	#endif

	submit_json_message("BOARD", String(board_type));
}

void loop() {
	if (millis() - last_update >= UPDATE_PERIOD_MS) {
		// alternate the LED between high and low
		// to show that we're still alive
		digitalWrite(LED_A, (led_on_state) ? HIGH : LOW);
		led_on_state = !led_on_state;

		// Check equipment alarms
		if( (digitalRead(ALARM_NC_PIN) == HIGH) or (digitalRead(ALARM_NO_PIN) == LOW) )
		{
			equip_alarm = TRUE;
			digitalWrite(LED_B, HIGH);
		} else {
			equip_alarm = FALSE;
			digitalWrite(LED_B, LOW);
		}

		if( equip_alarm != equip_alarm_last ){
			equip_alarm_last = equip_alarm;
			if ( equip_alarm ){
				submit_json_message("ALARM", "equipment alarm");
			} else {
				submit_json_message("CLEAR", "equipment alarm clear");
			}
		}


		// Power state
		batt_percent = fuel.getSoC();
		usb_power = isUsbPowered();

		if (usb_power != usb_power_last) {
			if (usb_power){
				submit_json_message("CLEAR", "usb power restored");
			} else {
				submit_json_message("ALARM", "no usb power");
			}
			usb_power_last = usb_power;
		}

		// Read Ambient data
		// Reading temperature or humidity takes about 250 milliseconds

		temp_amb = bme.readTemperature(); // degrees C
		humid_amb = bme.readHumidity(); // %


		// Check if any reads failed
		fault_bme = FALSE;
		if (isnan(temp_amb)) {
				submit_json_message("WARN", "failed to read from DHT sensor temperature");
			fault_bme = TRUE;
		}

		if (isnan(humid_amb)) {
				submit_json_message("WARN", "failed to read from DHT sensor humidity");
			fault_bme = TRUE;
		}

		// Read thermocouple data
		temp_tc = maxthermo.readThermocoupleTemperature();
		temp_tc_cj = maxthermo.readCJTemperature();

		// Check and print any faults
		uint8_t current_fault = maxthermo.readFault();
		if ( current_fault ) {

			if( fault_thermocouple != current_fault ){ // Only publish the fault when it differs from previous fault state
				if (current_fault & MAX31856_FAULT_CJRANGE) {
					submit_json_message("WARN", "cold junction range fault");
				}
				if (current_fault & MAX31856_FAULT_TCRANGE) {
					submit_json_message("WARN", "thermocouple range fault");
				}
				if (current_fault & MAX31856_FAULT_CJHIGH)  {
					submit_json_message("WARN", "cold junction high fault");
				}
				if (current_fault & MAX31856_FAULT_CJLOW)   {
					submit_json_message("WARN", "cold junction low fault");
				}
				if (current_fault & MAX31856_FAULT_TCHIGH)  {
					submit_json_message("WARN", "thermocouple high fault");
				}
				if (current_fault & MAX31856_FAULT_TCLOW)   {
					submit_json_message("WARN", "thermocouple low fault");
				}
				if (current_fault & MAX31856_FAULT_OVUV)    {
					submit_json_message("WARN", "over/under voltage fault");
				}
				if (current_fault & MAX31856_FAULT_OPEN)    {
					submit_json_message("WARN", "thermocouple open fault");
				}
			}

			fault_thermocouple = current_fault;

		} else {
			if( fault_thermocouple != current_fault ){
				// Has gone from fault to no fault
				submit_json_message("WARN", "thermocouple Fault(s) cleared");
				fault_thermocouple = current_fault;
			}
		}

		// Update sensor alarms
		if ( (temp_tc < alarm_temp_min) && equip_spec && ! fault_thermocouple ){
			low_t_alarm = TRUE;
		} else {
			low_t_alarm = FALSE;
		}

		if ( low_t_alarm != low_t_alarm_last ){
			if (low_t_alarm){
				submit_json_message("INFO", "internal temperature below minimum");
			}
			else
			{
				submit_json_message("INFO", "internal temperature no longer below minimum");
			}
			low_t_alarm_last = low_t_alarm;
		}


		if ( (temp_tc > alarm_temp_max) && equip_spec && ! fault_thermocouple ){
			high_t_alarm = TRUE;
		} else {
			high_t_alarm = FALSE;
		}

		if ( high_t_alarm != high_t_alarm_last ){
			if (high_t_alarm){
				submit_json_message("INFO", "internal temperature above maximum");
			} else {
				submit_json_message("INFO", "internal temperature no longer above maximum");
			}
			high_t_alarm_last = high_t_alarm;
		}

		if ( (temp_amb > alarm_ambient_t_max) && ! fault_bme ){
			amb_t_alarm = TRUE;
		} else {
			amb_t_alarm = FALSE;
		}

		if ( amb_t_alarm != amb_t_alarm_last ){
			if (amb_t_alarm){
				submit_json_message("INFO", "ambient temperature above maximum");
			} else {
				submit_json_message("INFO", "ambient temperature no longer above maximum");
			}
			amb_t_alarm_last = amb_t_alarm;
		}


		if ( (humid_amb > alarm_ambient_h_max && ! fault_bme ) ){
			amb_h_alarm = TRUE;
		} else {
			amb_h_alarm = FALSE;
		}

		if ( amb_h_alarm != amb_h_alarm_last ){
			if (amb_t_alarm){
				submit_json_message("INFO", "ambient humidity above maximum");
			} else {
				submit_json_message("INFO", "ambient humidity no longer above maximum");
			}
			amb_h_alarm_last = amb_h_alarm;
		}

		// Create a status summary for display. Shows only one status at a time,
		// put higher priority items later in sequence so they overwrite
		// lower priority items.
		String monitor_status = "System is nominal";
		bool nominal = TRUE;

		if (fault_bme){
			monitor_status =          "WARN: Ambient sensor";
		}

		if (fault_thermocouple){
			monitor_status =          "WARN: Thermocouple";
		}

		if ( abs( temp_tc_cj - temp_amb  ) > 3 ){
			monitor_status =          "INFO: Temp mismatch";
		}

		if ( amb_h_alarm ){
			monitor_status =          "INFO: Ambient humid";
		}

		if ( amb_t_alarm ){
			monitor_status =          "INFO: Ambient temp";
		}

		if ( ! usb_power ){
			monitor_status =          "ALARM: No power";
		}

		if ( equip_alarm ){
			monitor_status =          "ALARM: Equip. alarm";
		}

		if ( low_t_alarm ){
			monitor_status =          "INFO: Low temp";
		}

		if ( high_t_alarm ){
			monitor_status =          "INFO: High temp";
		}

		if ( monitor_status != "System is nominal" ){
			nominal = FALSE;
		}

		// Update display
		display.clearDisplay();
		// Size 1 has xx characters per row
		display.setTextSize(1);
		display.setTextColor(WHITE);
		display.invertDisplay(! nominal);
		display.setCursor(0,0);
		display.println(board_name);
		display.println(monitor_status);
		display.println(String::format("Ambient  Temp: %.1f C", temp_amb));
		display.println(String::format("Ambient Humid: %.0f %%", humid_amb));
		display.println("Internal Temp:");
		display.setTextSize(3);
		display.println(String::format("%.1f C", temp_tc));

		display.display();
	last_update = millis();
	}
}


// EEPROM handlers
// These allow for non-volatile storage of board-specific settings
// EEPROM that has not been written will have a value of 0xFFFFFFFF,
// need to chehck for that and set a default value of EEPROM has not
// been initialized.

double read_eeprom_temp( int address, float default_temp ){
	float temp_read = default_temp;
	uint32_t temp_read_int;
	EEPROM.get( address, temp_read_int );
	if ( temp_read_int != 0xFFFFFFFF ){
		EEPROM.get( address, temp_read );
	}
	// Stored as a float but used as a double so that it works with Particle.variable()
	double temp_read_double = temp_read;
	return (temp_read_double);
}

int write_alarm_temp_max ( String temp_string ) {
	float temp = temp_string.toFloat();
	float address = alarm_temp_max_address;
	EEPROM.put( address, temp);

	// Check the written value and load it into global variable
	float written = read_eeprom_temp( address, -1000000 );
	if ( abs( written - temp ) < 0.1 ){
		// All is well
		alarm_temp_max = written;
		return( 0 );
	} else {
		// There was a problem
		return( 1 );
	}
}

int write_alarm_temp_min ( String temp_string ) {
	float temp = temp_string.toFloat();
	float address = alarm_temp_min_address;
	EEPROM.put( address, temp);

	// Check the written value and load it into global variable
	float written = read_eeprom_temp( address, -1000000 );
	if ( abs( written - temp ) < 0.1 ){
		// All is well
		alarm_temp_min = written;
		return( 0 );
	} else {
		// There was a problem
		return( 1 );
	}
}

int clear_eeprom( String extra ){
	EEPROM.clear();
	return(0);
}

bool submit_json_message(const char* msg_type, const char* message){
	// formatting for JSON payload, which makes these variables available from particle webhooks
	char const *json_out_fmt = "{ \"message\": \"%s\", \"board_name\": \"%s\", \"temp_tc\": %.1f, \"temp_amb\": %.1f, \"humid_amb\": %.0f,\"alarm_temp_min\": %.1f, \"alarm_temp_max\": %.1f }";
	String json_payload = String::format( json_out_fmt, message, board_name, temp_tc, temp_amb, humid_amb, alarm_temp_min, alarm_temp_max );
	return( Particle.publish(msg_type, json_payload, PRIVATE) );
}

// Xenon and Boron do power management and status checks differently
// Use conditional directives to pick thhe right approachh and compile time
// See thread at https://community.particle.io/t/battery-charging-indicator-not-working/48345/5

#if (PLATFORM_ID == PLATFORM_BORON)

bool isUsbPowered() {
	// builds on https://community.particle.io/t/boron-battery-connected/47789/9
	int powerSource = DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_POWER_SOURCE);

	if ( (powerSource == 5) || (powerSource == 0) ){
		return( FALSE );
	} else {
		return( TRUE );
	}

}

#else

bool isUsbPowered() {
	return( digitalRead(PWR) == HIGH );
}

#endif
