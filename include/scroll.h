#pragma once
#include <M5Unified.h>

class PanelSystem; 
 
class ScrollSystem {
private:
  int touchStartY = 0;
  int scrollStartOffSet = 0;
  bool isDragging = false;
 
public:
  void handleScroll(PanelSystem& panels, std::function<void()> redrawAll);
};
 