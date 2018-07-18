/*
// typedef struct chunk {
  // u32 magic;
  // u8 recovered;		// TODO: make union for flags recovered and sync_state
  // u8 sync_state;		// flag: locked for RDMA operation ? saved
  // dma_addr_t paddr;
  // size_t size;
  // struct list_head list;
// } chunk_t;

enum {
	rmda_chunk_new = 0,
	rdma_chunk_locked,
	rmda_chunk_saved,
	rmda_chunk_deleted
}

//--------------------------------------------------------------------------------------------------

void *nvdimm_alloc(size_t size, int flags){
	//...
	chunk->sync_state = 0;
	//...
}

//--------------------------------------------------------------------------------------------------

void
sync(bool all) {
	
	if(all) {
		//rmda_clear_remote();
	}

	chunk_t *chunk, *tmp;
	struct list_head *head = nvdimm_get_list_head();
	nvdimm_list_mutex_lock();
	list_for_each_entry_safe(chunk, tmp, head, list) {
		if(all || chunk->sync_state == rmda_chunk_new) {
			chunk->sync_state = rdma_chunk_lock;
			rdma_chunk_send(chunk, chunk->size);
	}
	nvdimm_list_mutex_unlock();
}

//--------------------------------------------------------------------------------------------------

void
rdma_chunk_send(void *mem, size_t size) {
  ;
}

*/