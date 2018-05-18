#include <Bbz.h> // reference to our library 

Advertiser a = Advertiser(); // invoke advertiser from library 

void setup() {
    a.init(2); // initialize advertiser, 2 is the id of node in network
}

void loop() {
  	// display in this test is serial console
  	a.advertise("display");
}
