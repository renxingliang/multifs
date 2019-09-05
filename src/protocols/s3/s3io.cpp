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
#include "s3io.h"

#include <string.h>
#include <iostream>
#include <tr1/memory>

S3Io::S3Io(){
	memset(file_path, 0, PATH_MAX);
}

S3Io::~S3Io(){
}

int S3Io::open(mode_t mode, char filepath[PATH_MAX]) {
	int iret = -1;
	char **args = nullptr;

	do
	{
		std::vector<std::string> vecret = split_path(filepath);
		vecret.push_back("umask=0000");						// allow other user access
		//vecret.push_back("use_cache=/home/pc/Desktop");	// set cache dir
		vecret.push_back("del_cache");						// allow delete cache
		//vecret.push_back("dbglevel=dbg");					// allow print dbg information
		//vecret.push_back("-f");

		int vecsize = vecret.size();
		if (vecsize == 0 ||
			vecsize > 20) {
			break;
		}

		args = new(std::nothrow) char *[40];
		if (args == nullptr)
		{
			break;
		}
		memset(args, 0, 20);

		args[0] = new(std::nothrow) char[strlen("multfs") + 1];
		if (args[0] == nullptr)
		{
			delete[] args;
			break;
		}
		memset(args[0], 0, strlen("multfs") + 1);
		strcpy(args[0], "multfs");

		// format s3fs parameters
		for (int i = 0; i < vecsize; i++) {
			args[i + 1] = new(std::nothrow) char[vecret[i].size() + 1];
			if (args[i+1] == nullptr)
			{
				break;
			}
			memset(args[i + 1], 0, vecret[i].size() + 1);
			strcpy(args[i + 1], vecret[i].c_str());
		}

		iret = init(vecsize + 1, args);
		if (iret == -1) {
			break;
		}

		s3fs_init(&conn_info);

		iret = s3fs_open(object_name.c_str(), &file_info, mode);
		if (iret == -1)
		{
			break;
		}

		strcpy(file_path, filepath);

		iret = 0;
	} while (false);

	if (args != nullptr)
	{
		for (int i = 0; i < 40; i++)
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
	return s3fs_read(object_name.c_str(), buf, size, offset, &file_info);
}

int S3Io::write(const char* buf, size_t size, off_t offset) {
	return s3fs_write(object_name.c_str(), buf, size, offset, &file_info);
}

int S3Io::remove(char filepath[PATH_MAX]) {
	int iret = open(0, filepath);
	if (iret == 0) {
		iret = s3fs_unlink(object_name.c_str());
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

// split path for formatting s3fs parameters
std::vector<std::string> S3Io::split_path(std::string strpath) {
	std::vector<std::string> vecret;

	do
	{
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