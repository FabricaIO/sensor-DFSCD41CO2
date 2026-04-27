/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2026 Sam Groveman
*
* External libraries needed:
* DFRobot_SCD4X: https://github.com/DFRobot/DFRobot_SCD4X/
*
* https://www.dfrobot.com/product-2646.html
*
* Contributors: Sam Groveman
*/
#pragma once
#include <Sensor.h>
#include <Wire.h>
#include <DFRobot_SCD4X.h>

/// @brief Device for interfacing with the DF Robot SCD41 CO2 sensor
class DFSCD41CO2 : public Sensor {
	public:
		DFSCD41CO2(String Name, TwoWire* I2C_bus = &Wire, uint8_t Address = 0x62, String ConfigFile = "DFSCD41CO2.json");
		DFSCD41CO2(String Name, int sda, int scl, TwoWire* I2C_bus = &Wire, uint8_t Address = 0x62, String ConfigFile = "DFSCD41CO2.json");

		bool begin();
		bool takeMeasurement();
		String getConfig();
		bool setConfig(String config, bool save);

	protected:
		/// @brief DFSCD41CO2 sensor configuration
		struct {
			/// @brief Temperature offset in Celsius
			float tempOffset = 0.0;

			/// @brief Current sensor altitude in meters
			uint16_t currentAltitude = 0;

			/// @brief Enable automatic self-calibration
			bool autoCalibration = true;

			/// @brief Enable low power measurement mode (30s interval instead of 5s)
			bool lowPowerMode = false;
		} current_config;

		/// @brief I2C bus in use
		TwoWire* i2c_bus;

		/// @brief SCL pin in use
		int scl_pin = -1;

		/// @brief SDA pin in use
		int sda_pin = -1;

		/// @brief SCD41 sensor object
		DFRobot_SCD4X scd41_sensor;

		/// @brief Full path to config file
		String config_path;

		void setSensorConfig();
};
