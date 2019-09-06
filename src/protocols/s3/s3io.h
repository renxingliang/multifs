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


#pragma once

#include <stdint.h>
#include <vector>
#include <string>

#include "../../ifile.h"
#include "s3fs.h"

class S3Io :
	public IFile
{
public:
	S3Io();
	~S3Io();

	virtual int open(mode_t mode, char filepath[PATH_MAX]);
	virtual int close();
	virtual int remove(char filepath[PATH_MAX]);
	virtual int flush();
	virtual int getstat(struct stat* stbuf);
	virtual int truncate(off_t size);
	virtual int read(char* buf, size_t size, off_t offset);
	virtual int write(const char* buf, size_t size, off_t offset);
	virtual int config(size_t single_cache_size_n, char *cachepath, char *debug_mark);

private:
	std::vector<std::string> split_path(std::string path);
	std::string get_current_name();

private:
	fuse_file_info file_info;	// opened file infor
	char file_path[PATH_MAX];	// saved full file path
	std::string object_name;	// saved object name
	fuse_conn_info conn_info;	//
	size_t single_cache_size_m;	// single cache size
	std::string cachepath;		// cache path
	std::string debug_mark;		// info or debug or trace
};