#pragma once

#include <ArduinoJson.h>

#include <string>

#include "WeReadClient.h"

/**
 * Per-book 5-step cache flow.
 *
 * Single source of truth for "which APIs to hit, how to filter, and how to
 * persist the response". Both `WeReadCacheBookActivity` (single-book UI) and
 * `WeReadSyncAllActivity` (bulk shelf sync) drive themselves through these
 * helpers — adding/removing a step only needs to be done here.
 *
 * Steps run in order: 笔记 → 公开书评 → 章节 → 热门标注 → 相似推荐.
 *
 * On the SD side: `WeReadCacheStore` owns the file layout; we just call into
 * its save* / load* functions with the parsed POD vectors.
 */
namespace WeReadBookCacheFlow {

enum class Step : int {
  Notes = 0,
  ReviewsPublic,
  Chapters,
  BestMarks,
  Similar,
};
inline constexpr int kTotalSteps = 5;

// Localized display name for the step.
const char* stepName(int step);

// API endpoint for the step (e.g. "/book/bookmarklist"). Stable string literal.
const char* stepApiName(int step);

// Fill `body` + `filter` for the step's request. Caller spawns the fetch task
// itself — this helper only knows about the wire shape.
void buildRequest(int step, const std::string& bookId, JsonDocument& body, JsonDocument& filter);

// Parse the response and persist to SD via WeReadCacheStore. Returns false if
// the SD write itself failed (treated as fatal by callers).
bool parseAndSave(int step, const std::string& bookId, JsonDocument& resp);

// Write an empty (count = 0) cache file for the step. Used when the network
// step fails with a non-fatal error so the offline reader sees "no rows"
// rather than "not cached".
void saveEmpty(int step, const std::string& bookId);

// True if the error should abort the whole flow rather than skip-with-empty.
// Wi-Fi loss / missing key / skill upgrade are unrecoverable mid-flow.
// Everything else (HTTP errors, parse failures, server-side errcodes) can
// legitimately mean "this API has no data for this book".
bool isFatalErr(WeReadClient::Err err);

}  // namespace WeReadBookCacheFlow
