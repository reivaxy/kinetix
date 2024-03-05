#include "FingerMovement.h"


FingerMovement::FingerMovement(float _relativeTargetPosition) {
   relativeTargetPosition = _relativeTargetPosition;
   startDelay = 0;
   step = DEFAULT_STEP;
}

FingerMovement::FingerMovement(float _relativeTargetPosition, uint32_t _startDelay) {
   relativeTargetPosition = _relativeTargetPosition;
   startDelay = _startDelay;
   step = DEFAULT_STEP;
}

FingerMovement::FingerMovement(float _relativeTargetPosition, uint32_t _startDelay, float _step) {
   relativeTargetPosition = _relativeTargetPosition;
   startDelay = _startDelay;
   step = _step;
}