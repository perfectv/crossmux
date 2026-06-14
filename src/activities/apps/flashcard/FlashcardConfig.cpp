#include "FlashcardConfig.h"
#include <HalStorage.h>
#include <Logging.h>
#include <cstdio>
#include <cstring>

static constexpr const char* TAG = "FlashcardConfig";
static constexpr const char* CONFIG_PATH = "/flashcard/settings.txt";//SD卡上的配置文件地址

// 全局配置实例定义
FlashcardConfig g_flashcardConfig;

bool loadFlashcardConfig() {
    // 先设置默认值，防止文件不存在时配置为 0
    g_flashcardConfig.newPerDay = 10;
    g_flashcardConfig.maxReview = 50;

    HalFile file;
    if (!Storage.openFileForRead(TAG, CONFIG_PATH, file)) {
        // 文件不存在，使用默认值
        LOG_DBG(TAG, "No config file, using defaults");
        return false;
    }

    // 读取整个文件（预计不超过256字节）
    char buf[256];
    size_t bytesRead = file.read(reinterpret_cast<uint8_t*>(buf), sizeof(buf) - 1);
    buf[bytesRead] = '\0';
    file.close();

    // 解析每一行，格式 key=value
    char* line = buf;
    char* nextLine;
    while (line && *line) {
        // 跳过空行和注释（可选）
        if (*line == '\n' || *line == '\r') {
            line++;
            continue;
        }
        // 查找换行符
        nextLine = strchr(line, '\n');
        if (nextLine) *nextLine = '\0';

        // 查找等号
        char* eq = strchr(line, '=');
        if (eq) {
            *eq = '\0';
            const char* key = line;
            const char* value = eq + 1;

            // 去除尾部空格（简单处理）
            while (*value == ' ') value++;

            if (strcmp(key, "newPerDay") == 0) {
                int v = atoi(value);
                if (v >= 5 && v <= 50) g_flashcardConfig.newPerDay = (uint8_t)v;
            } else if (strcmp(key, "maxReview") == 0) {
                int v = atoi(value);
                if (v >= 25 && v <= 250) g_flashcardConfig.maxReview = (uint8_t)v;
            }
        }

        line = nextLine ? nextLine + 1 : nullptr;
    }

    LOG_DBG(TAG, "Loaded config: newPerDay=%d, maxReview=%d",
            (int)g_flashcardConfig.newPerDay, (int)g_flashcardConfig.maxReview);
    return true;
}

void saveFlashcardConfig() {
    // 先确保目录存在
    Storage.ensureDirectoryExists("/flashcard");

    HalFile file;
    if (!Storage.openFileForWrite(TAG, CONFIG_PATH, file)) {
        LOG_ERR(TAG, "Failed to open config for write");
        return;
    }

    char buf[128];
    int len = snprintf(buf, sizeof(buf), "newPerDay=%d\nmaxReview=%d\n",
                       (int)g_flashcardConfig.newPerDay,
                       (int)g_flashcardConfig.maxReview);
    file.write(reinterpret_cast<const uint8_t*>(buf), (size_t)len);
    file.close();
    LOG_DBG(TAG, "Config saved");
}