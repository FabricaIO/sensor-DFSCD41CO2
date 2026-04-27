#include "DFSCD41CO2.h"

/// @brief Creates a new SCD41 sensor object
/// @param Name The device name
/// @param I2C_bus The I2C bus attached to the sensor
/// @param Address Address of the SCD41 sensor
/// @param ConfigFile The file name to store settings in
DFSCD41CO2::DFSCD41CO2(String Name, TwoWire* I2C_bus, uint8_t Address, String ConfigFile) : scd41_sensor(I2C_bus, Address), Sensor(Name) {
	config_path = "/settings/sen/" + ConfigFile;
	i2c_bus = I2C_bus;
}

/// @brief Creates a new SCD41 sensor object
/// @param Name The device name
/// @param sda SDA pin to use for I2C bus
/// @param scl SCL pin to use for I2C bus
/// @param I2C_bus The I2C bus attached to the sensor
/// @param Address Address of the SCD41 sensor
/// @param ConfigFile The file name to store settings in
DFSCD41CO2::DFSCD41CO2(String Name, int sda, int scl, TwoWire* I2C_bus, uint8_t Address, String ConfigFile) : scd41_sensor(I2C_bus, Address), Sensor(Name) {
	config_path = "/settings/sen/" + ConfigFile;
	i2c_bus = I2C_bus;
	scl_pin = scl;
	sda_pin = sda;
}

/// @brief Starts the SCD41 sensor
/// @return True on success
bool DFSCD41CO2::begin() {
	Description.parameterQuantity = 3;
	Description.type = "CO2 Environmental Sensor";
	Description.parameters = {"CO2", "Temperature", "Humidity"};
	Description.units = {"ppm", "C", "%RH"};
	values.resize(Description.parameterQuantity);

	bool result = false;
	// Start I2C bus if not started
	if (scl_pin > -1 && sda_pin > -1) {
		if (!i2c_bus->begin(sda_pin, scl_pin)) {
			return false;
		}
	} else {
		if (!i2c_bus->begin()) {
			return false;
		}
	}

	// Initialize SCD41
	if (!scd41_sensor.begin()) {
		return false;
	}
	// Disable measurement mode
	scd41_sensor.enablePeriodMeasure(SCD4X_STOP_PERIODIC_MEASURE);
	delay(550);
	Logger.println("Performing self test on DSC41...");
	if(scd41_sensor.performSelfTest() != 0) {
		Logger.println("Self test failed.");
		return false;
	}
	// Create settings directory if necessary
	if (!checkConfig(config_path)) {
		// Set defaults
		if (!setConfig(getConfig(), true)) {
			return false;
		}
	} else {
		// Load settings
		if (!setConfig(Storage::readFile(config_path), false)) {
			return false;
		}
	}
	return true;
}

/// @brief Takes a measurement
/// @return True on success
bool DFSCD41CO2::takeMeasurement() {
	// Check if measurement is ready
	if (!scd41_sensor.getDataReadyStatus()) {
		Logger.println("No new data available yet, min 5 seconds (30 in low power) between measurements.");
		return false;
	}

	// Read measurement from sensor
	DFRobot_SCD4X::sSensorMeasurement_t sensorData;
	scd41_sensor.readMeasurement(&sensorData);

	// Assign values
	values[0] = sensorData.CO2ppm;
	values[1] = sensorData.temp;
	values[2] = sensorData.humidity;
	return true;
}

/// @brief Gets the current config
/// @return A JSON string of the config
String DFSCD41CO2::getConfig() {
	// Allocate the JSON document
	JsonDocument doc;

	// Assign current values
	doc["Name"] = Description.name;
	doc["tempOffset"] = current_config.tempOffset;
	doc["currentAltitude"] = current_config.currentAltitude;
	doc["autoCalibration"] = current_config.autoCalibration;
	doc["lowPowerMode"] = current_config.lowPowerMode;

	// Create string to hold output
	String output;

	// Serialize to string
	serializeJson(doc, output);
	return output;
}

/// @brief Sets the configuration for this device
/// @param config A JSON string of the configuration settings
/// @param save If the configuration should be saved to a file
/// @return True on success
bool DFSCD41CO2::setConfig(String config, bool save) {
	// Allocate the JSON document
	JsonDocument doc;

	// Deserialize file contents
	DeserializationError error = deserializeJson(doc, config);

	// Test if parsing succeeds.
	if (error) {
		Logger.print(F("Deserialization failed: "));
		Logger.println(error.f_str());
		return false;
	}

	// Assign loaded values
	Description.name = doc["Name"].as<String>();
	current_config.tempOffset = doc["tempOffset"].as<float>();
	current_config.currentAltitude = doc["altitude"].as<uint16_t>();
	current_config.autoCalibration = doc["autoCalibration"].as<bool>();
	current_config.lowPowerMode = doc["lowPowerMode"].as<bool>();

	setSensorConfig();

	if (save) {
		return saveConfig(config_path, config);
	}
	return true;
}

/// @brief Updates the settings on the sensor
void DFSCD41CO2::setSensorConfig() {
	// Disable measurement mode
	scd41_sensor.enablePeriodMeasure(SCD4X_STOP_PERIODIC_MEASURE);
	delay(550);
	scd41_sensor.setTempComp(current_config.tempOffset);
	scd41_sensor.setSensorAltitude(current_config.currentAltitude);
	scd41_sensor.setAutoCalibMode(current_config.autoCalibration);
	// Apply measurement mode
	if (current_config.lowPowerMode) {
		scd41_sensor.enablePeriodMeasure(SCD4X_START_LOW_POWER_MEASURE);
	} else {
		scd41_sensor.enablePeriodMeasure(SCD4X_START_PERIODIC_MEASURE);
	}
}