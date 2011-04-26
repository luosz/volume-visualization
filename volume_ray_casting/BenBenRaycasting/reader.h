/*
 * Copyright (c) 2005  Institute for Visualization and Interactive
 * Systems, University of Stuttgart, Germany
 *
 * This source code is distributed as part of the single-pass volume
 * rendering project. Details about this project can be found on the
 * project web page at http://www.vis.uni-stuttgart.de/eng/research/
 * fields/current/spvolren. This file may be distributed, modified,
 * and used free of charge as long as this copyright notice is
 * included in its original form. Commercial use is strictly
 * prohibited.
 *
 * Filename: reader.h
 * 
 */

#ifndef _READER_H
#define _READER_H

typedef enum {DATRAW_UCHAR, DATRAW_FLOAT, DATRAW_USHORT} DataType;

void readData(char *filename, int *sizes, float *dists, void **data, 
			  DataType *type, int *numComponents);

int getDataTypeSize(DataType t);

#endif
