# Single webpage application to manage the KinetiX 

This is a single webpage application that takes advantage of Chrome's BLE access features to connect to a KinetiX and control it.

It also works with Edge on Windows and MacOs.

> [!WARNING]
> Of course it won't work on iOS. Browsers on iOS are limited to Apple's WebKit, which does not implement BLE access.
>
> But it works on MACOs (Chrome and Edge, no Safari nor Firefox)... Apple ecosystem homogeneity is a joke.
> 


## On your local server
If you cloned or forked this repo, you can run a local server on your machine and access this page through http://localhost

The easiest way to do that is to open this file in IntelliJ or Android Studio (which is based on IntelliJ) and click on a browser preview icon
(pick only Chrome or Edge, Firefox does not provide Bluetooth BLE access), it will create a temporary local server and display the page.

<img width="640" alt="image" src="https://github.com/user-attachments/assets/fd005c96-2a11-4ca3-bbdd-00bba3bbe5e1" />

## Published page
Or, you can access its published version here, which does not send any information anywhere else but your KinetiX device!

https://reivaxy.github.io/kinetix/


<img width="494" height="789" alt="image" src="https://github.com/user-attachments/assets/9585c898-c002-4bfa-b0b3-7a15c2eb02db" />

The content of this folder is deployed by a github workflow to the page above each time something is pushed to it.


> [!IMPORTANT]
> Voice Recognition is coming soon! It is not as convenient as the Android App, but... it works.

More information in the wiki: https://github.com/reivaxy/kinetix/wiki/09.-Chrome-Browser-Application
