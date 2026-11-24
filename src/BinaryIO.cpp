#include "BinaryIO.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <system_error>

namespace ReliefOps::io {

namespace {

constexpr std::uint32_t kMaxStringLength = 4096;

}  // namespace

void writeString(std::ostream& out, const std::string& text)
{
    if (text.size() > kMaxStringLength) {
        throw PersistenceException("string field too long (" + std::to_string(text.size()) + " bytes)");
    }
    writePod(out, static_cast<std::uint32_t>(text.size()));
    out.write(text.data(), static_cast<std::streamsize>(text.size()));
}

std::string readString(std::istream& in)
{
    const auto length = readPod<std::uint32_t>(in);
    if (length > kMaxStringLength) {
        throw PersistenceException("string field length " + std::to_string(length) + " is not plausible");
    }
    std::string text(length, '\0');
    if (length > 0 && !in.read(text.data(), length)) {
        throw PersistenceException("string field truncated");
    }
    return text;
}

std::uint32_t checksum(const std::string& bytes)
{
    std::uint32_t hash = 2166136261u;
    for (unsigned char c : bytes) {
        hash ^= c;
        hash *= 16777619u;
    }
    return hash;
}

void writeArchive(const std::string& path, std::uint32_t magic, std::uint16_t version,
                  const std::string& payload)
{
    const std::string tempPath = path + ".tmp";
    {
        std::ofstream out(tempPath, std::ios::binary | std::ios::trunc);
        if (!out) {
            throw PersistenceException("cannot open " + tempPath + " for writing");
        }
        writePod(out, magic);
        writePod(out, version);
        writePod(out, static_cast<std::uint64_t>(payload.size()));
        writePod(out, checksum(payload));
        out.write(payload.data(), static_cast<std::streamsize>(payload.size()));
        out.flush();
        if (!out) {
            std::error_code ignored;
            std::filesystem::remove(tempPath, ignored);
            throw PersistenceException("write to " + tempPath + " failed");
        }
    }

    std::error_code ec;
    std::filesystem::rename(tempPath, path, ec);
    if (ec) {
        std::error_code ignored;
        std::filesystem::remove(tempPath, ignored);
        throw PersistenceException("could not replace " + path + ": " + ec.message());
    }
}

std::string readArchive(const std::string& path, std::uint32_t magic, std::uint16_t version)
{
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw PersistenceException("cannot open " + path);
    }

    if (readPod<std::uint32_t>(in) != magic) {
        throw PersistenceException(path + " is not the expected archive type");
    }
    const auto fileVersion = readPod<std::uint16_t>(in);
    if (fileVersion != version) {
        throw PersistenceException(path + " has unsupported version " + std::to_string(fileVersion));
    }
    const auto size = readPod<std::uint64_t>(in);
    const auto expected = readPod<std::uint32_t>(in);

    std::string payload((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (payload.size() != size) {
        throw PersistenceException(path + " payload size mismatch");
    }
    if (checksum(payload) != expected) {
        throw PersistenceException(path + " failed checksum verification");
    }
    return payload;
}

}  // namespace ReliefOps::io
