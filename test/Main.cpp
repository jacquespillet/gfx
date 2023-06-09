#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <assert.h>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

#define SizeOfAttribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute);


struct row
{   
    uint32_t ID;
    char Username[COLUMN_USERNAME_SIZE+1];
    char Email[COLUMN_EMAIL_SIZE+1];
};

enum class  metaCommandResult
{
    Success,
    Unrecognized
};

enum class prepareCommandResult
{
    Success,
    SyntaxError,
    StringTooLong,
    NegativeID,
    Unrecognized
};

enum class executeResult
{
    Success,
    TableFull
};

enum class statementType
{
    Insert,
    Select
};

struct statement
{
    statementType Type;
    row RowToInsert;
};


//Util
void PrintRow(row &Row)
{
    std::cout << Row.ID << " " << Row.Username << " " << Row.Email << std::endl;
}

std::vector<std::string> SplitString(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

//Compact representation of a row

const uint32_t ID_SIZE = SizeOfAttribute(row, ID);
const uint32_t USERNAME_SIZE = SizeOfAttribute(row, Username);
const uint32_t EMAIL_SIZE = SizeOfAttribute(row, Email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE ;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

void SerializeRow(row *Source, void *Destination)
{
    memcpy((uint8_t*)(Destination) + ID_OFFSET, &Source->ID, ID_SIZE);
    memcpy((uint8_t*)(Destination) + USERNAME_OFFSET, &Source->Username, USERNAME_SIZE);
    memcpy((uint8_t*)(Destination) + EMAIL_OFFSET, &Source->Email, EMAIL_SIZE);
}

void DeserializeRow(void *Source, row *Destination)
{
    memcpy(&Destination->ID, (uint8_t*)(Source) + ID_OFFSET, ID_SIZE);
    memcpy(&Destination->Username, (uint8_t*)(Source) + USERNAME_OFFSET, COLUMN_USERNAME_SIZE+1);
    memcpy(&Destination->Email, (uint8_t*)(Source) + EMAIL_OFFSET, COLUMN_EMAIL_SIZE+1);
}




const uint32_t PAGE_SIZE = 4096;
#define TABLE_MAX_PAGES 100
const uint32_t ROWS_PER_PAGES = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGES * TABLE_MAX_PAGES;

//Pager
struct pager
{
    std::ifstream FileDescriptorRead;
    std::ofstream FileDescriptorWrite;
    size_t FileLength;
    void *Pages[TABLE_MAX_PAGES];

    pager(std::string FileName)
    {
        FileDescriptorRead.open(FileName,std::fstream::in | std::fstream::binary);
        if (!FileDescriptorRead.is_open())
        {
            FileDescriptorWrite.open(FileName, std::ofstream::out | std::ofstream::binary);
            FileDescriptorWrite.close();
            
            FileDescriptorRead.open(FileName,std::fstream::in | std::fstream::binary);
        }

        if (!FileDescriptorRead.is_open())
        {
            std::cout << "Error opening DB file" << std::endl;
            assert(false);
            exit(1);
        }
        
        FileDescriptorRead.seekg(0, std::ios_base::end);
        FileLength = FileDescriptorRead.tellg();
        if(FileLength == SIZE_MAX) FileLength=0;
        
        FileDescriptorRead.seekg(0, std::ios_base::beg);

        for (size_t i = 0; i < TABLE_MAX_PAGES; i++)
        {
            Pages[i] = nullptr;
        }
    }

    void *GetPage(uint32_t PageNum)
    {
        if(PageNum > TABLE_MAX_PAGES)
        {
            std::cout << "Tried to access page number  out of bounds " << std::endl;
            assert(false);
            exit(1);
        }


        if(Pages[PageNum] == nullptr)
        {
            //Cache miss : We don't have the page loaded.

            //Allocate memory for the page
            void *Page = malloc(PAGE_SIZE);
            uint32_t NumPages = FileLength / PAGE_SIZE;
            
            //There might be a partial page at the end of the file
            size_t Remainder = FileLength % PAGE_SIZE;
            if(Remainder)
            {
                NumPages++;
            }

            //If the page to fetch is within the existing pages
            if(PageNum <= NumPages )
            {
                //We read its content into the page
                FileDescriptorRead.seekg(PageNum * PAGE_SIZE, std::ios_base::beg);
                FileDescriptorRead.read((char*)Page, PAGE_SIZE);
                if(FileDescriptorRead.gcount() == -1)
                {
                    std::cout << "Error reading file " << std::endl;
                    assert(false);
                    exit(1);
                }
            }

            //Save it in the cache
            Pages[PageNum] = Page;
        }

        //Return it
        return Pages[PageNum];
    }

    void Flush(uint32_t PageNum, uint32_t Size)
    {
        if(Pages[PageNum] == nullptr)
        {
            std::cout <<"Tried to flush empty page " << std::endl;
            assert(false);
            exit(1);
        }

        if(!FileDescriptorWrite.good())
        {
            std::cout << "Error seeking " << std::endl;
            assert(false);
            exit(1);
        }

        FileDescriptorWrite.write((char*)Pages[PageNum], Size);

        if(!FileDescriptorWrite.good())
        {
           std::cout << "Error writing to file" << std::endl;
           assert(false);
           exit(1);
        }
    }
};

//Table
struct table
{
    uint32_t NumRows;
    pager *Pager;
    std::string FileName;
    table(std::string FileName)
    {
        this->FileName = FileName;
        this->Pager = new pager(FileName);
        this->NumRows= Pager->FileLength / ROW_SIZE;
    }

    void Close()
    {
        Pager->FileDescriptorRead.close();

        Pager->FileDescriptorWrite.open(FileName,std::fstream::out | std::fstream::trunc | std::fstream::binary);
        if (!Pager->FileDescriptorWrite.is_open())
        {
            std::cout << "Error opening the file " << std::endl;
            assert(false);
            exit(1);
        }
        Pager->FileDescriptorWrite.seekp(0, std::ios_base::beg);
        
        size_t position = Pager->FileDescriptorWrite.tellp();
        std::cout << position << std::endl;
        
        uint32_t NumFullPages = NumRows / ROWS_PER_PAGES;

        for(uint32_t i=0; i<NumFullPages; i++)
        {
            if(Pager->Pages[i] == nullptr)
            {
                continue;
            }
            Pager->Flush(i, PAGE_SIZE);
            size_t position = Pager->FileDescriptorWrite.tellp();
            std::cout << position << " " << PAGE_SIZE << std::endl;
            free(Pager->Pages[i]);
            Pager->Pages[i] = nullptr;
        }


        uint32_t NumAdditionalRows = NumRows % ROWS_PER_PAGES;
        if(NumAdditionalRows>0)
        {
            uint32_t PageNum = NumFullPages;
            if(Pager->Pages[PageNum] != nullptr)
            {
                Pager->Flush(PageNum, NumAdditionalRows * ROW_SIZE);
                size_t position = Pager->FileDescriptorWrite.tellp();
                std::cout << position << " " << NumAdditionalRows * ROW_SIZE << std::endl;

                free(Pager->Pages[PageNum]);
                Pager->Pages[PageNum] = nullptr;
            }
        }

        Pager->FileDescriptorWrite.close();

        for(uint32_t i=0; i<TABLE_MAX_PAGES; i++)
        {
            void *Page = Pager->Pages[i];
            if(Page)
            {
                free(Page);
                Pager->Pages[i] = nullptr;

            }
        }

        delete this->Pager;
    }
};


struct cursor 
{
    table *Table;
    uint32_t RowNum;
    bool EndOfTable;

    void Advance()
    {
        this->RowNum++;
        if(RowNum >= Table->NumRows)
        {
            this->EndOfTable=true;
        }
    }
};

cursor *StartTable(table *Table)
{
    cursor *Cursor = new cursor();
    Cursor->Table = Table;
    Cursor->RowNum = 0;
    Cursor->EndOfTable=false;
    return Cursor;
}

cursor *EndTable(table *Table)
{
    cursor *Cursor = new cursor();
    Cursor->Table = Table;
    Cursor->RowNum = Table->NumRows;
    Cursor->EndOfTable=true;
    return Cursor;
}

void *CursorValue(cursor *Cursor)
{
    uint32_t RowNum = Cursor->RowNum;
    //Which page is the row in ?
    uint32_t PageNum = RowNum / ROWS_PER_PAGES;
    

    void *Page = Cursor->Table->Pager->GetPage(PageNum);

    //What's the row position inside the page (in terms of rows, not bytes)
    uint32_t RowOffset = RowNum % ROWS_PER_PAGES;
    //Byte position of the row
    uint32_t ByteOffset = RowOffset * ROW_SIZE;


    return (void*)((uint8_t*)Page + ByteOffset);
}

//Commands
metaCommandResult ExecuteMetaCommand(std::string Command, table *Table)
{
    if(Command.compare(".exit") == 0)
    {
        Table->Close();
        exit(0);
    }
    else
    {
        return metaCommandResult::Unrecognized;
    }
}

prepareCommandResult PrepareInsert(std::string Command, statement *Statement)
{
    Statement->Type = statementType::Insert;
    
    std::vector<std::string> Arguments = SplitString(Command, " ");
    if(Arguments.size() != 4)
    {
        return prepareCommandResult::SyntaxError;
    }

    try
    {
        if(std::stoi(Arguments[1]) < 0)
        {
            return prepareCommandResult::NegativeID;
        }
    }
    catch(const std::exception& e)
    {
        return prepareCommandResult::SyntaxError;
    }
    
    if(Arguments[2].size() > COLUMN_USERNAME_SIZE)
    {
        return prepareCommandResult::StringTooLong;
    }
    if(Arguments[3].size() > COLUMN_EMAIL_SIZE)
    {
        return prepareCommandResult::StringTooLong;
    }
    
    Statement->RowToInsert.ID = std::stoi(Arguments[1]);
    strcpy(Statement->RowToInsert.Username, Arguments[2].c_str());
    strcpy(Statement->RowToInsert.Email, Arguments[3].c_str());

    return prepareCommandResult::Success;
}

prepareCommandResult PrepareStatement(std::string Command, statement *Statement)
{
    if(Command.compare(0, 6, "insert") == 0)
    {
        return PrepareInsert(Command, Statement);
    }
    if(Command.compare("select") == 0)
    {
        Statement->Type = statementType::Select;
        return prepareCommandResult::Success;
    }

    return prepareCommandResult::Unrecognized;
}

executeResult ExecuteInsert(statement *Statement, table *Table)
{
    if(Table->NumRows >= TABLE_MAX_ROWS)
    {
        return executeResult::TableFull;
    }

    row *RowToInsert = &Statement->RowToInsert;
    cursor *Cursor = EndTable(Table);
    SerializeRow(RowToInsert, CursorValue(Cursor));
    Table->NumRows++;

    return executeResult::Success;
}

executeResult ExecuteSelect(statement *Statement, table *Table)
{
    cursor *Cursor = StartTable(Table);
    row Row;
    while(!Cursor->EndOfTable)
    {
        DeserializeRow(CursorValue(Cursor), &Row);
        PrintRow(Row);
        Cursor->Advance();
    }

    delete Cursor;

    return executeResult::Success;
}

executeResult ExecuteStatement(statement *Statement, table *Table)
{
    switch (Statement->Type)
    {
    case statementType::Insert:
        return ExecuteInsert(Statement, Table);
    case statementType::Select:
        return ExecuteSelect(Statement, Table);    
    default:
        break;
    }
}

int main()
{
#if 0
    std::ifstream stream;
    stream.open("./Database", std::fstream::in | std::fstream::binary);
    while (!stream.eof())
    {
        row Row;
        stream.read((char*)&Row.ID, ID_SIZE);
        stream.read((char*)&Row.Username, USERNAME_SIZE);
        stream.read((char*)&Row.Email, EMAIL_SIZE);
        std::cout << Row.ID << " " << Row.Username << " " << Row.Email << std::endl;
    }
#else
    table Table("./Database");
    while(true)
    {
        std::cout << "db > ";        
        
        std::string Input;
        std::getline(std::cin, Input);
        
        if(Input[0] == '.')
        {
            if(ExecuteMetaCommand(Input, &Table) == metaCommandResult::Success)
            {

            }
            else
            {
                std::cout << "Unrecognized meta command : " << Input << std::endl;
            }
        }
        else
        {
            statement Statement;
            prepareCommandResult Result = PrepareStatement(Input, &Statement);
            switch (Result)
            {
            case prepareCommandResult::Success :
                break;
            case prepareCommandResult::SyntaxError:
                std::cout << "Could not parse statement : " << Input << std::endl;
                continue;
            case prepareCommandResult::StringTooLong:
                std::cout << "String too long : " << Input << std::endl;
                continue;
            case prepareCommandResult::NegativeID:
                std::cout << "Negative ID : " << Input << std::endl;
                continue;
            case prepareCommandResult::Unrecognized :
                std::cout << "Unrecognized keyword at start of  command : " << Input << std::endl;
                continue;
            } 

            executeResult ExecuteResult = ExecuteStatement(&Statement, &Table);
            switch (ExecuteResult)
            {   
            case executeResult::Success :
                std::cout << "Execute Success "<< std::endl;
                break;
            case executeResult::TableFull :
                std::cout << "Table is full "<< std::endl;
                break;
            }
        }
    }
#endif
}