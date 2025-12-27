# Python script

The main.py python script allows to connect to the hand and send it commands, get and update the experimentation settings, and upgrade the firmware through Wifi, from a personal computer.

<img width="512px" src="https://github.com/user-attachments/assets/f8a4f54d-439f-4846-b640-edd77aa169e8">

It was tested on Linux (Ubuntu) and Windows abut should work on Mac too.

> [!WARNING] 
> You need to have a Bluetooth LE stack on your PC. With a regular Bluetooth you won't be able to connect.
> If your default BT is the non LE one, you can add a BLE dongle on a USB port, it seems to be working.

`python main.py`

Click on connect, it may need around 30 seconds to connect the first time because it's connecting by name. The next times will use the device MAC address and will be faster.

Then click the buttons to send orders. Custom orders can also be sent through the input field.

If you don't want to install and use Python, a standalone executable for windows (generated from the python script) will be provided in the Releases.

A standalone for Mac should also be doable, as well as apps for mobile, which remains to be investigated.

I'm currently working on producing an android app from this script using Buildozer, which also might work for iPhones at some point.
