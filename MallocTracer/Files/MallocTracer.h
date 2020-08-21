#ifndef __MALLOC_TRANCER__
#define __MALLOC_TRANCER__

#define trace_malloc(size) _trace_malloc(size, __FILE__, __FUNCTION__, __LINE__)
#define trace_free(ptr) _trace_free(ptr, __FILE__, __FUNCTION__, __LINE__)

struct MallocTrancer {
   char * (*getMallocInfo)(void);
};

struct MallocTrancer * New_MallocTrancer(void);
void * _trace_malloc(size_t size, const char *file, const char *func,const long line);
void _trace_free(void * ptr, const char *file, const char *func,const long line);

#endif  /* __MALLOC_TRANCER__ */