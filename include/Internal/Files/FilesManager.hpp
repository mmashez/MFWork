#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;

namespace MF::FilesManager {    

    // normalize path into absolute canonical form
    inline std::optional<std::string> NormalizePath(const std::string& Path) noexcept {
        std::error_code Ec;
        auto Canon = fs::weakly_canonical(fs::u8path(Path), Ec);
        if (Ec) return std::nullopt;
        return Canon.string();
    }

    // check if path exists
    inline bool Exists(const std::string& Path) noexcept {
        std::error_code Ec;
        return fs::exists(fs::u8path(Path), Ec) && !Ec;
    }

    // check if path is a file
    inline bool IsFile(const std::string& Path) noexcept {
        std::error_code Ec;
        return fs::is_regular_file(fs::u8path(Path), Ec) && !Ec;
    }

    // check if path is a directory
    inline bool IsDirectory(const std::string& Path) noexcept {
        std::error_code Ec;
        return fs::is_directory(fs::u8path(Path), Ec) && !Ec;
    }

    // create directory tree
    inline std::optional<std::string> CreateDirectory(const std::string& Path) noexcept {
        std::error_code Ec;
        fs::create_directories(fs::u8path(Path), Ec);
        if (Ec) return Ec.message();
        return std::nullopt;
    }

    // create empty file
    inline std::optional<std::string> CreateFile(const std::string& Path) noexcept {
        try {
            std::ofstream File(fs::u8path(Path));
            if (!File.is_open()) return "Failed to create file: " + Path;
            return std::nullopt;
        } catch (const std::exception& e) {
            return e.what();
        }
    }

    // remove recursively
    inline std::optional<std::string> Remove(const std::string& Path) noexcept {
        std::error_code Ec;
        fs::remove_all(fs::u8path(Path), Ec);
        if (Ec) return Ec.message();
        return std::nullopt;
    }

    // file size
    inline std::optional<uintmax_t> FileSize(const std::string& Path) noexcept {
        std::error_code Ec;
        auto Size = fs::file_size(fs::u8path(Path), Ec);
        if (Ec) return std::nullopt;
        return Size;
    }

    // rename/move
    inline std::optional<std::string> Rename(const std::string& OldPath, const std::string& NewPath) noexcept {
        std::error_code Ec;
        fs::rename(fs::u8path(OldPath), fs::u8path(NewPath), Ec);
        if (Ec) return Ec.message();
        return std::nullopt;
    }

    // copy (recursive, overwrite)
    inline std::optional<std::string> Copy(const std::string& SrcPath, const std::string& DstPath) noexcept {
        std::error_code Ec;
        fs::copy(fs::u8path(SrcPath), fs::u8path(DstPath),
                 fs::copy_options::recursive | fs::copy_options::overwrite_existing, Ec);
        if (Ec) return Ec.message();
        return std::nullopt;
    }

    // read entire file into string
    inline std::optional<std::string> ReadFileToString(const std::string& Path) noexcept {
        try {
            std::ifstream File(fs::u8path(Path), std::ios::binary);
            if (!File.is_open()) return std::nullopt;
            std::string Content((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());
            return Content;
        } catch (...) {
            return std::nullopt;
        }
    }

    // write string to file (overwrite)
    inline std::optional<std::string> WriteStringToFile(const std::string& Path, std::string_view Data) noexcept {
        try {
            std::ofstream File(fs::u8path(Path), std::ios::binary | std::ios::trunc);
            if (!File.is_open()) return "Failed to open file for writing: " + Path;
            File.write(Data.data(), static_cast<std::streamsize>(Data.size()));
            return std::nullopt;
        } catch (const std::exception& e) {
            return e.what();
        }
    }

    // list directory contents
    inline std::vector<std::string> ListDirectory(const std::string& Path, bool Recursive = false) noexcept {
        std::vector<std::string> Results;
        std::error_code Ec;

        if (Recursive) {
            for (auto& Entry : fs::recursive_directory_iterator(fs::u8path(Path), Ec)) {
                Results.push_back(Entry.path().string());
            }
        } else {
            for (auto& Entry : fs::directory_iterator(fs::u8path(Path), Ec)) {
                Results.push_back(Entry.path().string());
            }
        }
        return Results;
    }

    // temporary file (unique)
    inline std::optional<std::string> TempFile(const std::string& Prefix = "mfwork_tmp") noexcept {
        try {
            auto Tmp = fs::temp_directory_path() / fs::path(Prefix + "_XXXXXX");
            std::ofstream(Tmp.string()).close();
            return Tmp.string();
        } catch (...) {
            return std::nullopt;
        }
    }

    // temporary directory (unique)
    inline std::optional<std::string> TempDirectory(const std::string& Prefix = "mfwork_tmpdir") noexcept {
        try {
            auto Tmp = fs::temp_directory_path() / fs::path(Prefix + "_XXXXXX");
            fs::create_directories(Tmp);
            return Tmp.string();
        } catch (...) {
            return std::nullopt;
        }
    }

    // check permissions
    inline bool IsReadable(const std::string& Path) noexcept {
        std::ifstream File(fs::u8path(Path));
        return File.good();
    }
    inline bool IsWritable(const std::string& Path) noexcept {
        std::ofstream File(fs::u8path(Path), std::ios::app);
        return File.good();
    }
}
