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
* File: multfs-test.cpp
* Description:
*     the interface for testting multfs
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
#include <sys/stat.h>
#include <iostream>
#include <tr1/memory>

#include "multifs_proto.h"

#define BUFFER_SIZE	(256)	

int socket_par = 0;

extern "C" {
	int init();
	int test_open(char *szpath, mode_t mode);
	int test_close();
	int test_flush();
	int test_read(off_t offset, char* data, size_t size);
	int test_remove(char *szpath);
	int test_stat();
	int test_write(off_t offset, char *data, size_t size);
	int test_truncate(size_t size);
	int config_cache(size_t single_cache_size_n, char *cachepath);
	int log_level(char *debug_mark);
}

static int child_proc = 0;

int check_child_exist()
{
	int iret = -1;

	do {
		if (child_proc == 0) {
			break;
		}

		char command[PATH_MAX] = { 0 };
		sprintf(command, "ps -C %d|wc -l", child_proc);

		FILE *fp = popen(command, "r");
		if (fp == NULL) {
			break;
		}

		char buf[PATH_MAX] = { 0 };
		if ((fgets(buf, PATH_MAX, fp)) != NULL) {
			if (atoi(buf) == 1) {
				iret = 0;
			}
		}
		pclose(fp);
	} while (false);

	return iret;
}

int test_open(char *szpath, mode_t mode) {

	int iret = 1;

	do {
		if (szpath == nullptr) {
			printf("path invalid!\n");
			break;
		}

		// test open
		int len = sizeof(multifs_command_header) + sizeof(multifs_command_open_in);
		unsigned char* p = new unsigned char[len + 1];
		if (p == nullptr) {
			break;
		}
		std::auto_ptr<unsigned char> tmp(p);
		memset(p, 0, len + 1);

		multifs_command_header *pmsg_header = (multifs_command_header*)p;
		pmsg_header->command = NFS_COMMAND_OPEN;
		pmsg_header->magic = MULTIFS_HEADER_MAGIC;
		pmsg_header->mode = OP_REQUEST;
		pmsg_header->version = MULTIFS_VERSION;
		pmsg_header->sequence = 1;
		pmsg_header->payload = sizeof(multifs_command_open_in);

		//char szpath[] = "s3://AKIDe8NsCLD0TkJh5DDuNbdT3wiIfmeK5LRH:qRmsmUH5jCZhaOyJQH0Ui5JLBZlkZBYk@cos.ap-chengdu.myqcloud.com/test-1259750376/123";
		multifs_command_open_in *ppayload = (multifs_command_open_in*)(p + sizeof(multifs_command_header));
		ppayload->mode = mode;
		strcpy(ppayload->filepath, szpath);
		ssize_t bytes = write(socket_par, p, len);
		if (bytes == -1) {
			printf("open error, send data fail!\n");
			break;
		}

		multifs_command_header cmd_header = { 0 };
		int readlen = read(socket_par, &cmd_header, sizeof(multifs_command_header));
		if (readlen == -1) {
			printf("open request fail!\n");
			break;
		}
		else {
			if (cmd_header.command == NFS_COMMAND_OPEN) {
				iret = cmd_header.error;
			}
		}
	} while (false);

	return iret;
}

int test_close() {
	int iret = 1;

	do {
		if (check_child_exist() == -1) {
			break;
		}

		multifs_command_header cmd_header = { 0 };
		cmd_header.command = NFS_COMMAND_CLOSE;
		cmd_header.magic = MULTIFS_HEADER_MAGIC;
		cmd_header.mode = OP_REQUEST;
		cmd_header.version = MULTIFS_VERSION;
		cmd_header.sequence = 1;
		cmd_header.payload = 0;
		int bytes = write(socket_par, &cmd_header, sizeof(multifs_command_header));
		if (bytes == -1) {
			printf("open error, send data fail!\n");
			break;
		}

		multifs_command_header cmd_header_respone = { 0 };
		int readlen = read(socket_par, &cmd_header_respone, sizeof(multifs_command_header));
		if (readlen == -1) {
			break;
		}


		iret = cmd_header_respone.error;
	} while (false);

	return iret;
}

int test_flush() {
	int iret = 1;

	do {
		if (check_child_exist() == -1) {
			break;
		}

		multifs_command_header cmd_header = { 0 };
		cmd_header.command = NFS_COMMAND_FLUSH;
		cmd_header.magic = MULTIFS_HEADER_MAGIC;
		cmd_header.mode = OP_REQUEST;
		cmd_header.version = MULTIFS_VERSION;
		cmd_header.sequence = 1;
		cmd_header.payload = 0;
		int bytes = write(socket_par, &cmd_header, sizeof(multifs_command_header));
		if (bytes == -1) {
			printf("open error, send data fail!\n");
			break;
		}

		multifs_command_header cmd_header_respone = { 0 };
		int readlen = read(socket_par, &cmd_header_respone, sizeof(multifs_command_header));
		if (readlen == -1) {
			break;
		}

		iret = cmd_header_respone.error;
	} while (false);

	return iret;
}

int test_read(off_t offset, char* data, size_t size) {
	int iret = 0;

	do {
		if (check_child_exist() == -1) {
			break;
		}

		int len = sizeof(multifs_command_header) + sizeof(multifs_command_read_in);
		unsigned char *p = new unsigned char[len + 1];
		if (p == nullptr) {
			break;
		}
		std::auto_ptr<unsigned char> tmp(p);
		memset(p, 0, len + 1);

		multifs_command_header *msg_header = (multifs_command_header*)p;
		msg_header->command = NFS_COMMAND_READ;
		msg_header->magic = MULTIFS_HEADER_MAGIC;
		msg_header->mode = OP_REQUEST;
		msg_header->version = MULTIFS_VERSION;
		msg_header->sequence = 1;
		msg_header->payload = sizeof(multifs_command_read_in);
		multifs_command_read_in *read_in = (multifs_command_read_in *)(p + sizeof(multifs_command_header));
		read_in->offset = offset;
		read_in->size = size;
		int bytes = write(socket_par, p, len);
		if (bytes == -1) {
			printf("open error, send data fail!\n");
			break;
		}

		multifs_command_header cmd_header = { 0 };
		int readlen = read(socket_par, &cmd_header, sizeof(multifs_command_header));
		if (readlen == -1 ||
			cmd_header.payload == 0) {
			break;
		}

		readlen = read(socket_par, data, cmd_header.payload);
		if (readlen == -1) {
			break;
		}

		iret = cmd_header.error;
	} while (false);

	return iret;
}

int test_write(off_t offset, char *data, size_t size) {
	int iret = 0;

	do {
		if (check_child_exist() == -1) {
			printf("test_ write check child exist\n");
			break;
		}

		int len = sizeof(multifs_command_header) + sizeof(multifs_command_write_in) + size;
		unsigned char* p = new unsigned char[len + 1];
		if (p == nullptr) {
			break;
		}
		std::auto_ptr<unsigned char> tmp(p);
		memset(p, 0, len + 1);

		multifs_command_header* msg_header = (multifs_command_header*)p;
		msg_header->command = NFS_COMMAND_WRITE;
		msg_header->magic = MULTIFS_HEADER_MAGIC;
		msg_header->mode = OP_REQUEST;
		msg_header->version = MULTIFS_VERSION;
		msg_header->sequence = 1;
		msg_header->payload = sizeof(multifs_command_write_in) + strlen(data);
		multifs_command_write_in *write_in = (multifs_command_write_in *)(p + sizeof(multifs_command_header));
		write_in->offset = offset;
		write_in->size = size;
		strcpy(write_in->buf, data);
		int bytes = write(socket_par, p, len);
		if (bytes == -1) {
			printf("open error, send data fail!\n");
			break;
		}

		multifs_command_header cmd_header = { 0 };
		int readlen = read(socket_par, &cmd_header, sizeof(multifs_command_header));
		if (readlen == -1) {
			break;
		}

		iret = cmd_header.error;
	} while (false);

	return iret;
}

int test_truncate(size_t size) {
	int iret = 1;

	do {
		if (check_child_exist() == -1) {
			break;
		}

		int len = sizeof(multifs_command_header) + sizeof(multifs_command_truncate_in);
		unsigned char* p = new unsigned char[len + 1];
		if (p == nullptr) {
			break;
		}
		std::auto_ptr<unsigned char> tmp(p);
		memset(p, 0, len + 1);

		multifs_command_header* msg_header = (multifs_command_header*)p;
		msg_header->command = NTS_COMMAND_TRUNCATE;
		msg_header->magic = MULTIFS_HEADER_MAGIC;
		msg_header->mode = OP_REQUEST;
		msg_header->version = MULTIFS_VERSION;
		msg_header->sequence = 1;
		msg_header->payload = sizeof(multifs_command_truncate_in);
		multifs_command_truncate_in *truncate_in = (multifs_command_truncate_in *)(p + sizeof(multifs_command_header));
		truncate_in->size = size;
		int bytes = write(socket_par, p, len);
		if (bytes == -1) {
			printf("open error, send data fail!\n");
			break;
		}

		multifs_command_header cmd_header = { 0 };
		int readlen = read(socket_par, &cmd_header, sizeof(multifs_command_header));
		if (readlen == -1) {
			break;
		}

		iret = cmd_header.error;
	} while (false);

	return iret;
}

int test_stat() {
	int iret = 1;

	do {
		if (check_child_exist() == -1) {
			break;
		}

		multifs_command_header msg_header = { 0 };
		msg_header.command = NFS_COMMAND_STAT;
		msg_header.magic = MULTIFS_HEADER_MAGIC;
		msg_header.mode = OP_REQUEST;
		msg_header.version = MULTIFS_VERSION;
		msg_header.sequence = 1;
		msg_header.payload = 0;
		int bytes = write(socket_par, &msg_header, sizeof(multifs_command_header));
		if (bytes == -1) {
			printf("open error, send data fail!\n");
			break;
		}

		multifs_command_header cmd_header = { 0 };
		int readlen = read(socket_par, &cmd_header, sizeof(multifs_command_header));
		if (readlen == -1) {
			break;
		}

		iret = cmd_header.error;

		multifs_command_stat_out stat_out = { 0 };
		readlen = read(socket_par, &stat_out, sizeof(multifs_command_stat_out));
		if (readlen == -1) {
			break;
		}

		printf("file size %d\n", stat_out.stbuf.st_size);
		printf("atim %s\n", ctime(&stat_out.stbuf.st_atime));
		printf("mtim %s\n", ctime(&stat_out.stbuf.st_mtime));
		printf("ctim %s\n", ctime(&stat_out.stbuf.st_ctime));
	} while (false);

	return iret;
}

int test_remove(char *szpath) {
	int iret = 1;

	do {
		if (check_child_exist() == -1) {
			break;
		}

		int len = sizeof(multifs_command_header) + sizeof(multifs_command_remove_in);
		unsigned char* p = new unsigned char[len + 1];
		if (p == nullptr) {
			break;
		}
		std::auto_ptr<unsigned char> tmp(p);
		memset(p, 0, len + 1);

		multifs_command_header* msg_header = (multifs_command_header*)p;
		msg_header->command = NFS_COMMAND_REMOVE;
		msg_header->magic = MULTIFS_HEADER_MAGIC;
		msg_header->mode = OP_REQUEST;
		msg_header->version = MULTIFS_VERSION;
		msg_header->sequence = 1;
		msg_header->payload = sizeof(multifs_command_remove_in);
		multifs_command_remove_in *remove_in = (multifs_command_remove_in *)(p + sizeof(multifs_command_header));
		strcpy(remove_in->filepath, szpath);
		int bytes = write(socket_par, p, len);
		if (bytes == -1) {
			printf("open error, send data fail!");
			break;
		}

		multifs_command_header cmd_header = { 0 };
		int readlen = read(socket_par, &cmd_header, sizeof(multifs_command_header));
		if (readlen == -1) {
			break;
		}

		iret = cmd_header.error;
	} while (false);

	return iret;
}

int config_cache(size_t single_cache_size_n, char *cachepath) {
	int iret = 1;

	do {
		if (cachepath == nullptr) {
			printf("path invalid!\n");
			break;
		}

		if (check_child_exist() == -1) {
			break;
		}

		// test open
		int len = sizeof(multifs_command_header) + sizeof(multifs_command_cache_in);
		unsigned char* p = new unsigned char[len + 1];
		if (p == nullptr) {
			break;
		}
		std::auto_ptr<unsigned char> tmp(p);
		memset(p, 0, len + 1);

		multifs_command_header *pmsg_header = (multifs_command_header*)p;
		pmsg_header->command = NFS_COMMAND_CACHE;
		pmsg_header->magic = MULTIFS_HEADER_MAGIC;
		pmsg_header->mode = OP_REQUEST;
		pmsg_header->version = MULTIFS_VERSION;
		pmsg_header->sequence = 1;
		pmsg_header->payload = sizeof(multifs_command_cache_in);

		multifs_command_cache_in *ppayload = (multifs_command_cache_in*)(p + sizeof(multifs_command_header));
		ppayload->free_disk_size_m = single_cache_size_n;
		strcpy(ppayload->cachepath, cachepath);
		ssize_t bytes = write(socket_par, p, len);
		if (bytes == -1) {
			printf("open error, send data fail!\n");
			break;
		}

		multifs_command_header cmd_header = { 0 };
		int readlen = read(socket_par, &cmd_header, sizeof(multifs_command_header));
		if (readlen == -1) {
			printf("config request fail!\n");
			break;
		}
		else {
			if (cmd_header.command == NFS_COMMAND_OPEN) {
				iret = cmd_header.error;
			}
		}
	} while (false);

	return iret;
}

int log_level(char *debug_mark) {
	int iret = 1;

	do {
		if (debug_mark == nullptr) {
			printf("path invalid!\n");
			break;
		}

		if (check_child_exist() == -1) {
			break;
		}


		// test open
		int len = sizeof(multifs_command_header) + sizeof(multifs_command_log_in);
		unsigned char* p = new unsigned char[len + 1];
		if (p == nullptr) {
			break;
		}
		std::auto_ptr<unsigned char> tmp(p);
		memset(p, 0, len + 1);

		multifs_command_header *pmsg_header = (multifs_command_header*)p;
		pmsg_header->command = NFS_COMMAND_LOG;
		pmsg_header->magic = MULTIFS_HEADER_MAGIC;
		pmsg_header->mode = OP_REQUEST;
		pmsg_header->version = MULTIFS_VERSION;
		pmsg_header->sequence = 1;
		pmsg_header->payload = sizeof(multifs_command_log_in);

		multifs_command_log_in *ppayload = (multifs_command_log_in*)(p + sizeof(multifs_command_header));
		strcpy(ppayload->debug_mark, debug_mark);
		ssize_t bytes = write(socket_par, p, len);
		if (bytes == -1) {
			printf("open error, send data fail!\n");
			break;
		}

		multifs_command_header cmd_header = { 0 };
		int readlen = read(socket_par, &cmd_header, sizeof(multifs_command_header));
		if (readlen == -1) {
			printf("log level request fail!\n");
			break;
		}
		else {
			if (cmd_header.command == NFS_COMMAND_OPEN) {
				iret = cmd_header.error;
			}
		}
	} while (false);

	return iret;
}

int init() {
	int iret = 1;
	int fd[2] = { 0 };

	do {
		if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0) {
			break;
		}

		socket_par = fd[0];
		int  child = fd[1];
		pid_t pid = fork();
		if (pid < 0) {
			printf("fork error!\n");
			break;
		}

		if (pid == 0) {
			char * pcur_dir = get_current_dir_name();
			if (pcur_dir == NULL) {
				break;
			}

			char szfullpath[1024] = { 0 };
			strcpy(szfullpath, pcur_dir);
			strcat(szfullpath, "/multifs");
			printf("%s\n", szfullpath);

			char string[25];
			sprintf(string, "%d", child);

			if (execl(szfullpath, "multifs", string, NULL) == -1) {
				printf("create child error!\n");
			}
			else {
				printf("create child success!\n");
			}
			exit(0);
		}
		else {
			child_proc = pid;
			iret = 0;
		}
	} while (false);

	return iret;
}

int main(){
	return 0;
}