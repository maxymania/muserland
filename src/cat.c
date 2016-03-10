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
typedef const char* string_t;

FILE* src;

void copy(){
	int i;
	while((i=fgetc(src))!=EOF) putchar(i);
}

int main(int argc,const string_t* argv){
	int i,err;
	src = stdin;
	if(argc==1){
		copy();
		return 0;
	}
	err = 0;
	for(i=1;i<argc;++i){
		if((argv[i][0]=='-')&&(argv[i][1]==0))src = stdin;
		else src = fopen(argv[i],"rb");
		if(!src){
			err = 1;
			perror("cat");
			continue;
		}
		copy();
		if(src!=stdin)fclose(src);
	}
	return err;
}


