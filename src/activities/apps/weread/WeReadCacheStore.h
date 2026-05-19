#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "WeReadModels.h"

/**
 * Per-book offline cache for the WeRead companion.
 *
 * Each cached book lives in /.crosspoint/weread/<bookId>/, with one binary
 * file per API result. The shelf list itself lives at /.crosspoint/weread/shelf.bin.
 *
 * Binary layout, common to every file:
 *   [magic: 4 bytes]              e.g. "WRSF" shelf, "WRNT" notes, "WRMT" meta
 *   [version: uint16]             schema version — mismatched files are rejected
 *   [count:  uint32]              (omitted for meta.bin, which is a single record)
 *   [rows…]                       fields in the same order as WeReadModels.h
 *
 * Strings use the length-prefixed format from lib/Serialization (uint32 + bytes).
 * Numeric fields go through writePod<T>. A schema change MUST bump the version
 * for that file so old caches are silently dropped (no crash, no migration).
 *
 * All I/O goes through HalStorage (`Storage` singleton — thread-safe via mutex).
 * Failures return false and log via LOG_ERR; partial writes leave a stale file
 * on disk, which the next overwrite cycle replaces.
 */
namespace WeReadCacheStore {

constexpr const char* kRoot = "/.crosspoint/weread";

// ---- Shelf -----------------------------------------------------------------

bool saveShelf(const std::vector<WeReadModels::BookCard>& books);
bool loadShelf(std::vector<WeReadModels::BookCard>& outBooks);

// ---- Per-book --------------------------------------------------------------

bool saveBookMeta(const std::string& bookId, const WeReadModels::BookCard& card);
bool loadBookMeta(const std::string& bookId, WeReadModels::BookCard& outCard);

bool saveNotes(const std::string& bookId, const std::vector<WeReadModels::BookmarkRow>& rows);
bool loadNotes(const std::string& bookId, std::vector<WeReadModels::BookmarkRow>& outRows);

bool savePublicReviews(const std::string& bookId, const std::vector<WeReadModels::PublicReviewRow>& rows);
bool loadPublicReviews(const std::string& bookId, std::vector<WeReadModels::PublicReviewRow>& outRows);

bool saveChapters(const std::string& bookId, const std::vector<WeReadModels::ChapterRow>& rows);
bool loadChapters(const std::string& bookId, std::vector<WeReadModels::ChapterRow>& outRows);

bool saveBestMarks(const std::string& bookId, const std::vector<WeReadModels::BestMarkRow>& rows);
bool loadBestMarks(const std::string& bookId, std::vector<WeReadModels::BestMarkRow>& outRows);

bool saveSimilar(const std::string& bookId, const std::vector<WeReadModels::SearchRow>& rows);
bool loadSimilar(const std::string& bookId, std::vector<WeReadModels::SearchRow>& outRows);

// ---- Queries / management --------------------------------------------------

// True iff at least the notes.bin exists for this book — that's the file the
// cache flow writes last (well, second-to-last; similar.bin is last, but notes
// is what users care about). Using a single sentinel keeps the check cheap.
bool hasBookCached(const std::string& bookId);

// Delete the entire /.crosspoint/weread/<bookId>/ directory.
bool removeBookCache(const std::string& bookId);

}  // namespace WeReadCacheStore
