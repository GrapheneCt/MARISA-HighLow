#include <psp2/kernel/clib.h>
#include <psp2/kernel/iofilemgr.h> 
#include <psp2/sysmodule.h>
#include <stdlib.h>

#include "psarc.h"

#define RAMCACHEBLOCKSIZE 64 * 1024

SceInt32 sceFiosInitialize(const SceFiosParams* params);
void sceFiosTerminate();

SceFiosSize sceFiosArchiveGetMountBufferSizeSync(const ScePVoid attr, const SceName path, ScePVoid params);
SceInt32 sceFiosArchiveMountSync(const ScePVoid attr, SceFiosFH* fh, const SceName path, const SceName mount_point, SceFiosBuffer mount_buffer, ScePVoid params);
SceInt32 sceFiosArchiveUnmountSync(const ScePVoid attr, SceFiosFH fh);

SceInt32 sceFiosIOFilterAdd(SceInt32 index, SceVoid(*callback)(), ScePVoid context);
SceInt32 sceFiosIOFilterRemove(SceInt32 index);

void sceFiosIOFilterPsarcDearchiver();
void sceFiosIOFilterCache();

static SceInt64 s_op_storage[SCE_FIOS_OP_STORAGE_SIZE(64, MAX_PATH_LENGTH) / sizeof(SceInt64) + 1];
static SceInt64 s_chunk_storage[SCE_FIOS_CHUNK_STORAGE_SIZE(1024) / sizeof(SceInt64) + 1];
static SceInt64 s_fh_storage[SCE_FIOS_FH_STORAGE_SIZE(32, MAX_PATH_LENGTH) / sizeof(SceInt64) + 1];
static SceInt64 s_dh_storage[SCE_FIOS_DH_STORAGE_SIZE(32, MAX_PATH_LENGTH) / sizeof(SceInt64) + 1];

static SceFiosPsarcDearchiverContext s_dearchiver_context = SCE_FIOS_PSARC_DEARCHIVER_CONTEXT_INITIALIZER;
static SceByte s_dearchiver_work_buffer[3 * 64 * 1024] __attribute__((aligned(64)));
static SceInt32 s_archive_index = 0;
static SceFiosBuffer s_mount_buffer = SCE_FIOS_BUFFER_INITIALIZER;
static SceFiosFH s_archive_fh = -1;

SceFiosRamCacheContext s_ramcache_context = SCE_FIOS_RAM_CACHE_CONTEXT_INITIALIZER;
static SceByte s_ramcache_work_buffer[10 * (64 * 1024)] __attribute__((aligned(8)));

/* memcpy replacement for initializers */

ScePVoid* memcpy(ScePVoid destination, const ScePVoid source, SceSize num)
{
	return sceClibMemcpy(destination, source, num);
}

SceInt32 psarcInit(void)
{
	SceFiosParams params = SCE_FIOS_PARAMS_INITIALIZER;
	params.opStorage.pPtr = s_op_storage;
	params.opStorage.length = sizeof(s_op_storage);
	params.chunkStorage.pPtr = s_chunk_storage;
	params.chunkStorage.length = sizeof(s_chunk_storage);
	params.fhStorage.pPtr = s_fh_storage;
	params.fhStorage.length = sizeof(s_fh_storage);
	params.dhStorage.pPtr = s_dh_storage;
	params.dhStorage.length = sizeof(s_dh_storage);
	params.pathMax = MAX_PATH_LENGTH;

	return sceFiosInitialize(&params);
}

SceVoid psarcOpenArchive(const SceName path, const SceName mountpoint)
{
	/* dearchiver overlay */

	s_dearchiver_context.workBufferSize = sizeof(s_dearchiver_work_buffer);
	s_dearchiver_context.pWorkBuffer = s_dearchiver_work_buffer;
	sceFiosIOFilterAdd(s_archive_index, sceFiosIOFilterPsarcDearchiver, &s_dearchiver_context);

	/* ramcache */

	s_ramcache_context.pPath = path;
	s_ramcache_context.workBufferSize = sizeof(s_ramcache_work_buffer);
	s_ramcache_context.pWorkBuffer = s_ramcache_work_buffer;
	s_ramcache_context.blockSize = RAMCACHEBLOCKSIZE;
	sceFiosIOFilterAdd(s_archive_index + 1, sceFiosIOFilterCache, &s_ramcache_context);

	SceFiosSize result = sceFiosArchiveGetMountBufferSizeSync(NULL, path, NULL);
	s_mount_buffer.length = (SceSize)result;
	s_mount_buffer.pPtr = malloc(s_mount_buffer.length);
	sceFiosArchiveMountSync(NULL, &s_archive_fh, path, mountpoint, s_mount_buffer, NULL);
}

SceVoid psarcFinish(SceVoid)
{
	sceFiosArchiveUnmountSync(NULL, s_archive_fh);
	free(s_mount_buffer.pPtr);
	sceFiosIOFilterRemove(s_archive_index);
	sceFiosIOFilterRemove(s_archive_index + 1);
	sceFiosTerminate();
}
