#ifndef BCD_DEBUG_H
#define BCD_DEBUG_H
// Debuging
#ifdef CONFIG_DEBUG_STACK 
static const char TAG_STACK[] = CONFIG_TAG_STACK;
#endif //CONFIG_DEBUG_STACK

#ifdef CONFIG_DEBUG_HEAP
#include "esp_heap_trace.h"
#define NUM_RECORDS 100
static heap_trace_record_t trace_record[NUM_RECORDS]; // This buffer must be in internal RAM
#endif //CONFIG_DEBUG_HEAP
#endif //BCD_DEBUG_H