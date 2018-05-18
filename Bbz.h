/*
  Bbz.h

  Created by Bruno Anastácio, Bruno Fonseca, José Guerra
  MSc in Computer Engineering Mobile Computing - Mobility in Computational Systems
  year: 2018
*/


#ifndef Bbz_h
#define Bbz_h

#include <JeeLib.h>
#define RF69_COMPAT 0


#define RF12_433MHZ     1   ///< RFM12B 433 MHz frequency band.
#define RF12_868MHZ     2   ///< RFM12B 868 MHz frequency band.
#define RF12_915MHZ     3 ///< RFM12B 915 MHz frequency band.

#define MIN 5
#define MAX 40

#define SLEEP_TIME_REQUESTER 15000
#define SLEEP_TIME_ADVERTISER 5000
#define SEND_TIME 10000 //10 seconds
#define INTERVAL_BETWEEN_SEND 1000 //1 second
#define RECEIVE_TIME 20000 //20 seconds


class Requester
{
  public:
    Requester();
    void init(int id_node);
    float readTemperature();
    void displayTemperature(float temp = 0); // 0 is defaut and represent error

  private:
    char _messageTX[50]; 
    int _last_service_id = -1;
    float _lastTemp;
    bool receiveACK = 0;
    int failTimes = 0;

    void prepareMessageSearchService(float temp, String service);
    int waitServices(String service);
    void sendMessage(int rx_node_id = 0); // 0 is default to broadcast
    bool Requester::waitACK();
};


class Advertiser
{
  public:
    Advertiser();
    void init(int id_node);
    void advertise(String service = "");

  private: 
    bool boolTime = true;
    unsigned long start_time = 0;
    unsigned long sum_time = 0;
    unsigned long delayBetweenAdvertise = 500; // 0.5 segundo
    unsigned long delayToStopAdvertise = 3000; // 3segundos
    char _messageToAdvertise[50];

    void sendACK(int n_id);
    void receive();

};


#endif

