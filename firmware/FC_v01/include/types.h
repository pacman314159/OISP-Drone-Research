#pragma once

// Standardized Physical Sensor Data Structs (Explicit units per [Note 0000])
struct IMUSample {
  float a[3]; // Accelerometer: m/s^2
  float g[3]; // Gyroscope: rad/s
  float t;    // Temperature: degree Celsius
};
