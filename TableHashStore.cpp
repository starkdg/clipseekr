#include "TableHashStore.hpp"


TableHashStore::TableHashStore(const string &tablefile, const string &tmptablefile){
    int error;
    this->ptable    = NULL;
    this->ptmptable = NULL;

    this->tablefile = tablefile;
    this->tmptablefile = tmptablefile;

    if (!tablefile.empty()){
	this->ptable = table_read(tablefile.c_str(), &error);
	if (this->ptable == NULL){
	    throw TableException("unable to read table file");
	}
    }

    if (!tmptablefile.empty()){
	this->ptmptable = table_read(tmptablefile.c_str(), &error);
	if (this->ptmptable == NULL){
	    this->ptmptable = table_alloc(1<<26, &error);
	    if (this->ptmptable == NULL){
		throw TableException("unable to allocate tmp table");
	    }
	}
    }
}

TableHashStore::~TableHashStore(){}
 

void TableHashStore::PutHashKeyValue(const struct ImgHash *pimghash, ClipTBLEntry *pentry, bool overwrite){
    int over = (overwrite) ? 1 : 0;
    if (ptable)
	table_insert_kd(ptable, (void*)pimghash, sizeof(struct ImgHash), (void*)pentry, sizeof(ClipTBLEntry), 0, 0, over);
}

void TableHashStore::GetHashValue(const struct ImgHash *pimghash, ClipTBLEntry **pentry){
    int entrySize;
    if (ptable)
	table_retrieve(ptable, (void*)pimghash, sizeof(struct ImgHash), (void**)pentry, &entrySize);
}

void TableHashStore::PutTmpHashKeyValue(const struct ImgHash *pimghash, TmpClipTBLEntry *pentry, bool overwrite){
    int over = (overwrite) ? 1 : 0;
    if (ptmptable)
	table_insert_kd(ptmptable, (void*)pimghash, sizeof(struct ImgHash), (void*)pentry, sizeof(TmpClipTBLEntry), 0, 0, over);
}

void TableHashStore::GetTmpHashValue(const struct ImgHash *pimghash, TmpClipTBLEntry **pentry){
    int entrySize;
    if (ptmptable)
	table_retrieve(ptmptable, (void*)pimghash, sizeof(struct ImgHash), (void**)pentry, &entrySize);
}

void TableHashStore::DeleteTmpHashValue(const struct ImgHash *pimghash){

    if (ptmptable)
	table_delete(ptmptable, pimghash, sizeof(struct ImgHash), NULL, NULL);

}

static const string countKey = "NumberEntriesKey";

int64_t TableHashStore::GetCount() {
	int64_t *num = NULL;
       	int size_num;
        int error = table_retrieve(ptable, countKey.c_str(), 
        countKey.length(), (void **)&num, &size_num);
	if(error != TABLE_ERROR_NONE && num == NULL 
		&& size_num != sizeof(int64_t))
		return -1;

	return *num;
}

int TableHashStore::SetCount(int64_t count) {
    int error = table_insert_kd(ptable, countKey.c_str(), countKey.length(),
                            &count, sizeof(count), 0, 0, 1);	

	if (error != TABLE_ERROR_NONE)
		return -1;

	return 0;
}

void TableHashStore::Shutdown(){
    int error = TABLE_ERROR_NONE;
    if (!tmptablefile.empty() && ptmptable != NULL){
	error = table_write(ptmptable, tmptablefile.c_str(), 00755);
    }
    if (ptable != NULL)    table_free(ptable);
    if (ptmptable != NULL) table_free(ptmptable);
    if (error != TABLE_ERROR_NONE) throw TableException("unable to write tmp table");
}

