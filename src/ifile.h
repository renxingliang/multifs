// Copyright 2019 MesaTEE Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


/**
* File: ifile.h
* Description:
*     defined the base of file option
*/

#pragma once

#include "multifs_proto.h"

class IFile{
public:
	// open the file
	// [Note]
	// filepath type: protocl://ak:sk@domain/bucket/[dir]/object
	virtual int open(mode_t mode, char filepath[PATH_MAX]) = 0;

	// close the file witch you openned
	virtual int close() = 0;

	// remove the target file
	virtual int remove(char filepath[PATH_MAX]) = 0;

	// flush file cache, this operate will push the cache data to net
	virtual int flush() = 0;

	// Get file attributes
	virtual int getstat(struct stat* stbuf) = 0;

	// shrink or extend the size of a file to the specified size
	virtual int truncate(off_t size) = 0;

	// read from a file descriptor
	virtual int read(char* buf, size_t size, off_t offset) = 0;

	// write data to the file witch you have openned
	virtual int write(const char* buf, size_t size, off_t offset) = 0;

	virtual int config(size_t single_cache_size_n, char *cachepath, char *debug_mark) = 0;
};




