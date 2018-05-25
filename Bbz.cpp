/*
  Bbz.cpp 

  Created by Bruno Anastácio, Bruno Fonseca, José Guerra
  MSc in Computer Engineering Mobile Computing - Mobility in Computational Systems
  year: 2018
*/


//#include <JeeLib.h>
//#define RF69_COMPAT 0
#include "Bbz.h"


ISR(WDT_vect) { 
  Sleepy::watchdogEvent();
} // interrupt handler for JeeLabs Sleepy power saving


char _messageReceived[MAX_MSG_SIZE];

Requester::Requester(){ }

// initialize rfm12b requester/transmiter
void Requester::init(int id_node){
  pinMode(4,OUTPUT); // Funky v3 RFM12B power control pin
  digitalWrite(4,LOW); //Make sure the RFM12B is on, yes LOW is ON
  delay(100); // Delay (or sleep) to allow the RFM12B to start up
  rf12_initialize(id_node,RF12_868MHZ,1); 

 // rf12_sleep(0); // set to sleep mode
  Serial.begin(9600);
  delay(5000);
  Serial.println("Initialize requester!");
}

float Requester::readTemperature(){
  float temp = 0;
  int randomNumber = random(MIN, MAX);
  int randomNumberDecimal = random(0,9);

  if(randomNumberDecimal == 0){
    temp = (float) randomNumber;
    return temp;
  }
  temp =  ((float)randomNumber / (float)randomNumberDecimal)+10;
  //Serial.println("Temperatura obtida!");
  _lastTemp = temp;
  return temp;
}


void Requester::displayTemperature(float temp){
  String service ="s:display";
  Requester::prepareMessage(temp);
  Requester::searchService(service);
}

void Requester::prepareMessage(float temp){
  if(temp > 0){
    String msg ="";
    msg = String("T: ") + temp + String(" ºC ");
    msg.toCharArray(_messageTX, MAX_MSG_SIZE);
    return;
  }
}

void Requester::searchService(String service){

  int service_id = -1;
  //procurar nós com serviço de display
  if(_last_service_id == -1){
    Serial.println("Searching for a service: " + service);
      int searchingNode = 1;
      uint32_t tStart = millis();
      while(searchingNode == 1){
        service_id = Requester::waitServices(service);
        if(service_id != -1){
          searchingNode = 0;
        }
        if((millis()-tStart) >= 15000){
          searchingNode = 0;
        }
      }
  }else{
    Serial.println("Already know a service with function: " + service);
    service_id = _last_service_id;
  }

  if(service_id == -1){
    Serial.print("Wanted services not found. - SLEEP: ");
    Serial.print(SLEEP_TIME_REQUESTER/1000); Serial.print(" SECONDS");Serial.println();
    delay(15000);
    return;
  }

  Requester::sendMessage(service_id); 
}


int Requester::waitServices(String service)
{
  char serviceAux[MAX_MSG_SIZE]; 
  int n_id = -1;
  if (rf12_recvDone()) //Is something arriving?
    {
      if (rf12_crc == 0) //No errors?
      {
          if((rf12_hdr & RF12_HDR_CTL) == 0) //Data packet?
          {
            n_id = (rf12_hdr & 0x1F); //Source address
            strcpy(_messageReceived, rf12_data);
           // _messageReceived = rf12_data; // Data extraction
            Serial.print("Service found: "); Serial.print(_messageReceived); Serial.println(); 
            service.toCharArray(serviceAux, MAX_MSG_SIZE); 
            delay(100);
            if(strcmp(_messageReceived, serviceAux)== 0){   // validar se é o serviço desejado
              return n_id;
            }
            return -1;
          }
          else
          {
            //Serial.println(“It is noot a data packet! Ack?");
          }
       }
       else
        {
        //Serial.println(“CRC error…");
        }
    }
    return n_id;
}

void Requester::sendMessage(int rx_node_id)
{
  for( uint32_t tStart = millis();  (millis()-tStart) < SEND_TIME; ){
     //rf12_sleep(-1); // Wake up RF module
      while (!rf12_canSend())
        rf12_recvDone();

      rf12_sendStart(rx_node_id, &_messageTX, sizeof _messageTX); // rx_node_id default = 0 to broadcast
      rf12_sendWait(0); // Wait for RF to finish sending
      Serial.print("Send message: "); Serial.print(_messageTX);
      Serial.print(" to the node: "); Serial.print(rx_node_id);
      Serial.println();

      // wait ACK
      receiveACK = Requester::waitACK();

      if(receiveACK == 1){
        return;
      }
      if(receiveACK != 1){
        failTimes ++;
      }
      if(failTimes == 10){
        Serial.println("Fail "); Serial.print(failTimes); Serial.println(" times, now go search for new service!");
        Serial.println();
        _last_service_id = -1;// procurar serviço e enviar mensagem
        receiveACK = 0;
        failTimes = 0;
      }

      // rf12_sleep(0); // Put RF module to sleep
      //Sleepy::loseSomeTime(500); //sleep 500 ms
      delay(INTERVAL_BETWEEN_SEND);
  }
}

bool Requester::waitACK()
{
  uint32_t timeInterval = 1 * 5;       //5 milisegundos
  char ack[4]; char confirmAck[4]; String _ack = "ack";
  int service_id;

  // wait ACK
  for( uint32_t tStart = millis();  (millis()-tStart) < timeInterval;  ){
    if (rf12_recvDone() && rf12_crc == 0) 
    {
      if((rf12_hdr & RF12_HDR_CTL) == 0) //Data packet?
      {
        service_id = (rf12_hdr & 0x1F); //Source address
        strcpy(ack, rf12_data);
        _ack.toCharArray(confirmAck, 4); 
        if(strcmp(ack, confirmAck)== 0){   // validar se é o serviço desejado
          Serial.print("RECEIVED ACK - SLEEP "); Serial.print(SLEEP_TIME_REQUESTER/1000); Serial.print(" SECONDS");Serial.println();
          _last_service_id = service_id;
          receiveACK = 0;
          failTimes = 0;
          delay(SLEEP_TIME_REQUESTER);
          return 1;
        }     
      }
    }else{
      //return 0;
    }
  }
  return 0;  
}


Advertiser::Advertiser(){ }

// initialize rfm12b advertiser/service
void Advertiser::init(int id_node){
  pinMode(4,OUTPUT); // Funky v3 RFM12B power control pin
  digitalWrite(4,LOW); //Make sure the RFM12B is on, yes LOW is ON
  delay(100); // Delay (or sleep) to allow the RFM12B to start up
  rf12_initialize(id_node,RF12_868MHZ,1); 
  // rf12_control(0xC000); // Adjust low battery voltage to 2.2V, only for RFM12B
  Serial.begin(9600);
  delay(2000);
  Serial.println("SERVICE/ADVERTISER NODE ...");
}

void Advertiser::advertise(String service){ // anuncia o serviço durante 3 segundos de 0.5 em 0.5 segundos, depois fica 10 segundos à escuta
  String msg ="s:"+service;
  msg.toCharArray(_messageToAdvertise, MAX_MSG_SIZE);

  if(boolTime == true){
    start_time = millis();
    boolTime = false;
  }

  if(sum_time < DELAY_TO_STOP_ADVERTISE){
    while (!rf12_canSend())
      rf12_recvDone();

    Serial.println("Advertise: s:display");
    rf12_sendStart(0, &_messageToAdvertise, sizeof _messageToAdvertise); // 0 to broadcast service
    rf12_sendWait(0); // 0= nome / 2=standby Wait for RF to finish sending
    delay(DELAY_BETWEEN_ADVERTISE);

    sum_time = (millis()-start_time); 
  }else{
   // rf12_sleep(-1); 
    sum_time = 0;
    boolTime = true;

    Serial.println("Wait for messages!");
    for( uint32_t tStart = millis();  (millis()-tStart) < RECEIVE_TIME;  ){
      Advertiser::receive();
    }
  }

}

void Advertiser::receive(){
    if (rf12_recvDone()) //Is something arriving?
    {
      if (rf12_crc == 0) //No errors?
      {
          if((rf12_hdr & RF12_HDR_CTL) == 0) //Data packet?
          {
            int n_id = (rf12_hdr & 0x1F); //Source address
            strcpy(_messageReceived, rf12_data);
           // _messageReceived = rf12_data; // Data extraction
            Serial.print("Message Received: "); Serial.print(_messageReceived);
            Serial.print(" Sent by the node: ");Serial.print(n_id);
            Serial.println();

            Advertiser::sendACK(n_id);
          }else{
            //Serial.println(“It is noot a data packet! Ack?");
          }
       }else{
        //Serial.println(“CRC error…");
        }
    }  
}

void Advertiser::sendACK(int n_id){
    //send ACK
    String msg ="ack";
    char ack[4];
    msg.toCharArray(ack, 4);
    rf12_sendStart(n_id, &ack, sizeof ack); // 0 to broadcast service
    rf12_sendWait(0);
    Serial.print("SEND ACK - SLEEP "); Serial.print(SLEEP_TIME_ADVERTISER/1000); Serial.print(" SECONDS");Serial.println();
    delay(SLEEP_TIME_ADVERTISER);
}