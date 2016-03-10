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
#include <libgen.h>
typedef const char* string_t;
#define BASDTR (char*)

static int opt_s=0;


int main(int argc,const string_t* argv){
	int i;
	string_t target=NULL,slink=NULL;
	string_t arg;
	for(i=1;i<argc;++i){
		if(argv[i][0]=='-'){
			for(arg = argv[i]+1;*arg;++arg)
			switch(*arg){
				case 's':opt_s = 1;
			}
			continue;
		}
		if(target)	slink	= argv[i];
		else		target	= argv[i];
	}
	if(!target)	return 1;
	if(!slink)	slink = basename(BASDTR target);
	if(!slink)	return 1;
	if(opt_s)	symlink(target,slink);
	else		link(target,slink);
	return 0;
}


