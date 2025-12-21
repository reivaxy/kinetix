# Python script

This scipt allows to connect to the hand and send it commands.

> [!WARNING] 
> You need to have a Bluetooth LE stack on your PC. With a regular Bluetooth you won't be able to connect.
> If your default BT is the non LE one, you can add a BLE dongle on a USB port, it seems to be working.

`python kinetixGui.py`

Click on connect, it may need around 30 seconds to connect. Then click the buttons to send orders. Custom orders can be sent through the input field.

It should work with the the pre-filled name "Kinetix". If it does not try finding the Mac address of the ESP and use the Address input field and radio button.

An executable for windows (generated from the python script) is also provided, double clik it in your file explorer.

