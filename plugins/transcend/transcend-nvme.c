#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "linux/nvme_ioctl.h"
#include "nvme.h"
#include "nvme-print.h"
#include "nvme-ioctl.h"
#include "plugin.h"
#include "argconfig.h"
#include "suffix.h"

#define CREATE_CMD
#include "transcend-nvme.h"

static const __u32 OP_BAD_BLOCK = 0xc2;
static const __u32 DW10_BAD_BLOCK = 0x400;
static const __u32 DW12_BAD_BLOCK = 0x5a;

static const __u32 OP_WAI = 0x02;
static const __u32 DW10_WAI = 0x3ff04c6;


static int getHealthValue(int argc, char **argv, struct command *cmd, struct plugin *plugin)
{
	struct nvme_smart_log smart_log;
	char *desc = "Get nvme health percentage.";
 	int result=0, fd;
	int  percent_used = 0, healthvalue=0;
	 
	OPT_ARGS(opts) = {
		OPT_END()
	};

	fd = parse_and_open(argc, argv, desc, opts);
	 
	if (fd < 0) {
		printf("\nDevice not found \n");;
		return -1;
	}
	result = nvme_smart_log(fd, 0xffffffff, &smart_log);
	if (!result) {
		printf("Transcend NVME heath value: ");
		percent_used =smart_log.percent_used;
		
		if(percent_used>100 || percent_used<0)
		{
			printf("0%%\n");
		}
		else
		{
			healthvalue = 100 - percent_used;
			printf("%d%%\n",healthvalue);
		}
			 
	}

	return result;
}

 
static int getBadblock(int argc, char **argv, struct command *cmd, struct plugin *plugin)
{

	char *desc = "Get nvme bad block number.";
 	int result=0, fd;
 
	OPT_ARGS(opts) = {
		 
		OPT_END()
	};

	fd = parse_and_open(argc, argv, desc, opts);
	if (fd < 0) {
		printf("\nDevice not found \n");;
		return -1;
	}
	unsigned char data[1]={0};
	struct nvme_passthru_cmd nvmecmd;
	memset(&nvmecmd,0,sizeof(nvmecmd));
	nvmecmd.opcode=OP_BAD_BLOCK;
	nvmecmd.cdw10=DW10_BAD_BLOCK;
	nvmecmd.cdw12=DW12_BAD_BLOCK;
	nvmecmd.addr =(__u64) data;
	nvmecmd.data_len = 0x1;
	result = nvme_submit_admin_passthru(fd,&nvmecmd);
	if(!result) {
		int badblock  = data[0];
		printf("Transcend NVME badblock count: %d\n",badblock);
	}

	return result;
}

static int getWAI(int argc, char **argv, struct command *cmd, struct plugin *plugin)
{

	char *desc = "Get NVME write amplification.";
 	int result=0, fd;
 
	OPT_ARGS(opts) = {
		 
		OPT_END()
	};

	fd = parse_and_open(argc, argv, desc, opts);
	if (fd < 0) {
		printf("\nDevice not found \n");;
		return -1;
	}
	unsigned char data[1024]={0};
	struct nvme_passthru_cmd nvmecmd;
	memset(&nvmecmd,0,sizeof(nvmecmd));
	nvmecmd.opcode=OP_WAI;
	nvmecmd.cdw10=DW10_WAI;
	nvmecmd.addr =(__u64) data;
	nvmecmd.data_len = 0x400;
	result = nvme_submit_admin_passthru(fd,&nvmecmd);
	if(!result) {
		float waitlc  = data[1008];
		float waislc  = data[1016];
		printf("Transcend NVME WAI TLC: %f\n",waitlc/100);
		printf("Transcend NVME WAI SLC: %f\n",waislc/100);
	}

	return result;
}


