#pragma once

#include <cstdint>
#include <string>

#include "../../../util/ButtonNavigator.h"
#include "../../Activity.h"

class WeReadSetupActivity final : public Activity {
 public:
  WeReadSetupActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);
  ~WeReadSetupActivity() override = default;

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  ButtonNavigator buttonNavigator;
  int selected = 0;

  // Row layout:
  //   0 = Set/Replace Key
  //   1 = Clear Key (only when keyPresent)
  enum class Banner : uint8_t { None, Saved, Cleared, Invalid };
  Banner banner = Banner::None;

  bool keyPresent = false;

  void refresh();
  int itemCount() const;
  void onSelect();
};
