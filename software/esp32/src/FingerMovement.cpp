#include "FingerMovement.h"


FingerMovement::FingerMovement(int _relativeTargetPosition) {
   relativeTargetPosition = _relativeTargetPosition;
   startDelay = 0;
   step = DEFAULT_STEP;
}

FingerMovement::FingerMovement(int _relativeTargetPosition, uint32_t _startDelay) {
   relativeTargetPosition = _relativeTargetPosition;
   startDelay = _startDelay;
   step = DEFAULT_STEP;
}

FingerMovement::FingerMovement(int _relativeTargetPosition, uint32_t _startDelay, float _step) {
   relativeTargetPosition = _relativeTargetPosition;
   startDelay = _startDelay;
   step = _step;
}