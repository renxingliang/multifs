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

#include "../../ifile.h"
#include <stdint.h>

class SmbIo :
	public IFile
{
public:
	SmbIo();
	~SmbIo();

	virtual int open(mode_t mode, char filepath[PATH_MAX]);
	virtual int close();
	virtual int remove(char filepath[PATH_MAX]);
	virtual int flush();
	virtual int getstat(struct stat* stbuf);
	virtual int truncate(off_t size);
	virtual int read(char* buf, size_t size, off_t offset);
	virtual int write(const char* buf, size_t size, off_t offset);

private:

};