#pragma once

#include <iostream>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

#define SizeOfAttribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute);

struct row
{   
    uint32_t ID;
    char Username[COLUMN_USERNAME_SIZE+1];
    char Email[COLUMN_EMAIL_SIZE+1];

    void Print()
    {
      std::cout << this->ID << " " << this->Username << " " << this->Email << std::endl;     
    }
};

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
    memcpy(&Destination->Username, (uint8_t*)(Source) + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&Destination->Email, (uint8_t*)(Source) + EMAIL_OFFSET, EMAIL_SIZE);
}
