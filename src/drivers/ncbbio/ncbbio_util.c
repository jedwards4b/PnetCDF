/*
 *  Copyright (C) 2017, Northwestern University and Argonne National Laboratory
 *  See COPYRIGHT notice in top-level directory.
 */
/* $Id$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ncbbio_driver.h>

/*
 * Extract mpi hints and set up the flags
 */
void ncbbio_extract_hint(NC_bb *ncbbp, MPI_Info info){
    int flag;
    char value[MPI_MAX_INFO_VAL];

    ncbbp->hints = NC_LOG_HINT_DEL_ON_CLOSE | NC_LOG_HINT_FLUSH_ON_READ |
                    NC_LOG_HINT_FLUSH_ON_SYNC;
    // Directory to place log files
    MPI_Info_get(info, "nc_burst_buf_dirname", MPI_MAX_INFO_VAL - 1,
                 value, &flag);
    if (flag) {
        strncpy(ncbbp->logbase, value, PATH_MAX);
    }
    else {
        memset(ncbbp->logbase, 0, sizeof(ncbbp->logbase));
    }
    // Overwrite the logfile is already exists (disable)
    MPI_Info_get(info, "nc_burst_buf_overwrite", MPI_MAX_INFO_VAL - 1,
                 value, &flag);
    if (flag && strcasecmp(value, "enable") == 0){
        ncbbp->hints |= NC_LOG_HINT_LOG_OVERWRITE;
    }

    /* Use shared logfiles among processes on the same compute node (default is
     * disabled). This feature depends on the availability of MPI constant
     * MPI_COMM_TYPE_SHARED, which is first defined in MPI standard version 3.0
     */
    MPI_Info_get(info, "nc_burst_buf_shared_logs", MPI_MAX_INFO_VAL - 1,
                 value, &flag);
    if (flag && strcasecmp(value, "enable") == 0){
        ncbbp->hints |= NC_LOG_HINT_LOG_SHARE;
    }

    // Delete the logfile after file closing (enable)
    MPI_Info_get(info, "nc_burst_buf_del_on_close", MPI_MAX_INFO_VAL - 1,
                 value, &flag);
    if (flag && strcasecmp(value, "disable") == 0){
        ncbbp->hints ^= NC_LOG_HINT_DEL_ON_CLOSE;
    }
    // Buffer size used to flush the log (0 (unlimited))
    MPI_Info_get(info, "nc_burst_buf_flush_buffer_size", MPI_MAX_INFO_VAL - 1,
                 value, &flag);
    if (flag){
        long int bsize = strtol(value, NULL, 0);
        if (bsize < 0) {
            bsize = 0;
        }
        ncbbp->flushbuffersize = (size_t)bsize; // Unit: byte
    }
    else{
        ncbbp->flushbuffersize = 0; // 0 means unlimited}
    }
}

/*
 * Export hint based on flag
 * NOTE: We only set up the hint if it is not the default setting
 *       user hint maching the default behavior will be ignored
 */
void ncbbio_export_hint(NC_bb *ncbbp, MPI_Info info){
    char value[MPI_MAX_INFO_VAL];

    MPI_Info_set(info, "nc_burst_buf", "enable");
    if (ncbbp->hints & NC_LOG_HINT_LOG_OVERWRITE) {
        MPI_Info_set(info, "nc_burst_buf_overwrite", "enable");
    }
    if (ncbbp->hints & NC_LOG_HINT_LOG_SHARE) {
        MPI_Info_set(info, "nc_burst_buf_shared_logs", "enable");
    }
    if (!(ncbbp->hints & NC_LOG_HINT_DEL_ON_CLOSE)) {
        MPI_Info_set(info, "nc_burst_buf_del_on_close", "disable");
    }
    if (ncbbp->logbase[0] != '\0') {
        MPI_Info_set(info, "nc_burst_buf_dirname", ncbbp->logbase);
    }
    if (ncbbp->flushbuffersize > 0) {
        sprintf(value, "%llu", ncbbp->flushbuffersize);
        MPI_Info_set(info, "nc_burst_buf_flush_buffer_size", value);
    }
}
