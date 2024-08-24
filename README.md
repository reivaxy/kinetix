# KINETIX HAND

French: https://github-com.translate.goog/reivaxy/kinetix?_x_tr_sl=en&_x_tr_tl=fr&_x_tr_hl=fr

Spanish: https://github-com.translate.goog/reivaxy/kinetix?_x_tr_sl=en&_x_tr_tl=fr&_x_tr_hl=fr

German: https://github-com.translate.goog/reivaxy/kinetix?_x_tr_sl=en&_x_tr_tl=fr&_x_tr_hl=fr

> [!NOTE]
> The work in this repository is released under licence Creative Commons 
> Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0) open-source license<br/>
> Please read this license to understand what you may and may not do with these files (software and 
> hardware):<br/> 
> https://creativecommons.org/licenses/by-nc-sa/4.0/<br/><br/>
> The 3D part of this work is based on the Kinetic Hand, created and released by Mat Bowtell:<br/>
> https://www.thingiverse.com/thing:4618922<br/><br/>
> To me, the most important part of it is "non commercial": do not use this work to sell anything, 
> in any way, no matter how derivative it is.<br/><br/>
> =============== DISCLAIMER of liability =============== <br/>
> This is not a medical device. You must not use it as a missing limb replacement without checking with
> medical professionals if this is appropriate.<br/>
> Also, this is still a work in progress and should not be used until it has reached some level of 
> completion and testing.<br/>  
> You build it and use it at your own risks.  

         
### Important: you need not build the firmware or the android app, binaries are available in the [releases section](https://github.com/reivaxy/kinetix/releases)

I've been a maker and board member of [the French e-Nable association](https://e-nable.fr/fr/) for a few years, and I've 
worked on several 3D printed devices intended for people missing part of a hand or an arm.

The most recent of these devices is the [Kinetic hand, designed by Mat Bowtell](https://www.thingiverse.com/thing:4618922), which is easy to print and assemble, with 
a smoother and more natural look than older devices such as the Phoenix Unlimbited.

These devices are muscle activated and require their user to still have a functional wrist or an elbow.
So far I've only seen one low cost motor activated 3D printed device, the [Exii Hackberry](https://www.exiii-hackberry.com/). 
It's a great device, but I found it somehow uneasy to print and assemble, it's still a bit expensive, and I thought
more recent electronic components would allow going further.

Which is why in 2023 I started to work on this project to combine both new hand design, and recent 
electronic microcontroller to drive the servomotors. I've investigated several solutions and here is the most promising one.

The cost is estimated around $60.

**BEWARE**: for now, voice control via bluetooth is only available with the ANDROID application. 
There is no iPhone application. If you are willing to develop a free open source iPhone application please get in touch :) 


BEFORE YOU START ANYTHING, please check the [Wiki pages for printing and assembling instructions](https://github.com/reivaxy/kinetix/wiki), especially the page on parts and Servo Motors that come in different shapes.

This is still very much a Work In Progress:
- the onboard current monitoring is still being tested and tweaked, which means **for now the motors will not stop for obstacles and might 
  hurt you** (although they don't have that much strength, better safe than sorry)
- the bluetooth connectivity and phone app are not started yet

                                                      
Like the Hackberry and unlike the Kinetic, due to tight mechanical constraints, the Kinetix can't be resized to fit smaller limbs.
I have another project which might help with this, that is using only one big servo for all fingers, of course with
very different capabilities and features.

Here is the early prototype, testing speed:

https://github.com/reivaxy/kinetix/assets/3592991/b644f965-3c86-4bcd-a1fd-c3cf471b4016

And here is the status on May 12th 2024. Check [latest release](https://github.com/reivaxy/kinetix/releases/) for APK with voice control (please enable sound on this video or it won't make much sense :) )

https://github.com/reivaxy/kinetix/assets/3592991/68d8df24-c723-40e2-a849-26ecd64f889a
                                                                                                            
