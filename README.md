# ai-embedded
BrainUI module for Esp32 and Arduino. 


AI-Camera modules start stuttering when used with GPIO pins. Use ESP httpd server for unblocking gpio access when streaming video.

Do not use EPS32 cheap camera modules, they are very slow. They clock at 40 to 80mhz

## Code Editor
Use Visual Code and plaform io extension.
Select environment according to your board

#### Directory Structure
--src
--src--bleCar Bluetooth car control (BLE), with steering and one drive motor
--src--carWithCamera Wifi streaming of esp32 camera with wifi control for motors

#### AT Commands Help
TBD


#### API Endpoints for streaming
TBD


#### Communication CMD Structure