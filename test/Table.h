const uint32_t PAGE_SIZE = 4096;
#define TABLE_MAX_PAGES 100
const uint32_t ROWS_PER_PAGES = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGES * TABLE_MAX_PAGES;

struct table
{
    uint32_t NumRows;
    void* Pages[TABLE_MAX_PAGES];

    table()
    {
        NumRows=0;
        for(uint32_t i=0; i<TABLE_MAX_PAGES; i++)
        {
            Pages[i] = nullptr;
        }
    }

    ~table()
    {
        for(uint32_t i=0; i<TABLE_MAX_PAGES; i++)
        {
            free(Pages[i]);
        }
    }
};

void *RowSlot(table *Table, uint32_t RowNum)
{
    //Which page is the row in ?
    uint32_t PageNum = RowNum / ROWS_PER_PAGES;
    void *Page = Table->Pages[PageNum];
    //If the page is null, allocate it
    if(Page == nullptr)
    {
        Page = Table->Pages[PageNum] = malloc(PAGE_SIZE);
    }
    
    //What's the row position inside the page (in terms of rows, not bytes)
    uint32_t RowOffset = RowNum % ROWS_PER_PAGES;
    //Byte position of the row
    uint32_t ByteOffset = RowOffset * ROW_SIZE;


    return (void*)((uint8_t*)Page + ByteOffset);
}
