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
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <fcntl.h>
#include <unistd.h>
typedef const char* string_t;

#define MAXF 64000
static string_t files[MAXF];
int nfiles=0;

#define caseof(v,c) case v:c;break;

static inline int idig(char c){
	return (c>='0') && (c<='9');
}
static inline int ddig(char c){
	return c-'0';
}

static int scanInts(string_t str,int* v,string_t *dstr){
	int i=0;
	for(;;){
		if(i>=6)break;
		if(!idig(str[0]))break;
		if(!idig(str[1]))break;
		v[i] = (ddig(str[0])*10) + ddig(str[1]);
		i+=1;
		str+=2;
	}
	*dstr = str;
	return i;
}
static int parseSec(string_t sec){
	if(
		(sec[0]=='.') &&
		idig(sec[1]) &&
		idig(sec[2]) )
		return (ddig(sec[1])*10) + ddig(sec[2]);
	return 0;
}

static int parseDate(string_t str,struct tm *t){
	int i,n;
	int date[6];
	string_t sec;
	n = scanInts(str,date,&sec);
	if(n<4){
		if(!strptime(str, "%Y-%m-%d %H:%M:%S", t))
			return 1;
	}else{
		i=0;
		if(n==6){
			t->tm_year = (date[0]*100) + date[1];
			i=2;
		} else if(n==5){
			t->tm_year = date[0]+1900;
			if(t->tm_year < 1969)
				t->tm_year += 100;
			i=1;
		}
		else t->tm_year = 2000; /* bogus! */
		t->tm_mon = date[i++];
		t->tm_mday = date[i++];
		t->tm_hour = date[i++];
		t->tm_min = date[i++];
		t->tm_sec = parseSec(sec);
	}
	return 0;
}

#define caseof(v,c) case v:c;break;

int main(int argc,const string_t* argv){
	int i,opt_m=0,opt_a=0,res;
	struct utimbuf targ,*ptarg=NULL;
	struct tm tm;
	struct stat stb;
	string_t opts;
	for(i=1;i<argc;++i){
		if(*(argv[i])!='-'){
			files[nfiles++]=argv[i];
			continue;
		}
		opts = argv[i]+1;
		for(;;){
			if(!*opts)break;
			switch(*opts++){
			case't':
				if(parseDate(argv[++i],&tm)) return 1;
				targ.actime = targ.modtime= mktime(&tm);
				ptarg = &targ;
				break;
			case 'r':
				if(stat(argv[++i],&stb)) return 1;
				targ.actime  = stb.st_atime;
				targ.modtime = stb.st_mtime;
				ptarg = &targ;
				break;
			caseof('m',opt_m=1)
			caseof('a',opt_a=1)
			}
		}
	}
	if(opt_m||opt_a)
		if(!ptarg){
			targ.actime = targ.modtime = time(NULL);
			ptarg = &targ;
		}
	for(i=0;i<nfiles;++i){
		res = stat(files[i],&stb);
		if(opt_m||opt_a) if(res) continue;
		if(opt_m)
			targ.actime  = stb.st_atime;
		if(opt_a)
			targ.modtime = stb.st_mtime;
		if(res) {
			res = open(files[i],O_CREAT|O_WRONLY,0644);
			if(res>=0)close(res);
		}
		utime(files[i],ptarg);
	}
	return 0;
}


