#pragma once
#include <vector>
#include <string>

#include "Row.h"
#include "Util.h"
#include "Table.h"

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


//Commands
metaCommandResult ExecuteMetaCommand(std::string Command)
{
    if(Command.compare(".exit") == 0)
    {
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
    SerializeRow(RowToInsert, RowSlot(Table, Table->NumRows));
    Table->NumRows++;

    return executeResult::Success;
}

executeResult ExecuteSelect(statement *Statement, table *Table)
{
    row Row;
    for(uint32_t i=0; i<Table->NumRows; i++)
    {
        DeserializeRow(RowSlot(Table, i), &Row);
        Row.Print();
    }

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