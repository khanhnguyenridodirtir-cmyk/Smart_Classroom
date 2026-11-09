#include "Read_Temp_Hum.h"

RS485_SENSOR_TO_UART::RS485_SENSOR_TO_UART
    (uint8_t SlaveID, uint32_t BAUDRATE, uint8_t TX_PIN, uint8_t RX_PIN, uint8_t NumReg, HardwareSerial * serial ){
    this->serialPort = serial; 
    this->TX = TX_PIN ;
    this->RX = RX_PIN;
    this->BAUDRATE=BAUDRATE;
    this->NUM_REG=NumReg;
    this->slaveId = SlaveID;
    this->DataSensor = new DataFormSensor[this->NUM_REG];
    node.begin(SlaveID, *serialPort);
}

void RS485_SENSOR_TO_UART:: ClearBuffer(){
    while ( this->serialPort->available() > 0) {
      this->serialPort->read();
    }
}
RS485_SENSOR_TO_UART::~RS485_SENSOR_TO_UART(){
    delete []DataSensor;
}

DataFormSensor RS485_SENSOR_TO_UART::GetData(uint8_t indexReg){
    return DataSensor[indexReg];
}
void RS485_SENSOR_TO_UART::ReadSensor(){
    
    for(int i =0; i<this->NUM_REG;i++){
        this->ClearBuffer();
         uint8_t infor = node.readHoldingRegisters(i, 1);
          float dataFormSensor =node.getResponseBuffer(0) / 10.0f;
          DataSensor[i]={dataFormSensor, (infor == node.ku8MBSuccess)};
          
        vTaskDelay(pdMS_TO_TICKS(100));
    }
 
} 
void RS485_SENSOR_TO_UART::begin() {
    // Tự động khởi tạo Serial với các thông số đã lưu trong Constructor
    this->serialPort->begin(this->BAUDRATE, SERIAL_8N1, this->RX, this->TX);
}
  
size_t RS485_SENSOR_TO_UART::printTo(Print& p) const {
    size_t n = 0;
    n += p.print("Slave ID: ");
    n += p.println(slaveId);
    
    // In ra toàn bộ thanh ghi đã đọc được
    for(uint8_t i = 0; i < NUM_REG; i++) {
        if(DataSensor[i].flag==1){
            n += p.print("Reg[");
            n += p.print(i);
            n += p.print("] = ");
            n += p.println(DataSensor[i].Data);
        }
        
    }
    return n; // Trả về số byte đã in (quy định bắt buộc của Arduino)
}