#include <Bbz.h> // reference to our library 

Requester r = Requester();  // invoke requester from library 
float temp = 0; // initialize float variable to store the read value of the temperature

void setup() {
   r.init(1); // initialize advertiser, 1 is the id of node in network
}

void loop() {
  temp = r.readTemperature(); // call to read a new temperature(RANDOM VALUES)
  r.displayTemperature(temp);  // function responsible for displaying information on another network node
}
