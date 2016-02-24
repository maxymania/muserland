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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <grp.h>
#include <pwd.h>

typedef const char* string_t;
typedef const string_t* argv_t;
#define caseof(v,c) case v:c;break;


int gargc;
string_t* gargv;

string_t curdir;
char* buffer;

int opt_l=0;
int opt_all=0;
int opt_nogroup=0;
int opt_nouser=0;
int opt_stream = 0;
int opt_inode = 0;
int opt_ctime = 0;
int opt_atime = 0;
int opt_folloflink = 0;
int opt_ugid_asnum = 0;

static void
parse(int argc,argv_t argv){
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
			}
		}
	}
}

static char t2c(unsigned char t){
	#define to(a,b) case a:return b;
	switch(t){
		to(DT_BLK ,'b')
		to(DT_CHR ,'c')
		to(DT_DIR ,'d')
		to(DT_FIFO,'f')
		to(DT_LNK ,'l')
		to(DT_SOCK,'s')
	}
	#undef to
	return '-';
}

static string_t uidgid(struct stat *sb){
	struct group *gr;
	struct passwd *pw;
	int n=0;
	static char name[128];
	static char parts[1000];
	parts[0]=0;
	if(opt_ugid_asnum){
		if(!opt_nouser) n+=snprintf(parts,1000,"%6d ",(int)sb->st_uid);
		if(!opt_nogroup) snprintf(parts+n,1000+n,"%6d",(int)sb->st_gid);
	}else{
		if(!opt_nouser){
			pw = getpwuid(sb->st_uid);
			n+=snprintf(parts,1000,"%s ",pw?pw->pw_name:"");
		}
		if(!opt_nogroup){
			gr = getgrgid(sb->st_gid);
			snprintf(parts+n,1000+n,"%s",gr?gr->gr_name:"");
		}
	}
	return parts;
}
static string_t fdate(time_t t){
	struct tm *tm;
	static char parts[1000];
	tm = localtime(&t);
	strftime(parts,1000-1,"%G/%m/%d %H:%M:%S",tm);
	//snprintf(parts,1000,"%04d/%02d/%02d % 2d:%02d:%02d",(int)sb->st_uid,(int)sb->st_gid);
	return parts;
}
static string_t ino(struct stat *sb){
	static char parts[16];
	parts[0]=0;
	if(opt_inode) snprintf(parts,16,"%9d ",(int)sb->st_ino);
	return parts;
}

int isfirst = 0;
static void printout(struct dirent *de){
	struct stat sb;
	#define DOSTAT \
		sprintf(buffer,"%s/%s",curdir,de->d_name); \
		if(opt_folloflink)lstat(buffer,&sb);else stat(buffer,&sb);
	if(de->d_name[0]=='.'&&!opt_all)return;
	if(opt_l){
		DOSTAT
		printf(
			"%s%c%c%c%c%c%c%c%c%c%c %s % 9d %s %s\n",
			ino(&sb),
			t2c(de->d_type),
			sb.st_mode&0400?'x':'-',
			sb.st_mode&0200?'w':'-',
			sb.st_mode&0100?'r':'-',
			sb.st_mode&040?'x':'-',
			sb.st_mode&020?'w':'-',
			sb.st_mode&010?'r':'-',
			sb.st_mode&04?'x':'-',
			sb.st_mode&02?'w':'-',
			sb.st_mode&01?'r':'-',

			uidgid(&sb),

			(int)sb.st_size,

			fdate(opt_ctime?sb.st_ctime:opt_atime?sb.st_atime:sb.st_mtime),
			de->d_name);
		return;
	}
	if(opt_stream){
		if(opt_inode){
			DOSTAT
			if(isfirst) printf(", %d %s",(int)sb.st_ino,de->d_name);
			else printf("%d %s",(int)sb.st_ino,de->d_name);
		}else{
			if(isfirst) printf(", %s",de->d_name);
			else printf("%s",de->d_name);
		}
		isfirst = 1;
		return;
	}
	if(opt_inode){DOSTAT}
	printf("%s%s\n",ino(&sb),de->d_name);
}

static void list(string_t s){
	curdir = s;
	struct dirent *de;
	DIR* d = opendir(s);
	if(!d)return;
	while(de=readdir(d))
		printout(de);
	if(opt_stream)printf("\n");
}

int main(int argc,argv_t argv){
	buffer = malloc(1<<13);
	int i;
	parse(argc-1,argv+1);
	if(gargc<1)
		list(".");
	else for(i=0;i<gargc;++i){
		printf("%s:\n",gargv[i]);
		list(gargv[i]);
	}
	return 0;
}
// -EOF


