# esp32_dmx_cable_tester_web
dmx/xlr cable tester based on ESP32 board with web interface via wi-fi

Connection scheme:  
esp32 pins <----> DMX connectors pins 

<pre>[ ESP32 ]                           [ XLR Female ]  
GPIO 13 ---------------------------> Pin 1 (screen/ground)  
GPIO 12 ---------------------------> Pin 2 (data-)  
GPIO 14 ---------------------------> Pin 3 (data+)  

[ XLR Male ]                        [ ESP32 ]  
Pin 1 (screen/ground) -------------> GPIO 27  
Pin 2 (data-) ---------------------> GPIO 26  
Pin 3 (data+) ---------------------> GPIO 25  
</pre>
