#include "ButtonNavigator.h"

const MappedInputManager* ButtonNavigator::mappedInput = nullptr;

void ButtonNavigator::onNext(const Callback& callback) {
  onNextPress(callback);
  onNextContinuous(callback);
}
void ButtonNavigator::onPrevious(const Callback& callback) {
  onPreviousPress(callback);
  onPreviousContinuous(callback);
}

void ButtonNavigator::onNextPress(const Callback& callback) { onPress(getNextButtons(), callback); }

void ButtonNavigator::onPreviousPress(const Callback& callback) { onPress(getPreviousButtons(), callback); }

void ButtonNavigator::onNextContinuous(const Callback& callback) { onContinuous(getNextButtons(), callback); }

void ButtonNavigator::onPreviousContinuous(const Callback& callback) { onContinuous(getPreviousButtons(), callback); }

void ButtonNavigator::onPress(const Buttons& buttons, const Callback& callback) {
  bool buttonPressed = false;
  for (const MappedInputManager::Button button : buttons) {
    if (mappedInput->wasPressed(button)) {
      buttonPressed = true;
      break;
    }
  }

  if (buttonPressed && !recentlyNavigatedContinuously()) {
    callback();
  }
}

void ButtonNavigator::onContinuous(const Buttons& buttons, const Callback& callback) {
  bool buttonPressed = false;
  for (const MappedInputManager::Button button : buttons) {
    if (mappedInput->isPressed(button)) {
      buttonPressed = true;
      break;
    }
  }

  if (buttonPressed && shouldNavigateContinuously()) {
    callback();
    lastContinuousNavTime = millis();
  }
}

bool ButtonNavigator::shouldNavigateContinuously() const {
  const bool buttonHeldLongEnough = mappedInput->getHeldTime() > continuousStartMs;
  const bool navigationIntervalElapsed = (millis() - lastContinuousNavTime) > continuousIntervalMs;

  return buttonHeldLongEnough && navigationIntervalElapsed;
}

bool ButtonNavigator::recentlyNavigatedContinuously() {
  if (lastContinuousNavTime != 0) {
    lastContinuousNavTime = 0;
    return true;
  }

  return false;
}