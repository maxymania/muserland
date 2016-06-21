/*
 * Copyright (c) 2016 Simon Schmidt
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <stdio.h>
#include <sys/stat.h>
#include <libgen.h>
#include <unistd.h>
#include <fcntl.h>
typedef const char* string_t;

#define bnm(x) (string_t)basename((char*)x)
int copy_object(string_t from, string_t to);
int copy_link(string_t from, string_t to);
int copy_file(string_t from, string_t to);
int copy_dir(string_t from, string_t to);

char buffer[4096];
char buffer2[4096];


#ifndef S_ISDIR
#ifdef S_ISFILE
#define S_ISDIR S_ISFILE
#else
#error "No S_ISDIR, No S_ISFILE"
#endif
#endif

int main(int argc,const string_t* argv){
	int status,i;
	struct stat statbuf;
	if(argc<3)
		goto printhowto;
	if(stat(argv[argc-1],&statbuf)){
		/*
		 * The destination file/dir does not exist.
		 */
		if(argc>3)
			goto printhowto;
	}else{
		/*
		 * The destination file/dir does exist.
		 */
		if(S_ISDIR(statbuf.st_mode))
			/*
			 * The destination is a directory.
			 */
			goto copydir;
		else /*if(!(S_ISREG(statbuf.st_mode)))*/
			/*
			 * If the destination is not a directory, fail.
			 */
			goto printhowto;
	}
	return copy_object(argv[1],argv[2]);
copydir:
	status = 0;
	for(i=1;i<argc-1;++i)
		if(copy_dir(argv[i],argv[argc-1]))
			status = 1;
	return status;
printhowto:
	fprintf(stderr,"Usage: cp from-file-or-dir dest-name; or cp file1 ... fileN dest-dir\n");
	return 1;
}

int copy_dir(string_t from, string_t to){
	if(snprintf(buffer,sizeof buffer,"%s/%s",to,bnm(from))<1)return 1;
	return copy_object(from,buffer);
}

int copy_object(string_t from, string_t to){
	struct stat statbuf;
	if(stat(from,&statbuf)) return 1;
	if(S_ISDIR (statbuf.st_mode)) { printf("cp: directory ``%s'' was skipped\n",from); return 1; }
	if(S_ISSOCK(statbuf.st_mode)) { printf("cp: socket ``%s'' was skipped\n",from); return 1; }
	if(S_ISFIFO(statbuf.st_mode)) { printf("cp: named pipe ``%s'' was skipped\n",from); return 1; }
	if(S_ISBLK (statbuf.st_mode)) { printf("cp: block device ``%s'' was skipped\n",from); return 1; }
	if(S_ISCHR (statbuf.st_mode)) { printf("cp: char device ``%s'' was skipped\n",from); return 1; }
	if(S_ISLNK (statbuf.st_mode)) return copy_link(from,to);
	if(S_ISREG (statbuf.st_mode)) return copy_file(from,to);
	printf("cp: special file ``%s'' was skipped\n",from);
	return 1;
}

int copy_link(string_t from, string_t to){
	ssize_t len = readlink(from,buffer2,sizeof(buffer2)-1);
	if(len<0)return 1;
	buffer2[len]=0;
	return symlink(buffer2,to);
}

int copy_file(string_t from, string_t to){
	int n;
	int src = open(from,O_RDONLY);
	if(src<0)return 1;
	int dst = open(to,O_WRONLY|O_CREAT);
	if(dst<0){
		close(src);
		return 1;
	}
	for(;;){
		n = read(src,buffer,sizeof buffer);
		if(n<1)break;
		write(dst,buffer,n);
	}
	close(src);
	close(dst);
	return 0;
}


