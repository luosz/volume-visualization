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
 * Filename: sysconf.h
 * 
 */

#ifndef _SYSCONF_H
#define _SYSCONF_H

#if defined(unix) || defined(UNIX)
#define DIR_SEP '/'
#else
#define DIR_SEP '\\'
#endif

#endif
