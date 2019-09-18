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
* File: main.cpp
* Description:
*     recv file option command and execute it
*/

#include <cstdio>
#include <string.h> 
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdint.h>

#include <iostream>
#include <string>

#include "ifile.h"
#include "./protocols/ftp/ftpio.h"
#include "./protocols/s3/s3io.h"
#include "./protocols/smb/smbio.h"
#include "multifs_proto.h"

static int socket_child = 0;	// the socket for communicatting with parent
static IFile *file = nullptr;	// the file handle 
static size_t single_cache_size_m = 0;
static char debug_mark[PATH_MAX] = { 0 };
static char cachepath[PATH_MAX] = { 0 };

void get_tag_fs(char *path) {

	do {
		if (path == nullptr) {
			break;
		}

		std::string strpath = path;
		int pos = strpath.find(":");
		if (pos == std::string::npos) {
			break;
		}

		std::string strprotocol_type = strpath.substr(0, pos);
		if (strprotocol_type.size() == 0) {
			break;
		}

		if (strprotocol_type.compare("s3") == 0) {
			file = new(std::nothrow) S3Io;
			if (file == nullptr) {
				break;
			}
		}
		else if (strprotocol_type.compare("ftp") == 0) {
			file = new(std::nothrow) FtpIo;
			if (file == nullptr) {
				break;
			}
		}
		else  if (strprotocol_type.compare("smb") == 0) {
			file = new(std::nothrow) SmbIo;
			if (file == nullptr) {
				break;
			}
		}
	} while (false);
}

int dspcmd(multifs_command_header *msg_header, unsigned char *data) {
	multifs_command_header *msg_out = nullptr;

	do {
		if (msg_header == nullptr) {
			break;
		}

		if (msg_header->command != MFS_COMMAND_CLOSE && 
			msg_header->command != MFS_COMMAND_FLUSH && 
			msg_header->command != MFS_COMMAND_STAT &&
			data == nullptr) {
			break;
		}

		if (msg_header->command != MFS_COMMAND_OPEN &&
			msg_header->command != MFS_COMMAND_REMOVE &&
			file == nullptr) {
		}

		msg_out = msg_header;

		switch (msg_header->command) {
		case MFS_COMMAND_OPEN: {
			msg_out->payload = 0;

			multifs_command_open_in *cmd_open = (multifs_command_open_in*)data;
			get_tag_fs(cmd_open->filepath);
			if (file == nullptr) {
				msg_out->error == -1;
				break;
			}

			file->config_cache(single_cache_size_m, cachepath);
			file->log_level(debug_mark);
			msg_out->error = file->open(cmd_open->mode, cmd_open->filepath);

			break;
		}

		case MFS_COMMAND_CLOSE: {
			msg_out->error = file->close();
			msg_out->payload = 0;

			break;
		}

		case MFS_COMMAND_REMOVE: {
			msg_out->payload = 0;

			multifs_command_remove_in *cmd_remove = (multifs_command_remove_in*)data;
			get_tag_fs(cmd_remove->filepath);
			if (file == nullptr) {
				msg_out->error == -1;
				break;
			}
			msg_out->error = file->remove(cmd_remove->filepath);

			break;
		}

		case MFS_COMMAND_READ: {
			msg_header->payload = 0;

			multifs_command_read_in *cmd_read = (multifs_command_read_in*)data;
			unsigned char* read_buffer = new(std::nothrow) unsigned char[cmd_read->size + 1];
			if (read_buffer == nullptr) {
				break;
			}
			memset(read_buffer, 0, cmd_read->size + 1);
			printf("need read len %d\n", cmd_read->size);

			size_t read_bytes = 0;
			msg_header->error = file->read((char*)read_buffer, cmd_read->size, cmd_read->offset, &read_bytes);
			if (msg_header->error == 0) {
				int len = cmd_read->size + sizeof(multifs_command_header) + sizeof(multifs_command_read_out);
				unsigned char* p = new(std::nothrow) unsigned char[len + 1];
				if (p != nullptr) {
					memset(p, 0, len + 1);
					msg_header->payload = cmd_read->size + sizeof(multifs_command_read_out);;
					memcpy(p, msg_header, sizeof(multifs_command_header));
					multifs_command_read_out *pout = (multifs_command_read_out*)(p + sizeof(multifs_command_header));
					pout->size = read_bytes;
					memcpy(pout->buf, read_buffer, read_bytes);
					printf("read data %s\n", pout->buf);
					msg_out = (multifs_command_header*)p;
				}
			}

			delete[] read_buffer;
			read_buffer = nullptr;

			break;
		}

		// call flush after write
		case MFS_COMMAND_WRITE: {
			msg_header->payload = 0;

			multifs_command_write_in *cmd_write = (multifs_command_write_in*)data;
			size_t write_bytes = 0;
			msg_header->error = file->write(cmd_write->buf, cmd_write->size, cmd_write->offset, &write_bytes);
			if (msg_header->error == 0) {
				break;
			}

			size_t len = sizeof(multifs_command_header) + sizeof(multifs_command_write_out);
			unsigned char*p = new(std::nothrow) unsigned char[len + 1];
			if (p == nullptr) {
				break;
			}

			memset(p, 0, len + 1);
			msg_header->payload = sizeof(multifs_command_write_out);
			memcpy(p, msg_header, sizeof(multifs_command_header));
			multifs_command_write_out *pout = (multifs_command_write_out*)(p + sizeof(multifs_command_header));
			pout->size = write_bytes;
			msg_out = (multifs_command_header*)p;

			break;
		}

		case MFS_COMMAND_FLUSH: {
			msg_out->error = file->flush();
			msg_out->payload = 0;

			break;
		}

		case MFS_COMMAND_TRUNCATE: {
			multifs_command_truncate_in *cmd_truncat = (multifs_command_truncate_in*)data;
			msg_out->error = file->truncate(cmd_truncat->size);
			msg_out->payload = 0;

			break;
		}

		case MFS_COMMAND_STAT: {
			printf("get cmd stat\n");
			multifs_command_stat_in *cmd_truncat = (multifs_command_stat_in*)data;
			get_tag_fs(cmd_truncat->filepath);
			if (file == nullptr) {
				msg_out->error == -1;
				break;
			}

			msg_header->payload = 0;

			printf("get stat success\n");
			multifs_command_stat_out stat = { 0 };
			msg_header->error = file->getstat(cmd_truncat->filepath, &stat.stbuf);
			if (msg_out->error != 0) {
				break;
			}

			printf("get stat success\n");
			int len = sizeof(multifs_command_header) + sizeof(multifs_command_stat_out);
			unsigned char *p = new(std::nothrow) unsigned char[len + 1];
			if (p == nullptr) {
				break;
			}
			memset(p, 0, len + 1);

			msg_header->payload = sizeof(multifs_command_stat_out);
			memcpy(p, msg_header, sizeof(multifs_command_header));
			memcpy(p + sizeof(multifs_command_header), &stat, sizeof(multifs_command_stat_out));
			msg_out = (multifs_command_header*)p;
			printf("get stat success\n");

			break;
		}
		case MFS_COMMAND_CACHE: {
			multifs_command_cache_in *config = (multifs_command_cache_in*)data;
			single_cache_size_m = config->free_disk_size_m;
			strcpy(cachepath, config->cachepath);
			msg_out->payload = 0;
			break;
		}

		case MFS_COMMAND_LOG: {
			multifs_command_log_in *log = (multifs_command_log_in *)data;
			strcpy(debug_mark, log->debug_mark);
			msg_out->payload = 0;
			break;
		}
		}
	} while (false);

	msg_out->mode = OP_ANSWER;
	msg_out->error = -msg_out->error;
	int iret = write(socket_child, msg_out, sizeof(multifs_command_header) + msg_out->payload);
	if (iret == -1)	{
		printf("write pipe error!\n");
	}

	iret = msg_out->error;
	if (msg_out != nullptr && 
		(msg_out->command == MFS_COMMAND_STAT || (msg_out->command == MFS_COMMAND_READ && msg_out->payload != 0))) {
		delete[] msg_out;
		msg_out = nullptr;
	}

	return iret;
}


int main(int argc, char *argv[])
{
	int iret = 0;
	
	do {
		if (argc == 1) {
			printf("parameters error !\n");
			break;
		}

		socket_child = atoi(argv[1]);

		while (true) {
			// read header
			multifs_command_header msg_header = { 0 };
			int read_bytes = read(socket_child, &msg_header, sizeof(multifs_command_header));
			if (read_bytes == -1) {
				printf("read pipe error!\n");
				break;
			}

			// check header
			if (msg_header.version != MULTIFS_PROTO_VERSION ||
				msg_header.magic != MULTIFS_HEADER_MAGIC ||
				msg_header.mode != OP_REQUEST) {
				continue; 
			}

			// get the cmd payload
			unsigned char *pbuffer = nullptr;
			if (msg_header.payload != 0) {
				pbuffer = new(std::nothrow) unsigned char[msg_header.payload + 1];
				if (pbuffer == nullptr) {
					break;
				}

				memset(pbuffer, 0, msg_header.payload + 1);
				read_bytes = read(socket_child, pbuffer, msg_header.payload);
				if (read_bytes == -1) {
					delete pbuffer;
					pbuffer = nullptr;
					break;
				}				
			}

			// dispatch option
			iret = dspcmd(&msg_header, pbuffer);
			if (pbuffer != nullptr) {
				delete pbuffer;
				pbuffer = nullptr;
			}

			// Delete is a separate file operation, so exit the child process after the operation is completed.
			if (msg_header.command == MFS_COMMAND_CLOSE ||
				msg_header.command == MFS_COMMAND_REMOVE ||
				(msg_header.command == MFS_COMMAND_OPEN && iret != 0)) {
				printf("get close event! %d\n", iret);
				break;
			}

			if (iret == -1) {
				printf("option fail!\n");
				break;
			}
		}

		if (file != nullptr) {
			delete file;
			file = nullptr;
		}
	} while (false);

	printf("child process exist!\n");
    return iret;
}
