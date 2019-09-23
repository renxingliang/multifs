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
#include <string>

#include "../../ifile.h"
#include <stdint.h>

class FtpIo:
	public IFile
{
public:
	virtual int open(mode_t mode, char *filepath);
	virtual int close();
	virtual int remove(char *filepath);
	virtual int flush();
	virtual int getstat(char *filepath, struct stat* stbuf);
	virtual int truncate(off_t size);
	virtual int read(char* buf, size_t size, off_t offset, size_t *read_bytes);
	virtual int write(const char* buf, size_t size, off_t offset, size_t *write_bytes);
	virtual int config_cache(size_t single_cache_size_n, char *cachepath);
	virtual int log_level(char *log_mark);
private:
	size_t single_cache_size_m;
	std::string cachepath;
};
