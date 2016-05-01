/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Description: -------------------
	blimp
	- "b limited array"
	- does not grow or shrink max container size
	- does not check out of bounds
	blist
	- can reallocate itself to expand
	- checks bounds on push

Functions:
	blimp_set(): reserve the memory
	blist_set(): reserve the memory
	.[]: get memory without need to cast
	.push():  increase length. returns pointer to new item
	foreach(): loop through contents
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#ifndef _INCLUDE_BLISTS_
#define _INCLUDE_BLISTS_

#ifdef _INCLUDE_BLIMP_TOO_
#define blimp(VAR_TYPE,VAR_NAME) struct { \
		VAR_TYPE* list; \
		uint32_t length; \
		uint32_t max_length; \
		inline VAR_TYPE& operator[](u4 index) const { \
			return list[index]; \
		} \
		inline VAR_TYPE* push() { \
    		length++; \
			return &list[length-1]; \
    	} \
	} VAR_NAME;

#define blimp_set(VAR_LOC, VAR_MEM, VAR_TYPE, VAR_MAX) VAR_LOC.length = 0; \
	VAR_LOC.max_length = VAR_MAX; \
	VAR_LOC.list = (VAR_TYPE*)alloc(VAR_MEM,sizeof(VAR_TYPE) * VAR_MAX);
#endif
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void* BLIST_REALLOCATE_MORE_SPACE(void* mem_start, uint32_t element_size, uint32_t length, uint32_t &max_length, GameMemory* mem_arena)
{
	max_length = ((max_length-1) * 2) + 1;
	void* mem_new_start = alloc(*mem_arena, element_size * max_length);
	memcpy(mem_new_start,mem_start,element_size * length);
	return mem_new_start;
}	

#define blist(VAR_TYPE, VAR_NAME) struct { \
	union { \
		VAR_TYPE* list; \
		void* mem_start; \
	}; \
	uint32_t length; \
	uint32_t max_length; \
	uint32_t element_size; \
	GameMemory* mem_arena; \
	inline VAR_TYPE& operator[](u4 index) const \
    { \
        return list[index]; \
    } \
    inline VAR_TYPE* push() { \
    	length++; \
		if (length >= max_length) \
		{ \
			mem_start = BLIST_REALLOCATE_MORE_SPACE(mem_start, element_size, length, max_length, mem_arena); \
		} \
		return &list[length-1]; \
    } \
    inline VAR_TYPE* last() { \
    	if (length > 0) { \
    		return &list[length-1]; \
    	} else { \
    		return &list[0]; \
    	} \
    } \
} VAR_NAME;

#define blist_set(VAR_LOC, VAR_MEM, VAR_TYPE, VAR_MAX) VAR_LOC.length = 0; \
	VAR_LOC.max_length = VAR_MAX + 1; \
	VAR_LOC.mem_start = alloc(VAR_MEM,sizeof(VAR_TYPE) * VAR_MAX); \
	VAR_LOC.mem_arena = &VAR_MEM; \
	VAR_LOC.element_size = sizeof(VAR_TYPE);

#define foreach(VAR_LOC,VAR_INDEX) for (s4 VAR_INDEX = 0; VAR_INDEX < VAR_LOC.length; VAR_INDEX++)

#endif