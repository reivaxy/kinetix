# Tests with RP2040

These sources are preliminary tests using an RP2040, which can't be used with the PCB provided in this repository.

They are not maintained, no new feature or bug fix will be done on them.

They allow chaining predefined finger positions with timings for each position, and looping with very simple instructions such as

```
  hand = new Hand();
  hmf = new HandMovementFactory(hand);
  seq = new Sequence(3); // 0 is repeat forever
  seq->addMovement(hmf->fist());
  seq->addMovement(hmf->one());
  seq->addMovement(hmf->two());
  seq->addMovement(hmf->three());
  seq->addMovement(hmf->four());
  seq->addMovement(hmf->idle(), 5000);
  seq->addMovement(hmf->openPinch());
  seq->addMovement(hmf->ok());
  seq->addMovement(hmf->pointing());
  seq->addMovement(hmf->closePinch());

  seq->start();
```


This video (not related to code example above) was testing speed:

https://github.com/reivaxy/kinetix/assets/3592991/b644f965-3c86-4bcd-a1fd-c3cf471b4016
                        
Wiring exemple:
                                   
<img src="img/rp2040.jpg" width="512px">

Prototype:

<img src="img/handRP2040.jpg" width="512px">
