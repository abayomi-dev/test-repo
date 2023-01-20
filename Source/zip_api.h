#ifndef _ZIP_API_H_
#define _ZIP_API_H_

#define NAME_SIZE 16

typedef struct 
{
    char filename[NAME_SIZE + 1];//文件名
    int size;//解压后的文件大小
}ZFILEINFO;

/*----------------------------
 *创建zip文件，如果存在，将被替换
 *filename:文件名
 *返回<0失败，>=0 句柄索引；
---------------------------------*/
int CreateZip(const char* filename);

/*----------------------------
 *添加文件到zip文件；
 *fid：CreateZip返回的文件句柄索引；
 *filename:文件名
 *返回 <0 失败；0成功
---------------------------------*/
int AddFiletoZip(int fid, char *filename);

/*----------------------------
 *关闭zip文件；
 *fid：CreateZip返回的文件句柄索引；
 *返回 <0 失败；0成功
---------------------------------*/
int CloseZip(int fid, const char* global_comment);

/*----------------------------
 *解压zip文件；
 *z_filename: zip文件名
 *返回 <0 失败；0成功
 *如果zip文件存在目录，只会将文件解压；不会产生相对于的目录结构
---------------------------------*/
int Unzip(const char *z_filename);

/*----------------------------
 *获取zip文件指定文件名解压后的内容及大小；
 * z_filename: zip文件名
 * i_filename:索引文件名
 * buf缓冲区
 * size：缓冲区大小
 * >=0返回实际的数据大小,<0失败
 * 搜索所有zip内的目录的文件， 匹配第一个符合的文件。
---------------------------------*/
int UnzipFileByName(const char *z_filename, const char *i_filename,
					unsigned char *buf, int size);

/*----------------------------
 *获取zip文件内的文件信息结构；
 * z_filename: zip文件名
 * f_info:文件信息缓冲区
 * n:预获取的f_info的数目
 *返回<0失败，>=0获取到的文件信息数目；
 * 搜索所有zip内包括目录里的文件。
---------------------------------*/
int ZipFileInfo(const char *z_filename, ZFILEINFO *f_info, int n);

int CompressData(unsigned char *out, unsigned int *outLen,
			 const unsigned char *in, unsigned int inLen);

int DecompressData(unsigned char *out, unsigned int *outLen,
			 const unsigned char *in, unsigned int inLen);
#endif
