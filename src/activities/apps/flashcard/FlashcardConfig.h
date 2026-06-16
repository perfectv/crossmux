#pragma once
#include <cstdint>

struct FlashcardConfig {
  uint8_t newPerDay = 10;
  uint8_t maxReview = 50;
};

extern FlashcardConfig g_flashcardConfig;

bool loadFlashcardConfig();
void saveFlashcardConfig();