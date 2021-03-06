#ifndef PREVIEW_LIBRARY_HPP
#define PREVIEW_LIBRARY_HPP

#include <memory>
#include <string>

#include "../resource/texture2d.h"

#include "../lib/sqlite/sqlite3.h"

#include "singleton.hpp"

class PreviewLibrary : public Singleton<PreviewLibrary> {
    sqlite3* _db;
    std::shared_ptr<Texture2D> no_preview_tex;
    std::shared_ptr<Texture2D> err_preview_tex;
    std::map<std::string, std::shared_ptr<Texture2D>> loaded_thumbs;

public:
    PreviewLibrary();
    ~PreviewLibrary();
    std::shared_ptr<Texture2D> getPreview(const std::string& res_path, int64_t timestamp);
    std::shared_ptr<Texture2D> getPreviewPlaceholder();

    void                       markForReload(const std::string& res_path);
};

#endif
