#include "Include/Util.h"

namespace hlgfx
{
void AddItem(std::vector<u8> &Blob, void *Item, sz Size)
{
    sz Cursor = Blob.size();
    Blob.resize(Cursor + Size);
    memcpy(Blob.data() + Cursor, Item, Size);
}
}