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

#define BMASK_USER  05700
#define BMASK_GROUP 02070
#define BMASK_OTHER 00007
/* All users, except setuid bit */
#define BMASK_ALL   01777

#define BIT_READ   00444
#define BIT_WRITE  00222
#define BIT_EXEC   00111
#define BIT_SETID  06000
#define BIT_STICKY 01000

typedef const char* string_t;

static void donothing(){}

static int where=0;
static int what=0;
static int op='=';
#define caseof(v,c) case v:c;break;

int main(int argc,const string_t* argv){
	int octal = 0,i;
	struct stat stb;
	string_t mode;
	if(argc<3){
		printf("Usage: chmod 0642 files...; or chmod [uoga][+-][rwxstugo] files..\n");
		return 1;
	}
	mode = argv[1];
	if(*mode>='0' && *mode<='7'){
		while(*mode>='0' && *mode<='7'){
			where <<= 3; what <<= 3;
			where |= 7;
			what  |= (*mode++)-'0';
		}
		where &= 07777;
		what  &= 07777;
	}else{
		for(;;)switch(*mode++){
		caseof('u',where|=BMASK_USER)
		caseof('g',where|=BMASK_GROUP)
		caseof('o',where|=BMASK_OTHER)
		caseof('a',where|=BMASK_ALL)
		case '+':
			op = '+';
			goto rest_of;
		case '-':
			op = '-';
			goto rest_of;
		case '=':
			op = '=';
			goto rest_of;
		case 0:
			goto rest_of;
		}
		rest_of:
		for(;;)switch(*mode++){
		caseof('r',what|=BIT_READ)
		caseof('w',what|=BIT_WRITE)
		caseof('x',what|=BIT_EXEC)
		caseof('s',what|=BIT_SETID)
		caseof('t',what|=BIT_STICKY)
		case 0: goto jmpout;
		}
		jmpout:
		/* Keep the compiler happy! */
		donothing();
	}
	for(i=0;i<argc;++i){
		stat(argv[i],&stb);
		switch(op){
		caseof('+', stb.st_mode |= (what&where))
		caseof('-', stb.st_mode &= ~(what&where))
		caseof('=', stb.st_mode &= ~where;
					stb.st_mode |= (what&where))
		default:continue;
		}
		chmod(argv[i],stb.st_mode);
	}
	return 0;
}


