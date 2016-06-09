# GEVCUExtension
The solution for a separate Arduino Due based CAN device which interacts with GEVCU.
It does the following:

* Control a 16-channel 12V relay board to control various functions in the car (like HV contactors, brake light, pumps, ...). 
  The control commands are issued by GEVCU and received via CAN bus.
* Control an Eberspaecher 6kW water heater via 33.3kbps SW-CAN
* Collect data from temperature sensors and report them via CAN bus to GEVCU
* Collect data from water flow sensors (heater and cooler) and send them via CAN bus to GEVCU
