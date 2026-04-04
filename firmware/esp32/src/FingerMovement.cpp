#include "FingerMovement.h"


FingerMovement::FingerMovement(int _relativeTargetPosition) {
   normalizedTargetPosition = _relativeTargetPosition;
   startDelay = 0;
   step = DEFAULT_STEP;
}

FingerMovement::FingerMovement(int _relativeTargetPosition, uint32_t _startDelay) {
   normalizedTargetPosition = _relativeTargetPosition;
   startDelay = _startDelay;
   step = DEFAULT_STEP;
}

FingerMovement::FingerMovement(int _relativeTargetPosition, uint32_t _startDelay, float _step) {
   normalizedTargetPosition = _relativeTargetPosition;
   startDelay = _startDelay;
   step = _step;
}

FingerMovement::~FingerMovement() {  
}