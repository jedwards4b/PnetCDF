/*
 *  Copyright (C) 2013, Northwestern University and Argonne National Laboratory
 *  See COPYRIGHT notice in top-level directory.
 */
/* $Id$ */

/* This program tests if PnetCDF can detect CDF-5 data types that are not
 * available in CDF-1 and CDF-2 formats
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <pnetcdf.h>

#define ERR {if(err!=NC_NOERR) {printf("Error(%d) at line %d: %s\n",err,__LINE__,ncmpi_strerror(err)); nerr++; }}

/*----< test_attr_types() >---------------------------------------------------*/
static
int test_attr_types(char *filename,
                    int   format)
{
    int i, err, rank, ncid, cmode, nerr=0, attr=0;
    MPI_Info info=MPI_INFO_NULL;
    MPI_Comm comm=MPI_COMM_WORLD;
    nc_type xtype[5]={NC_UBYTE, NC_USHORT, NC_UINT, NC_INT64, NC_UINT64};

    MPI_Comm_rank(comm, &rank);
    cmode = NC_CLOBBER|format;

    /* create a file in CDF-1 or CDF-2 format */
    err = ncmpi_create(comm, filename, cmode, info, &ncid); ERR
    for (i=0; i<5; i++) {
        char name[32];
        sprintf(name, "gattr_%d", i);
        err = ncmpi_put_att_int(ncid, NC_GLOBAL, name, xtype[i], 1, &attr);
        if (err != NC_ESTRICTCDF2) {
            printf("Error (line=%d): expecting NC_ESTRICTCDF2 but got %d\n", __LINE__,err);
            nerr++;
        }
    }

    err = ncmpi_close(ncid); ERR

    return nerr;
}

/*----< test_var_types() >----------------------------------------------------*/
static
int test_var_types(char *filename,
                   int   format)
{
    int i, err, rank, ncid, cmode, nerr=0;
    int dimid, varid[5];
    MPI_Info info=MPI_INFO_NULL;
    MPI_Comm comm=MPI_COMM_WORLD;
    nc_type xtype[5]={NC_UBYTE, NC_USHORT, NC_UINT, NC_INT64, NC_UINT64};

    MPI_Comm_rank(comm, &rank);
    cmode = NC_CLOBBER|format;

    /* create a file in CDF-1 or CDF-2 format */
    err = ncmpi_create(comm, filename, cmode, info, &ncid); ERR
    err = ncmpi_def_dim(ncid, "dim", NC_UNLIMITED, &dimid); ERR
    for (i=0; i<5; i++) {
        char name[32];
        sprintf(name, "var_%d", i);
        err = ncmpi_def_var(ncid, name, xtype[i], 1, &dimid, &varid[i]);
        if (err != NC_ESTRICTCDF2) {
            printf("Error (line=%d): expecting NC_ESTRICTCDF2 but got %d\n", __LINE__,err);
            nerr++;
        }
    }
    err = ncmpi_close(ncid); ERR

    return nerr;
}

/*----< main() >--------------------------------------------------------------*/
int main(int argc, char **argv)
{
    char *filename="testfile.nc";
    int rank, nerr=0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc > 2) {
        if (!rank) printf("Usage: %s [filename]\n",argv[0]);
        MPI_Finalize();
        return 0;
    }
    if (argc == 2) filename = argv[1];

    if (rank > 0) {
        MPI_Finalize();
        return 0;
    }

    nerr += test_attr_types(filename, 0);
    nerr += test_attr_types(filename, NC_64BIT_OFFSET);

    nerr += test_var_types(filename, 0);
    nerr += test_var_types(filename, NC_64BIT_OFFSET);

    MPI_Offset malloc_size, sum_size;
    int err = ncmpi_inq_malloc_size(&malloc_size);
    if (err == NC_NOERR) {
        MPI_Reduce(&malloc_size, &sum_size, 1, MPI_OFFSET, MPI_SUM, 0, MPI_COMM_WORLD);
        if (rank == 0 && sum_size > 0)
            printf("heap memory allocated by PnetCDF internally has %lld bytes yet to be freed\n",
                   sum_size);
    }

    char cmd_str[80];
    sprintf(cmd_str, "*** TESTING C   %s for CDF-5 type in CDF-1 and 2 ", argv[0]);
    if (nerr == 0)
        printf("%-66s ------ pass\n", cmd_str);
    else
        printf("%-66s ------ failed\n", cmd_str);

    MPI_Finalize();
    return 0;
}

