#pragma once

#include <filesystem>
#include <string>
#include <system_error>
#include <fstream>

namespace fs = std::filesystem;

namespace MF::FilesManager {
    // Check if the given Path Exists (file or folder)
    static bool Exists(const std::string& Path) noexcept {
        std::error_code Ec;
        bool Result = fs::exists(fs::u8path(Path), Ec);
        return !Ec && Result;
    }

    // Check if the given Path is a Regular File
    static bool IsFile(const std::string& Path, std::string& OutError) noexcept {
        std::error_code Ec;
        bool Result = fs::is_regular_file(fs::u8path(Path), Ec);
        if (Ec) {
            OutError = Ec.message();
            return false;
        }
        return Result;
    }

    // Check if the given Path is a Directory
    static bool IsDirectory(const std::string& Path, std::string& OutError) noexcept {
        std::error_code Ec;
        bool Result = fs::is_directory(fs::u8path(Path), Ec);
        if (Ec) {
            OutError = Ec.message();
            return false;
        }
        return Result;
    }

    // Create directory (including intermediate directories)
    static bool CreateDirectory(const std::string& Path, std::string& OutError) noexcept {
        std::error_code Ec;
        bool Result = fs::create_directories(fs::u8path(Path), Ec);
        if (Ec) {
            OutError = Ec.message();
            return false;
        }
        return Result;
    }

    // Create file
    static bool CreateFile(const std::string& Path, std::string& OutError) noexcept {
        try {
            std::ofstream file(fs::u8path(Path));
            if (!file.is_open()) {
                OutError = "failed to create file: " + Path;
                return false;
            }
            return true;
        } catch (const std::exception& e) {
            OutError = e.what();
            return false;
        }
    }


    // Remove file/directory recursively
    static bool Remove(const std::string& Path, std::string& OutError) noexcept {
        std::error_code Ec;
        uintmax_t Count = fs::remove_all(fs::u8path(Path), Ec);
        if (Ec) {
            OutError = Ec.message();
            return false;
        }
        return Count > 0;
    }

    // Get File Size in bytes
    static bool GetFileSize(const std::string& Path, uintmax_t& OutSize, std::string& OutError) noexcept {
        std::error_code Ec;
        uintmax_t Size = fs::file_size(fs::u8path(Path), Ec);
        if (Ec) {
            OutError = Ec.message();
            return false;
        }
        OutSize = Size;
        return true;
    }

    // Rename File or Directory
    static bool Rename(const std::string& OldPath, const std::string& NewPath, std::string& OutError) noexcept {
        std::error_code Ec;
        fs::rename(fs::u8path(OldPath), fs::u8path(NewPath), Ec);
        if (Ec) {
            OutError = Ec.message();
            return false;
        }
        return true;
    }

    // Copy File or Directory (recursive)
    static bool Copy(const std::string& SrcPath, const std::string& DstPath, std::string& OutError) noexcept {
        std::error_code Ec;
        fs::copy_options Options = fs::copy_options::recursive | fs::copy_options::overwrite_existing;
        fs::copy(fs::u8path(SrcPath), fs::u8path(DstPath), Options, Ec);
        if (Ec) {
            OutError = Ec.message();
            return false;
        }
        return true;
    }
}