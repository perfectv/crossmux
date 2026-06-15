#include "FlashcardSettingsActivity.h"

#include <GfxRenderer.h>
#include <HalStorage.h>
#include <I18n.h>

#include "components/UITheme.h"
#include "fontIds.h"

static constexpr int SETTINGS_COUNT = 2;
static constexpr uint8_t NEW_PER_DAY_MIN  = 5;
static constexpr uint8_t NEW_PER_DAY_MAX  = 50;
static constexpr uint8_t NEW_PER_DAY_STEP = 5;
static constexpr uint8_t MAX_REVIEW_MIN   = 25;
static constexpr uint8_t MAX_REVIEW_MAX   = 250;
static constexpr uint8_t MAX_REVIEW_STEP  = 25;

void FlashcardSettingsActivity::onEnter() {
    Activity::onEnter();
    // 从文件读取配置，如果文件不存在则使用默认值
    newPerDay = 10;
    maxReview = 50;
    HalFile file;
    if (Storage.openFileForRead("FlashcardSettings", "/flashcard/settings.txt", file)) {
        char buf[128];
        int n = file.read((uint8_t*)buf, sizeof(buf) - 1);
        buf[n] = '\0';
        file.close();
        // 解析 key=value 格式
        char* p = strstr(buf, "newPerDay=");
        if (p) {
            int v = atoi(p + 10);
            if (v >= NEW_PER_DAY_MIN && v <= NEW_PER_DAY_MAX) newPerDay = (uint8_t)v;
        }
        p = strstr(buf, "maxReview=");
        if (p) {
            int v = atoi(p + 10);
            if (v >= MAX_REVIEW_MIN && v <= MAX_REVIEW_MAX) maxReview = (uint8_t)v;
        }
    }
    // Clamp to valid range
    if (newPerDay < NEW_PER_DAY_MIN) newPerDay = NEW_PER_DAY_MIN;
    if (newPerDay > NEW_PER_DAY_MAX) newPerDay = NEW_PER_DAY_MAX;
    if (maxReview < MAX_REVIEW_MIN) maxReview = MAX_REVIEW_MIN;
    if (maxReview > MAX_REVIEW_MAX) maxReview = MAX_REVIEW_MAX;

    requestUpdate();
}

void FlashcardSettingsActivity::saveSettings() {
    // 确保目录存在
    Storage.ensureDirectoryExists("/flashcard");
    HalFile file;
    if (!Storage.openFileForWrite("FlashcardSettings", "/flashcard/settings.txt", file)) {
        return;
    }
    char buf[64];
    int len = snprintf(buf, sizeof(buf), "newPerDay=%d\nmaxReview=%d\n", newPerDay, maxReview);
    file.write((const uint8_t*)buf, (size_t)len);
    file.close();
}

void FlashcardSettingsActivity::loop() {
    if (!editing) {
        buttonNavigator.onNext([this] {
            selectedIndex = ButtonNavigator::nextIndex(selectedIndex, SETTINGS_COUNT);
            requestUpdate();
        });

        buttonNavigator.onPrevious([this] {
            selectedIndex = ButtonNavigator::previousIndex(selectedIndex, SETTINGS_COUNT);
            requestUpdate();
        });

        if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
            editing = true;
            requestUpdate();
        }

        if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
            saveSettings();
            finish();
        }
    } else {
        // Editing mode: Up increases value, Down decreases value
        buttonNavigator.onNext([this] {
            if (selectedIndex == 0) {
                if (static_cast<int>(newPerDay) + NEW_PER_DAY_STEP <= NEW_PER_DAY_MAX)
                    newPerDay += NEW_PER_DAY_STEP;
                else
                    newPerDay = NEW_PER_DAY_MAX;
            } else {
                if (static_cast<int>(maxReview) + MAX_REVIEW_STEP <= MAX_REVIEW_MAX)
                    maxReview += MAX_REVIEW_STEP;
                else
                    maxReview = MAX_REVIEW_MAX;
            }
            requestUpdate();
        });

        buttonNavigator.onPrevious([this] {
            if (selectedIndex == 0) {
                if (static_cast<int>(newPerDay) - NEW_PER_DAY_STEP >= NEW_PER_DAY_MIN)
                    newPerDay -= NEW_PER_DAY_STEP;
                else
                    newPerDay = NEW_PER_DAY_MIN;
            } else {
                if (static_cast<int>(maxReview) - MAX_REVIEW_STEP >= MAX_REVIEW_MIN)
                    maxReview -= MAX_REVIEW_STEP;
                else
                    maxReview = MAX_REVIEW_MIN;
            }
            requestUpdate();
        });

        if (mappedInput.wasReleased(MappedInputManager::Button::Confirm) ||
            mappedInput.wasReleased(MappedInputManager::Button::Back)) {
            editing = false;
            requestUpdate();
        }
    }
}

void FlashcardSettingsActivity::render(RenderLock&&) {
    const auto& metrics = UITheme::getInstance().getMetrics();
    const int pageWidth = renderer.getScreenWidth();
    const int pageHeight = renderer.getScreenHeight();

    renderer.clearScreen();

    GUI.drawHeader(renderer,
                   Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight},
                   tr(STR_FLASHCARD_SETTINGS));

    const int menuTop = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
    const int menuHeight = pageHeight - menuTop - metrics.buttonHintsHeight - metrics.verticalSpacing;

    // Build value strings, marking the editing row with an indicator
    char val0[16], val1[16];
    if (editing && selectedIndex == 0) {
        snprintf(val0, sizeof(val0), "[ %d ]", static_cast<int>(newPerDay));
    } else {
        snprintf(val0, sizeof(val0), "%d", static_cast<int>(newPerDay));
    }
    if (editing && selectedIndex == 1) {
        snprintf(val1, sizeof(val1), "[ %d ]", static_cast<int>(maxReview));
    } else {
        snprintf(val1, sizeof(val1), "%d", static_cast<int>(maxReview));
    }

    const char* values[SETTINGS_COUNT] = {val0, val1};

    GUI.drawList(
        renderer,
        Rect{0, menuTop, pageWidth, menuHeight},
        SETTINGS_COUNT,
        selectedIndex,
        [](int i) -> std::string {
            if (i == 0) return tr(STR_FLASHCARD_NEW_PER_DAY);
            return tr(STR_FLASHCARD_MAX_REVIEW);
        },
        nullptr,
        nullptr,
        [&values](int i) -> std::string {
            return values[i];
        },
        true);

    const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT),
                                             tr(STR_DIR_UP), tr(STR_DIR_DOWN));
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

    renderer.displayBuffer();
}