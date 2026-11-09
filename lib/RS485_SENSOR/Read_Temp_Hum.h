#ifndef __READ_TEMPTURE_HUM_SENSOR
#define __READ_TEMPTURE_HUM_SENSOR

#include <Arduino.h>
#include <ModbusMaster.h>
class DataFormSensor{
    public:
    float Data;
    uint8_t flag;
};
class RS485_SENSOR_TO_UART : public Printable{
private:
    ModbusMaster node;
    HardwareSerial* serialPort;
    uint8_t slaveId;
    uint32_t BAUDRATE;
    int8_t TX;
    int8_t RX;
    uint8_t NUM_REG;
    DataFormSensor *DataSensor; 

public:
    RS485_SENSOR_TO_UART(uint8_t SlaveID, uint32_t BAUDRATE, uint8_t TX_PIN, uint8_t RX_PIN, uint8_t NUM_REG , HardwareSerial* serial );
    void ClearBuffer();
    ~RS485_SENSOR_TO_UART();
    void ReadSensor();
    DataFormSensor GetData(uint8_t indexReg);
    void begin();
    size_t printTo(Print& p) const override;
};

#endif