#ifndef INDEX_STRINGTABLE_HPP_INCLUDED
#define INDEX_STRINGTABLE_HPP_INCLUDED

#include <cstdint>
#include <string>
#include <memory>

namespace utymap { namespace index {

// Provides the way to maintain strings.
// Index file consists of id-offset pairs where id - string id,
// offset - first character of the string inside data file.
// data file contains list of null terminated strings.
class StringTable
{
public:

    // Creates instance of StringTable using file path provided.
    StringTable(const std::string& path);

    StringTable(const StringTable&) = delete;
    StringTable& operator=(StringTable const&) = delete;

    ~StringTable();

    // Gets id of given string.
    uint32_t getId(const std::string& str);

    // Gets original string by id.
    std::string getString(uint32_t id);

    // Flushes changes to disk.
    void flush();

private:
    class StringTableImpl;
    std::unique_ptr<StringTableImpl> pimpl_;
};

}}
#endif  // INDEX_STRINGTABLE_HPP_INCLUDED
