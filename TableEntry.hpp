/*

ClipSeekr 1.0
Copyright (C) 2011 by Aetilius, Inc.
All rights reserved.

*/
#include <cstdint>

#ifndef _TABLE_ENTRY_H
#define _TABLE_ENTRY_H

#define ID_LENGTH 32

using namespace std;

struct ImgHash {
    uint64_t hashc;
};

typedef struct clip_table_entry_t {
  string id;
  uint32_t idnum;
  uint32_t seqnum;
  uint32_t total;
  clip_table_entry_t():id(""),idnum(0),seqnum(0),total(0){};
} ClipTBLEntry;

typedef struct tmp_clip_table_entry_t {
  uint64_t seqnum;   // frame index
  uint64_t touched;  // frame index last time accessed
  int count;         // number times recurrence
  tmp_clip_table_entry_t():seqnum(0),touched(0),count(0){};
} TmpClipTBLEntry;

#endif /* _TABLE_ENTRY_H */ 
