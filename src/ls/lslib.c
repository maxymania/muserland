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
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <grp.h>
#include <pwd.h>
#define LS_NO_EXTERN
#include "./lslib.h"

int gargc;
string_t* gargv;
static string_t curdir;
static char* buffer;

static int opt_l=0;
static int opt_all=0;
static int opt_nogroup=0;
static int opt_nouser=0;
static int opt_stream = 0;
static int opt_inode = 0;
static int opt_ctime = 0;
static int opt_atime = 0;
static int opt_folloflink = 0;
static int opt_ugid_asnum = 0;
static int opt_reverse = 0;
static int opt_p = 0;

#define NFILES 1024

struct dirbuf *flist[NFILES];
struct dirbuf **begin,**last,**end = &flist[NFILES];

#define NCACHE 128

struct grpinfo{
	id_t id;
	const char* name;
};

static struct grpinfo grpcache[NCACHE];
static struct grpinfo pwdcache[NCACHE];

static int next_p,next_g;

static const char* uid2n(uid_t uid){
	if(uid==0) return "root"; // TODO: getpwuid for that;
	int i;
	for(i=0;i<NCACHE;++i)
		if(pwdcache[i].id==uid)return pwdcache[i].name;
	struct passwd *pw = getpwuid(uid);
	i = next_p;
	next_p = (next_p+1)%NCACHE;
	pwdcache[i].id=uid;
	if(pw){
		char* name = malloc(strlen(pw->pw_name));
		strcpy(name,pw->pw_name);
		pwdcache[i].name = name;
	}else{
		pwdcache[i].name = NULL;
	}
	return pwdcache[i].name;
}

static const char* gid2n(uid_t gid){
	if(gid==0) return "root"; // TODO: getgrgid for that;
	int i;
	for(i=0;i<NCACHE;++i)
		if(grpcache[i].id==gid)return grpcache[i].name;
	struct group *gr = getgrgid(gid);
	i = next_g;
	next_g = (next_g+1)%NCACHE;
	grpcache[i].id=gid;
	if(gr){
		char* name = malloc(strlen(gr->gr_name));
		strcpy(name,gr->gr_name);
		grpcache[i].name = name;
	}else{
		grpcache[i].name = NULL;
	}
	return grpcache[i].name;
}

#define caseof(v,c) case v:c;break;

static inline void* pupk(const void*a){
	typedef void* ptr_t;
	return *((const ptr_t*)a);
}

static int compare_name (const void * a, const void * b)
{
	struct dirbuf *A=pupk(a),*B=pupk(b);
	return strcmp(A->dirent.d_name,B->dirent.d_name);
}

#define RETCOMP(a,b) if(a<b)return -1; if(a>b)return 1; return 0;
static int compare_mtime (const void * a, const void * b)
{
	struct dirbuf *A=pupk(a),*B=pupk(b);
	if(opt_ctime){
		RETCOMP(A->stat.st_ctime,B->stat.st_ctime);
	}
	if(opt_atime){
		RETCOMP(A->stat.st_atime,B->stat.st_atime);
	}
	RETCOMP(A->stat.st_mtime,B->stat.st_mtime);
}

static int (*comparator) (const void * a, const void * b);

static inline void reverse(void** a,int n){
	void* temp;
	int b=0,e=n-1;
	for(;b<e;b++,e--){
		temp=a[b];
		a[b]=a[e];
		a[e]=temp;
	}
}

void parse(int argc,argv_t argv)
{
	gargc = 0;
	gargv = malloc(argc*sizeof(string_t));
	string_t cur;
	while(argc>0){
		if(**argv!='-'){
			gargv[gargc++]=*argv++;argc--;
			continue;
		}
		cur = *argv++;argc--;
		for(cur++;*cur;cur++){
			switch(*cur){
				caseof('a',opt_all = 1)
				caseof('l',opt_l = 1)
				caseof('o',opt_nogroup = 1; opt_l = 1)
				caseof('g',opt_nouser = 1; opt_l = 1)
				caseof('n',opt_ugid_asnum = 1; opt_l = 1)
				caseof('m',opt_stream = 1)
				caseof('i',opt_inode = 1)
				caseof('c',opt_ctime = 1)
				caseof('u',opt_atime = 1)
				caseof('H',opt_folloflink = 1)
				caseof('r',opt_reverse = 1)
				caseof('t',comparator = compare_mtime)
				caseof('p',opt_p = 1)
			}
		}
	}
}

static char mode2c(mode_t t){
	#define to(a,b) case a:return b;
	switch(t&S_IFMT){
		to(S_IFBLK ,'b')
		to(S_IFCHR ,'c')
		to(S_IFDIR ,'d')
		to(S_IFLNK ,'l')
	}
	#undef to
	if(S_ISFIFO(t))return 'f';
	if(S_ISSOCK(t))return 's';
	return '-';
}

static const char* mode2slash(mode_t t){
	if(opt_p && S_ISDIR(t)) return "/";
	return "";
}

static string_t uidgid(struct stat *sb){
	struct group *gr;
	struct passwd *pw;
	const char* name;
	int n=0;
	static char parts[1000];
	parts[0]=0;
	if(opt_ugid_asnum){
		if(!opt_nouser) n+=snprintf(parts,1000,"%6d ",(int)sb->st_uid);
		if(!opt_nogroup) snprintf(parts+n,1000-n,"%6d",(int)sb->st_gid);
	}else{
		if(!opt_nouser){
			name = uid2n(sb->st_uid);
			n+=snprintf(parts,1000,"%s ",name?name:"");
		}
		if(!opt_nogroup){
			name = gid2n(sb->st_gid);
			snprintf(parts+n,1000-n,"%s",name?name:"");
		}
	}
	return parts;
}
static string_t fdate(time_t t){
	struct tm *tm;
	static char parts[1000];
	tm = localtime(&t);
	strftime(parts,1000-1,"%G/%m/%d %H:%M:%S",tm);
	return parts;
}
static string_t ino(struct stat *sb){
	static char parts[16];
	parts[0]=0;
	if(opt_inode) snprintf(parts,16,"%9d ",(int)sb->st_ino);
	return parts;
}

static int isfirst = 0;
static void printout(struct dirbuf *buf){
	struct stat sb;
	if(buf->dirent.d_name[0]=='.'&&!opt_all)return;
	if(opt_l){
		printf(
			"%s%c%c%c%c%c%c%c%c%c%c %s % 9d %s %s%s\n",
			ino(&(buf->stat)),
			mode2c(buf->stat.st_mode),
			buf->stat.st_mode&0400?'x':'-',
			buf->stat.st_mode&0200?'w':'-',
			buf->stat.st_mode&0100?'r':'-',
			buf->stat.st_mode&040?'x':'-',
			buf->stat.st_mode&020?'w':'-',
			buf->stat.st_mode&010?'r':'-',
			buf->stat.st_mode&04?'x':'-',
			buf->stat.st_mode&02?'w':'-',
			buf->stat.st_mode&01?'r':'-',

			uidgid(&(buf->stat)),

			(int)buf->stat.st_size,

			fdate(opt_ctime?buf->stat.st_ctime:opt_atime?buf->stat.st_atime:buf->stat.st_mtime),
			buf->dirent.d_name,
			mode2slash(buf->stat.st_mode));
		return;
	}
	if(opt_stream){
		if(opt_inode){
			if(isfirst) printf(", %d %s%s",(int)buf->stat.st_ino,buf->dirent.d_name,mode2slash(buf->stat.st_mode));
			else printf("%d %s%s",(int)buf->stat.st_ino,buf->dirent.d_name,mode2slash(buf->stat.st_mode));
		}else{
			if(isfirst) printf(", %s%s",buf->dirent.d_name,mode2slash(buf->stat.st_mode));
			else printf("%s%s",buf->dirent.d_name,mode2slash(buf->stat.st_mode));
		}
		isfirst = 1;
		return;
	}
	printf("%s%s%s\n",ino(&(buf->stat)),buf->dirent.d_name,mode2slash(buf->stat.st_mode));
}

static void setdentry(struct dirbuf **slot,struct dirent *de){
	if(!*slot)
		*slot = malloc(sizeof(struct dirbuf));
	if(!*slot)exit(1);
	(*slot)->dirent = *de;
	if(opt_l|opt_inode|opt_p){
		sprintf(buffer,"%s/%s",curdir,(*slot)->dirent.d_name);
		if(opt_folloflink)
			lstat(buffer,&((*slot)->stat));
		else stat(buffer,&((*slot)->stat));
	}
}

void list(string_t s){
	curdir = s;
	struct dirent *de;
	int n;
	DIR* d = opendir(s);
	if(!d)return;
	do{
		begin=last=flist;
		n=0;
		while(de=readdir(d)){
			setdentry(last,de);
			last++;
			n++;
			if(last==end)break;
		}
		qsort(begin,n,sizeof(void*),comparator);
		if(opt_reverse) reverse((void**)begin,n);
		for(;begin<last;begin++)
			printout(*begin);
	}while(de);
	if(opt_stream)printf("\n");
}

void lsinit(){
	struct grpinfo cinit = {0,0};
	comparator = compare_name;
	int i;
	buffer = malloc(1<<13);
	for(i=0;i<NFILES;++i)
		flist[i]=NULL;
	for(i=0;i<NCACHE;++i){
		grpcache[i] = cinit;
		pwdcache[i] = cinit;
	}
}
// -EOF

