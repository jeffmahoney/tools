/*
 * Copyright (c) 2015 SUSE.  All Rights Reserved.
 * Author: Jeff Mahoney <jeffm@suse.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 * This program outputs a mapping of where data is present in a file.
 * It depends on the host file system supporting SEEK_DATA and SEEK_HOLE.
 * The output format is simple:
 * 
 * $ sparsemap /media/test/testfs.img.2338
 * 0-2097152 (2.0M)
 * 4194304-4276224 (80.0k)
 * 12582912-21004288 (8.32M)
 * 29360128-29392896 (32.0k)
 * 37748736-65781760 (26.752M)
 * 67108864-67112960 (4.0k)
 * 1111490560-1139523584 (26.752M)
 * 2185232384-10738466816 (7.989G)
 */

#include <sys/types.h>
#include <sys/fcntl.h>
#include <linux/fs.h>
#include <unistd.h>
#include <libgen.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
	int fd;
	off_t offset;
	off_t hole = 0, data = 1;
	char f[] = {' ', 'k', 'M', 'G', 'T'};

	if (argc != 2) {
		fprintf(stderr, "usage: %s <file>\n", basename(argv[0]));
		return 1;
	}

	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror(argv[1]);
		return 1;
	}

	while (hole != data) {
		off_t left, right;
		int i;
		data = lseek(fd, hole, SEEK_DATA);
		hole = lseek(fd, data, SEEK_HOLE);

		left = hole - data;
		right = 0;

		if (hole == (off_t)-1 || data == (off_t)-1)
			break;

		for (i = 0; i < sizeof(f)/sizeof(f[0]); i++) {
			if (left / 1024 == 0)
				break;
			right = left % 1024;
			left /= 1024;
		}

		printf("%lu-%lu (%lu.%lu%c)\n",
			data, hole, left, right, f[i]);
	}
	close(fd);

	return 0;

}
