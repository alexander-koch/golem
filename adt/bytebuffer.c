#include "bytebuffer.h"

void bytebuffer_init(bytebuffer_t* buffer)
{
	buffer->data = 0;
	buffer->count = 0;
	buffer->capacity = 0;
}

void bytebuffer_clear(bytebuffer_t* buffer)
{
	free(buffer->data);
	bytebuffer_init(buffer);
}

void bytebuffer_fill(bytebuffer_t* buffer, uint8_t data, int count)
{
	if(buffer->capacity < buffer->count + count)
	{
		int capacity = buffer->capacity;
		while(capacity < buffer->count + count)
		{
			capacity = (capacity == 0) ? 8 : capacity * 2;
		}

		if(buffer->capacity == 0)
		{
			buffer->data = malloc(sizeof(uint8_t) * capacity);
		}
		else
		{
			buffer->data = realloc(buffer->data, sizeof(uint8_t) * capacity);
		}
		buffer->capacity = capacity;	
	}
	for(int i = 0; i < count; i++)
	{
		buffer->data[buffer->count++] = data;
	}
}

void bytebuffer_write(bytebuffer_t* buffer, uint8_t data)
{
	bytebuffer_fill(buffer, data, 1);
}
