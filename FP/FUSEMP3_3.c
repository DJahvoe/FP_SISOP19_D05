/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Minor modifications and note by Andy Sayler (2012) <www.andysayler.com>
  Source: fuse-2.8.7.tar.gz examples directory
  http://sourceforge.net/projects/fuse/files/fuse-2.X/
  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
  gcc -Wall `pkg-config fuse --cflags` fusexmp.c -o fusexmp `pkg-config fuse --libs`
  Note: This implementation is largely stateless and does not maintain
        open file handels between open and release calls (fi->fh).
        Instead, files are opened and closed as necessary inside read(), write(),
        etc calls. As such, the functions that rely on maintaining file handles are
        not implmented (fgetattr(), etc). Those seeking a more efficient and
        more complete implementation may wish to add fi->fh support to minimize
        open() and close() calls and support fh dependent functions.
*/

#define FUSE_USE_VERSION 28
#define HAVE_SETXATTR

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif
#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
//Cek File or Folder
#include <pwd.h>
#include <grp.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

static const char *dirpath = "/home/dejahvoe";

char PlayerName[100] = "DejahvoePlayer";

char AllDirectory[200][10000];
char MP3Directory[200][10000];
char NotBelongToRoot[50][1000];
//Insert file ke BelongToRoot
	int rootindex = 0;
	int storeindex = 0;
char Hasil[1000];
//int head = 0;
int tail = 0; //size

void insert(char str[1000])
{
	if(tail < 1000)
	{
		strcpy(AllDirectory[tail], str);
		tail++;
	}
}

void pop()
{
	strcpy(Hasil, AllDirectory[0]);
	int x;
	for(x = 0; x < tail - 1; x++)
	{
		strcpy(AllDirectory[x], AllDirectory[x+1]);
	}
	tail--;
}

void display()
{
	int x;
	for(x = 0; x < tail; x++)
	{
		printf("%d. %s\n", x, AllDirectory[x]);
	}
	printf("\n");
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	printf("\nINI GET ATTR\n");
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);
	int res;
	printf("\nSTATATTR : %s\n\n", fpath);
	printf("DIRPATH : %s\n", dirpath);
	printf("PATH : %s\n", path);

	res = lstat(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}
static void *xmp_init(struct fuse_conn_info *conn)
{
	return 0;
}

static void xmp_destroy(void* private_data)
{

	int x;
	for(x = 0; x < rootindex; x++)
	{
		char old[1000] = "";
		sprintf(old, "%s/%s", dirpath, NotBelongToRoot[x]);
		char new[1000] = "";
		sprintf(new, "%s", MP3Directory[x]);
		printf("OLD: %s\n", old);
		printf("NEW: %s\n", new);
		rename(old,new);
	}
}

static int xmp_access(const char *path, int mask)
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res;

	res = access(fpath, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res;

	res = readlink(fpath, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}

int once = 0;
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	DIR *dp;
	struct dirent *de;

	//AllDir
	insert(fpath);

	
	while(tail > 0)
	{
		printf("\n################################\n");
		display();
		char iterpath[1000];
		pop();
		strcpy(iterpath, Hasil);

		(void) offset;
		(void) fi;
		printf("\n\n\nCurDir : %s\n", iterpath);
		dp = opendir(iterpath);
		if (dp == NULL)
			return -errno;

		while ((de = readdir(dp)) != NULL) {
			//printf("\n-------------------------------------------\n");
			struct stat st;
			memset(&st, 0, sizeof(st));
			st.st_ino = de->d_ino;
			st.st_mode = de->d_type << 12;

			//Recursive Check
			char currentname[1000] = "";
			strcpy(currentname, de->d_name);

			//Check hidden directory
			printf("\nCurrentNameBefore: %s\n", currentname);
			if(currentname[0] == '.' || currentname[strlen(currentname) - 1] == '~')
			{
				printf("DOT SKIPPED\n");
				continue;
			}

			
			int length = strlen(currentname);

			char extension[5] = ".mp3";

			int same = 0;
			int x;
			for(x = 0; x < 4; x++)
			{
				if(currentname[length - 4 + x] == extension[x])
				{
					same++;
				}
			}
			struct stat atr;
			sprintf(currentname, "%s/%s", iterpath, de->d_name);
			//printf("\nCurrentName: %s\n", currentname);
			stat(currentname, &atr);
			if(S_ISREG(atr.st_mode))
			{
				//if(strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0)
				//{
					//printf("\n\nDetected File : %s\n\n", de->d_name);
					if(same == 4 /*|| strcmp(PlayerName, de->d_name) == 0*/)
					{
						char real[1000];
						strcpy(real, de->d_name);
						printf("\nFOUNDED MP3!\n");
						printf("RealDir: %s\n", currentname);
						printf("RealName: %s\n", de->d_name);

						//Cut dirpath buat bisa ke-detect
						char cutted[1000] = "";
						int diff_index = 0;

						int x;
						for(x = 0; x < strlen(currentname); x++)
						{
							if(dirpath[x] != currentname[x])
							{
								cutted[diff_index] = currentname[x];
								diff_index++;
							}
						}
						printf("CuttedDir: %s\n", cutted);

						//If same then location = root
						int same = 1;
						for(x = 0; x < strlen(real); x++)
						{
							if(cutted[x+1] != real[x])
							{
								same = 0;
								break;
							}
						}

						
						if(same == 0)
						{
							strcpy(NotBelongToRoot[rootindex],de->d_name);
								rootindex++;
								//Keep MP3 Original Directory
								strcpy(MP3Directory[storeindex], currentname);
								storeindex++;

								char old[1000] = "";
								char new[1000] = "";
								sprintf(old, "%s", currentname);
								sprintf(new, "%s/%s",dirpath, de->d_name);
								
								printf("OLD: %s\n", old);
								printf("NEW: %s\n", new);
								rename(old, new);
						}
						else
						{
							//Ganti Directory Root
							if (filler(buf, de->d_name, &st, 0)) //BUTUH DIREVISI? (cutted -> de_dname)
							break;
						}
						
					}
				//}
			}
			else if(S_ISDIR(atr.st_mode))
			{
				int cur_length = strlen(currentname);
				if((strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 && strcmp(de->d_name, "tes") != 0)
					&& !((currentname[cur_length-1] == '.') && (currentname[cur_length-2] == '.') && (currentname[cur_length-3] == '/'))
					)
				{
					//printf("\n\nDetected Dir : %s\n\n", de->d_name);
					//char combined[1000];
					//sprintf(combined, "%s", de->d_name);
					insert(currentname);
					//xmp_readdir(combined, buf, filler, offset, fi);
				}
				else if(strcmp(de->d_name, "tes") != 0)
				{
					if(filler(buf, de->d_name, &st, 0))
						break;
				}
				
			}
			
		}
		closedir(dp);
	}

	
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res;

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(fpath, mode);
	else
		res = mknod(fpath, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res;

	res = mkdir(fpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *path)
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res;

	res = unlink(fpath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res;

	res = rmdir(fpath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{

	int res;

	res = symlink(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
	int res;

	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_link(const char *from, const char *to)
{
	int res;

	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res;

	res = chmod(fpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res;

	res = lchown(fpath, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res;

	res = truncate(fpath, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res;
	struct timeval tv[2];

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(fpath, tv);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res;

	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int fd;
	int res;

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int fd;
	int res;

	(void) fi;
	fd = open(fpath, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res;

	res = statvfs(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi) {

	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
    (void) fi;

    int res;
    res = creat(fpath, mode);
    if(res == -1)
	return -errno;

    close(res);

    return 0;
}


static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) fi;
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

#ifdef HAVE_SETXATTR
static int xmp_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = lsetxattr(fpath, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = lgetxattr(fpath, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);
	int res = llistxattr(fpath, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_removexattr(const char *path, const char *name)
{
	  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = lremovexattr(fpath, name);
	if (res == -1)
		return -errno;
	return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.init 		= xmp_init,
	.destroy	= xmp_destroy,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.link		= xmp_link,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
	.utimens	= xmp_utimens,
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
	.create         = xmp_create,
	.release	= xmp_release,
	.fsync		= xmp_fsync,
#ifdef HAVE_SETXATTR
	.setxattr	= xmp_setxattr,
	.getxattr	= xmp_getxattr,
	.listxattr	= xmp_listxattr,
	.removexattr	= xmp_removexattr,
#endif
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
