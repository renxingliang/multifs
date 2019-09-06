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


#include "ftpio.h"

int FtpIo::open(mode_t mode, char filepath[PATH_MAX]){
	return 0;
}

int FtpIo::close(){
	return 0;
}

int FtpIo::remove(char filepath[PATH_MAX])
{
	return 0;
}

int FtpIo::flush() {

	return 0;
}

int FtpIo::getstat(struct stat * stbuf)
{
	return 0;
}

int FtpIo::truncate(off_t size)
{
	return 0;
}

int FtpIo::read(char * buf, size_t size, off_t offset)
{
	return 0;
}

int FtpIo::write(const char * buf, size_t size, off_t offset)
{
	return 0;
}

int FtpIo::config(size_t single_cache_size_m, char *pcachepath, char *debug_mark) {
	single_cache_size_m = single_cache_size_m;
	if (pcachepath != nullptr)
	{
		cachepath = pcachepath;
	}
}