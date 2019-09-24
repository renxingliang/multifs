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


#include "smbio.h"

SmbIo::SmbIo()
{
}

SmbIo::~SmbIo()
{
}

int SmbIo::open(mode_t mode, char *filepath, size_t *file_size) {
	return 0;
}

int SmbIo::close() {
	return 0;
}


int SmbIo::remove(char *filepath)
{
	return 0;
}

int SmbIo::flush()
{
	return 0;
}

int SmbIo::getstat(char *filepath, struct stat * stbuf)
{
	return 0;
}

int SmbIo::truncate(off_t size)
{
	return 0;
}

int SmbIo::read(char * buf, size_t size, off_t offset, size_t *read_bytes)
{
	return 0;
}

int SmbIo::write(const char * buf, size_t size, off_t offset, size_t *write_bytes)
{
	return 0;
}

int SmbIo::config_cache(size_t single_cache_size_m, char *pcachepath) {
	single_cache_size_m = single_cache_size_m;
	if (pcachepath != nullptr) {
		cachepath = pcachepath;
	}

	return 0;
}

int SmbIo::log_level(char *log_mark) {
	return 0;
}