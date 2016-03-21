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
#include <fcntl.h>
#include <unistd.h>
typedef const char* string_t;

void nothing(){};

int deNum(string_t s){
	int i=0;
	reloop:
	if(*s<'0')return -1;
	if(*s>'9')return -1;
	i*=10;
	i+=(*s)-'0';
	if(!*s)return i;
	goto reloop;
}
#define caseof(v,c) case v:c;break;
#define FA |0666

int main(int argc,const string_t* argv){
	int a,b;
	mode_t mode;
	dev_t  dev;
	if(argc!=5)goto usage;
	switch(*(argv[2])){
	caseof('b',mode = S_IFBLK FA)
	caseof('c',mode = S_IFCHR FA)
	caseof('p',mode = S_IFIFO FA)
	caseof('-',mode = S_IFREG FA)
	default:
		/* keep compiler happy! */
		nothing();
		goto usage;
	}
	a = deNum(argv[3]);
	b = deNum(argv[4]);
	if((a<0)||(b<0))goto usage;
	dev = (a<<8)|b;
	if(mknod(argv[1],mode,dev)<0)
		perror("mknod");
	return 0;
usage:
	printf("Usage: mknod filename b/c major minor\n");
	return 1;
}


