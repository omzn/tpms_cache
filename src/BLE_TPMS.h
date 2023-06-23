#ifndef BLE_TPMS_H
#define BLE_TPMS_H

#include <Arduino.h>

#define PRESSURE_CALIB (0)

#define BLETPMS_ManufacturerId (0x0001) 

#define BLETPMS_VenderId_FL (0x80eaca)
#define BLETPMS_VenderId_FR (0x81eaca)
#define BLETPMS_VenderId_RL (0x82eaca)
#define BLETPMS_VenderId_RR (0x83eaca)

#define BLETPMS_Tire_FL (0x11acb7) 
#define BLETPMS_Tire_FR (0x21aa09) 
#define BLETPMS_Tire_RL (0x31a6c5) 
#define BLETPMS_Tire_RR (0x41a834) 

#define TIRE_FL (0)
#define TIRE_FR (1)
#define TIRE_RL (2)
#define TIRE_RR (3)

class BLEtpms {
private:
  int _tire_id;
  uint32_t _dev_id;
  float _temp_c;
  uint32_t _temp_raw;
  float _pressure_kpa;
  uint32_t _pressure_raw;
  float _prev_pressure_kpa = 0;
  float _battery_percent;
  uint32_t _battery_raw;
  bool _updated = false;
  uint32_t _last_updated = 0;
public:
  static bool isManufacturerId(std::string d);
  static int tire_id(std::string d);
  BLEtpms();
  void scan(std::string d);
  void tire_id(int id,uint32_t d);
  int tire_id();
  uint32_t dev_id();
  uint32_t temp_raw();
  void temp_raw(int t);
  float temp();
  uint32_t pressure_raw();
  void pressure_raw(int p);
  float pressure();
  uint32_t battery_raw();
  void battery_raw(int b);
  float battery();
  float batteryV();
  void updated(bool);
  bool updated();
  String macaddress();
  uint32_t last_updated();
  void last_updated(uint32_t t);
};


#endif