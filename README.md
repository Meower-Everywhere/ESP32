# Meower for ESP32
Access Meower from the serial output on your ESP32 dev board. Tested with a Duinotech ESP32 board, should work on any ESP32-WROOM-32 board and most other ESP32 Dev Boards.

<img src="https://github.com/Meower-Everywhere/ESP32/blob/02e3565fd8fe1ebe9b65a5a673d2db1889371809/assets/meower-esp32.png" height="333" width="333">

# Installation
1. Download [meower.ino](https://github.com/Meower-Everywhere/ESP32/blob/9c3765a1a6e9f89ebd2904138feb9e090b19d283/meower.ino) to your computer and open Arduino IDE.
2. In Arduino IDE, open the meower.ino file you just downloaded
3. Go to Tools > Partition Scheme and change it to Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)
<img width="465.5" alt="Partition Scheme in Arduino IDE" src="https://github.com/Meower-Everywhere/ESP32/assets/72828296/744ad746-fc56-4a76-881b-dc95eccb3dc1">

4. Install ArduinoJson by Benoit Blanchon from the Library Manager
5. Connect your ESP32 board to your computer if you haven't already and click the Upload button
6. Open the Serial Monitor from Tools > Serial Monitor, make sure it is on 115200 baud and that the line ending is set to New Line
7. Press the EN button (or whatever button is to the left of the USB port when it is facing you)
8. Follow the steps to setup WiFi and Webhooks Username using the serial monitor
9. Once done, your WiFi info and username will be saved and you'll see the most recent post along with any new posts that come through.

# Usage
## Reading Posts
Once your ESP32 connects to WiFi, you'll see the most recent post appear. Your ESP32 will contact the Meower API to check for new posts every 0.5 seconds. If there are any new posts, they will be printed to serial output.
## Posting
To post, type post: followed by a space and the content you want to post into the serial monitor and press enter
## Changing Username
To change your username, type username: followed by a space and the username you want to use. Innaprorpiate usernames will result in a IP ban from Webhooks, which will completely prevent you from posting on your ESP32.
## Changing WiFi networks
By default, if the ESP32 cannot find the saved WiFi network after 15 seconds, it will ask you for a different SSID and Password, however if you want to use a different WiFi network while the saved one is still available, type wifi into the serial monitor and press enter to restart the WiFi setup wizard.

# Meower Everywhere Feature Checklist
- [x] - Viewing Posts
- [x] - Making Posts
- [x] - WiFi setup wizard*
- [x] - Discord & Revower bridge support
- [x] - Webhooks bridge support
- [x] - Username changer*
- [x] - Connecting to Meower via API
- [ ] - Logging in with a username and password
- [ ] - ~~Connecting to Meower via websockets~~
- [ ] - ~~Editing Posts~~
- [ ] - ~~Deleting Posts~~

`*` | Feature is a platform-specific requirement

~~Item~~ | Feature is not feasible on this specific platform
