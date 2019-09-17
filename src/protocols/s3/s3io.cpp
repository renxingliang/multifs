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


#include <cstdio>
#include <unistd.h>
#include <dirent.h>
#include "s3io.h"

#include <string.h>
#include <iostream>
#include <tr1/memory>

S3Io::S3Io(){
	memset(file_path, 0, PATH_MAX);
}

S3Io::~S3Io(){
}


std::string S3Io::get_current_name() {
	std::string name;

	do {
		char dir[PATH_MAX] = { 0 };
		if (readlink("/proc/self/exe", dir, PATH_MAX) <= 0) {
			break;
		}

		char *path_end = strrchr(dir, '/');
		if (path_end == nullptr ||
			strlen(path_end) == strlen("/")) {
			break;
		}

		path_end++;
		name = path_end;
	} while (false);

	return name;
}

int S3Io::open(mode_t mode, char filepath[PATH_MAX]) {
	int iret = -1;
	char **args = nullptr;
	int vecsize = 0;

	do
	{
		file_info.flags = mode;

		std::vector<std::string> vecret = split_path(filepath);
		vecret.push_back("umask=0000");						// allow other user access
		vecret.push_back("check_cache_dir_exist");			// check cache dir before operate
		vecret.push_back("del_cache");						// allow delete cache
		vecret.push_back("-f");
		vecret.push_back("connect_timeout=30");
		vecret.push_back("readwrite_timeout=30");
		vecret.push_back(cachepath.size() != 0 ? std::string("use_cache=") + cachepath : "use_cache=/tmp");
		vecret.push_back(single_cache_size_m != 0 ? std::string("ensure_diskfree=") + std::to_string(single_cache_size_m) : "ensure_diskfree=4096");

		if (debug_mark.size() != 0) {
			vecret.push_back(std::string("dbglevel=") + debug_mark);	// allow print dbg information
		}

		vecsize = vecret.size();
		if (vecsize == 0) {
			break;
		}

		args = new(std::nothrow) char *[vecsize];
		if (args == nullptr)
		{
			break;
		}
		memset(args, 0, vecsize);

		// format s3fs parameters
		for (int i = 0; i < vecsize; i++) {
			args[i] = new(std::nothrow) char[vecret[i].size() + 1];
			if (args[i] == nullptr)
			{
				break;
			}
			memset(args[i], 0, vecret[i].size() + 1);
			strcpy(args[i], vecret[i].c_str());
		}

		iret = init(vecsize, args);
		if (iret == -1) {
			break;
		}

		s3fs_init(&conn_info);
		iret = s3fs_open(object_name.c_str(), &file_info, mode);
		if (iret != 0) {
			printf("open fail !\n");
			break;
		}

		strcpy(file_path, filepath);
		if ((file_info.flags&O_TRUNC) == O_TRUNC) {
			iret = truncate(0);
			if (iret != 0) {
				printf("truncate fail!\n");
				break;
			}
		}

		// If the file already exists, it cannot be overwritten.
		if ((file_info.flags&O_CREAT) == O_CREAT &&
			(file_info.flags&O_EXCL) == O_EXCL) {
			if (iret == 0) {
				iret = -1;
				break;
			}
		}

		iret = 0;
	} while (false);

	if (args != nullptr)
	{
		for (int i = 0; i < vecsize; i++)
		{
			if (args[i] != nullptr)
			{
				delete[] args[i];
				args[i] = nullptr;
			}
		}

		delete[] args;
		args = nullptr;
	}

	return iret;
}

int S3Io::close() {
	int iret = s3fs_flush(object_name.c_str(), &file_info);
	if (iret == 0) {
		iret = uninit();
	}

	return iret;
}

int S3Io::read(char* buf, size_t size, off_t offset) {
	if ((file_info.flags&O_RDONLY) != O_RDONLY &&
		(file_info.flags&O_RDWR) != O_RDWR) {
		printf("can not read target file\n");
		return 0;
	}

	return s3fs_read(object_name.c_str(), buf, size, offset, &file_info);
}

int S3Io::write(const char* buf, size_t size, off_t offset) {
	int iret = -1;

	do {
		if ((file_info.flags&& O_WRONLY) != O_WRONLY &&
			(file_info.flags&& O_RDWR) != O_RDWR) {
			return 0;
		}

		struct stat stbuf = { 0 };

		// check length is valid
		// There is no need to check whether the file exist or not.
		// If the file does not exist, the length of the file is 0.
		getstat(&stbuf);
		if (stbuf.st_size < offset) {
			printf("invalid offset\n");
			break;
		}

		iret = s3fs_write(object_name.c_str(), buf, size, offset, &file_info);
	} while (false);
	
	return iret;
}

int S3Io::remove(char filepath[PATH_MAX]) {
	int iret = open(0, filepath);
	if (iret == 0) {
		iret = s3fs_unlink(object_name.c_str());

		// Match open operation
		close();
	}
	
	return iret;
}

int S3Io::flush() {
	return s3fs_flush(object_name.c_str(), &file_info);
}

int S3Io::getstat(struct stat* stbuf) {
	return s3fs_getattr(object_name.c_str(), stbuf);
}

int S3Io::truncate(off_t size) {
	return s3fs_truncate(object_name.c_str(), size);
}

int S3Io::config_cache(size_t size_m, char *pcachepath) {

	single_cache_size_m = size_m;
	if (pcachepath != nullptr) {
		cachepath = pcachepath;
	}

	return 0;
}

int S3Io::log_level(char *log_mark) {
	if (log_mark != nullptr) {
		debug_mark = log_mark;
	}

	return 0;
}

// split path for formatting s3fs parameters
std::vector<std::string> S3Io::split_path(std::string strpath) {
	std::vector<std::string> vecret;

	do
	{
		std::string name = get_current_name();
		if (name.size() != 0){
			vecret.push_back(name);
		}

		int pos = strpath.find("//");
		if (pos == std::string::npos) {
			break;
		}

		strpath = strpath.substr(pos + strlen("//"), strpath.size() - pos - strlen("//"));
		if (strpath.size() == 0) {
			break;
		}

		// get ak:sk
		pos = strpath.find("@");
		if (pos == std::string::npos) {
			break;
		}

		std::string strask = strpath.substr(0, pos);
		if (strask.size() == 0) {
			break;
		}
		vecret.push_back("ak_sk=" + strask);

		// get domain
		strpath = strpath.substr(pos + strlen("@"), strpath.size() - pos - strlen("@"));
		if (strpath.size() == 0) {
			break;
		}

		pos = strpath.find("/");
		if (pos == std::string::npos) {
			break;
		}

		std::string strurl = strpath.substr(0, pos);
		if (strurl.size() == 0) {
			break;
		}
		vecret.push_back("url=https://" + strurl);

		// get bucket name
		strpath = strpath.substr(pos + strlen("/"), strpath.size() - pos - strlen("/"));
		if (strpath.size() == 0) {
			break;
		}

		pos = strpath.find("/");
		if (pos == std::string::npos) {
			break;
		}

		std::string strbucket = strpath.substr(0, pos);
		if (strbucket.size() == 0) {
			break;
		}
		vecret.push_back("bucket_name=" + strbucket);

		// get the object and save it
		strpath = strpath.substr(pos, strpath.size() - pos);
		if (strpath.size() == 0) {
			break;
		}
		object_name = strpath;
	} while (false);

	return vecret;
}