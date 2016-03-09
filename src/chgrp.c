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
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <stdlib.h>
#include <unistd.h>
typedef const char* string_t;

int chknm(gid_t *gid,string_t str){
	int i;
	if(!*str)return 0;
	for(;;){
		i=*str++;
		if(!i)return 1;
		i-='0';
		if(i<0||i>9)return 0;
		*gid = ((*gid)*10)+i;
	}
	/* not reached */
	return 0;
}

int main(int argc,const string_t* argv){
	struct group *gr;
	struct stat stb;
	gid_t gid;
	int i,r;
	if(argc<3){
		printf("usage: chgrp gid/group file ...\n");
	}
	if(!chknm(&gid,argv[1])){
		gr=getgrnam(argv[1]);
		if(!gr){
			printf("unknown group: %s\n",argv[1]);
			return 1;
		}
		gid = gr->gr_gid;
	}
	r = 0;
	for(i = 2 ; i < argc ; ++i){
		stat(argv[i],&stb);
		if(chown(argv[i],stb.st_uid,gid))r = 1;
	}
	return(r);
}


