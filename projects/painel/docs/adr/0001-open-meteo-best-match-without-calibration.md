# Open-Meteo `best_match` without calibration

Painel uses the Open-Meteo `best_match` temperature with no offset, even though it differs by 2 to 3 °C from Apple's weather app. The models available for the configured city disagree with each other by about 3 °C, there is no evidence of an error that always leans the same way to justify an offset, and WeatherKit would require a paid Apple Developer account and a JWT token on the ESP32. The Yesterday comparison uses the same source for both days, so the absolute error matters less there.
