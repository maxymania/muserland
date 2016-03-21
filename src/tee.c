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
#include <unistd.h>
#include <fcntl.h>
typedef const char* string_t;
char buffer[512];
static void blokcp(int* files,int nfiles){
	int n,i;
	for(;;){
		n = read(0,buffer,512);
		if(n<1)break;
		for(i=0;i<nfiles;++i)
			write(files[i],buffer,n);
	}
}

#define caseof(v,c) case v:c;break;

int main(int argc,const string_t* argv){
	int i=1,opt_a=0,opt_i=0,flags = O_WRONLY|O_CREAT|O_TRUNC;
	string_t opt;
	int dfiles[argc];
	int nfiles=1;
	
	if(argc>1 && argv[1][0]=='-'){
		opt = argv[1]+1;
		while(*opt)
		switch(*opt++){
		caseof('a',opt_a=1)
		caseof('i',opt_i=1)
		}
		i=2;
	}
	if(opt_a)
		flags = O_WRONLY|O_CREAT|O_APPEND;
	if(opt_i)
		;
	dfiles[0]=1;
	for(;i<argc;++i){
		dfiles[nfiles++] = open(argv[i],flags,0644);
	}
	blokcp(dfiles,nfiles);
	return 0;
}


