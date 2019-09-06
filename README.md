# Multifs
multifs allows Linux and macOS to access multiple protocol file, this depends largely on the type of protocol carried in the file path. If the protocol type carried in the path is s3, then multifs can access S3 bucket.

Multfs is a stand-alone process. When a process starts, the parent process passes parameters into the socket used for communication. multfs wait for the command by socket.
# Interface
+ Structures used in communication
    ```
    typedef enum
    {
	    NFS_COMMAND_OPEN = 0,
	    NFS_COMMAND_CLOSE,
	    NFS_COMMAND_REMOVE,
	    NFS_COMMAND_READ,
	    NFS_COMMAND_WRITE,
	    NFS_COMMAND_FLUSH,
	    NTS_COMMAND_TRUNCATE,
	    NFS_COMMAND_STAT,
    } multifs_command_e;

    typedef enum
    {
	    OP_REQUEST = 1,        
	    OP_ANSWER,
    } multifs_opmode;

    typedef struct _multifs_command_header {

	    uint32_t magic;         // magic
	    uint32_t version;       // version
	    uint32_t mode;          // one of multifs_opmode
	    uint32_t command;       // one of multifs_command_e
	    uint32_t payload;       // payload lenght
	    uint32_t error;         // operate result
	    uint32_t sequence;      // Serial number
	    uint32_t reserved;      // Reserved field
    }multifs_command_header;
    ```
Each communication with multifs should contain a standard header (multifs_command_header). The data needed for operation is behind the header. The data carried by multifs is different according to the command, full as follows:
## open
+ payload struct
    ```
    typedef struct _multifs_command_open_in {
	    mode_t mode;                // access mode
	    char filepath[PATH_MAX];    // object path
    } multifs_command_open_in;
    ```


## read
+ payload struct
    ```
    typedef struct _multifs_command_read_in {
	    off_t offset;
	    size_t size;
    } multifs_command_read_in;
    ```
## write
+ payload struct
    ```
    typedef struct _multifs_command_write_in {
	    off_t offset;
	    size_t size;
	    char buf[0];
    } multifs_command_write_in;
    ```
## flush

## stat
+ answer struct
    ```
    typedef struct _multifs_command_stat_out {
	    struct stat stbuf;
    } multifs_command_stat_out;
    ```
## truncate
+ payload struct
    ```
    typedef struct _multifs_command_truncate_in {
	    size_t size;
    } multifs_command_truncate_in;
    ```
## remove
+ payload struct
    ```
    typedef struct _multifs_command_remove_in {
	    char filepath[PATH_MAX];
    } multifs_command_remove_in;
    ```
## close

# Build
+ ubuntu
    ```
    sudo apt-get install automake autotools-dev \
        fuse g++ git libcurl4-openssl-dev libfuse-dev \
        libssl-dev libxml2-dev make pkg-config
    
    make
    ```
# Example
+ read
    ```
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

	printf("wait read back!\n");

	multifs_command_header cmd_header = { 0 };
	int readlen = read(socket_par, &cmd_header, sizeof(multifs_command_header));
	if (readlen == -1) {
		break;
	}

 	unsigned char* data = new(std::nothrow) unsigned char[cmd_header.payload + 1];
 	if (data == nullptr) {
 		printf("read data error!\n");
 		break;
 	}

	readlen = read(socket_par, data, cmd_header.payload);
	if (readlen == -1) {
		delete data;
		data = nullptr;
		break;
	}
	
	printf("recv from child thread :%d %d %s\n", cmd_header.payload, readlen, data);
	delete data;
	data = nullptr;
    ```

