/*
1输出elf_header中入口地址，文件属性，段表在文件中的偏移，段表数量，段表每一项大小,段表字符串表在段表中的下标等信息。
2 输出段表中每个段的名称，大小，偏移，段虚拟地址，段的类型等信息
3 输出段表字符串表中的内容
4 输出字符串表中的内容
5 输出符号表中的信息，符号名，符号大小，符号所在段，符号对应的值等信息

假设hello hello2是已经链接过的可执行文件
readelf是自己写的程序
./readelf hello解析hello
./readelf hello2解析hello2

*/
#include <elf.h>
#include <iostream> /*文件流不使用FILE？？*/
#include <fstream>
#include <iomanip>//setw(),用来设置输出字符串的宽度
//#include <string>//使用 std::string 动态定义字符串
#include <cstring>//strcmp
#include <cstdint>
#include <cstdio>  /*用于提供c风格的输入和输出操作，printf,sprintf snprintf,scanf,fscanf,sscanf,
fopen,fclose,fwrite,fread,getchar,putchar,fgets,fputs,FILE*/
using namespace std;

//1输出elf_header中入口地址，文件属性，段表在文件中的偏移，段表数量，段表每一项大小,段表字符串表在段表中的下标等信息。
//readelf -h hello >readelf-hhello.txt
int readEhdr(FILE *fp){

    // 解析head
	Elf64_Ehdr elf_head;
    int a;//读取文件数据是否成功
 
	// 读取ELF Header到elf_head中
	a = fread(&elf_head, sizeof(Elf64_Ehdr), 1, fp);
    /*size_t fread(void *ptr, size_t size, size_t count, FILE *stream);
    它的作用是从指定的文件流 (stream) 中读取数据，将读取到的数据存储到 ptr 指向的内存位置。参数说明如下：
    ptr：指向存储读取数据的内存块的指针。
    size：每个数据项的大小（以字节为单位）。
    count：要读取的数据项的个数。
    stream：指定的文件流。*/

    if (a==0)
	{
		cout<<"读取文件头失败"<<endl;
		return 0;
	}
 
	// 判断ELF文件类型，是不是ELF文件
	if (elf_head.e_ident[0] != 0x7F ||
		elf_head.e_ident[1] != 'E' ||
		elf_head.e_ident[2] != 'L' ||
		elf_head.e_ident[3] != 'F')
	{
		cout<<"Not a ELF file"<<endl;
		return 0;
	}

    cout<<"________________________________________________________"<<endl;
    cout<<"| "<<"ELF文件头："<<endl;
    cout<<"|________________________________________________________"<<endl;
    cout<<"| "<<hex<<showbase<<"入口地址："<<elf_head.e_entry<<endl;//转换成16进制，并且以0x开始

    switch (elf_head.e_type)
    {
    case 1:
        cout<<"| "<<"文件类型：REL（可重定位文件）"<<endl;
        break;
    case 2:
        cout<<"| "<<"文件类型:EXEC（可执行文件）"<<endl;
        break;
    case 3:
        cout<<"| "<<"文件类型：DYN（共享目标文件）"<<endl;
        break; 
    default:
        break;
    }
    cout<<"| "<<dec<<"段表在文件中的偏移："<<elf_head.e_shoff<<"(bytes into file)"<<endl;
    cout<<"| "<<"段表数量："<<elf_head.e_shnum<<endl;
    cout<<"| "<<"段表每一项的大小："<<elf_head.e_shentsize<<"(bytes)"<<endl;
    cout<<"| "<<"段表字符串在段表中的下标："<<elf_head.e_shstrndx<<endl;
    cout<<"| "<<"EIL文件头本身大小："<<elf_head.e_ehsize<<"(bytes)"<<endl;
    cout<<"|________________________________________________________"<<endl;
    return 0;
}



//读取段表字符串表下标为p开头内容，以为\0结尾
char * readshname(FILE *fp,int p){
    Elf64_Ehdr elf_head;
    int a{};
	// 读取ELF Header到elf_head中
	fread(&elf_head, sizeof(Elf64_Ehdr), 1, fp);

    //定义一个指向Elf64_Shdr类型的指针，也就是说elf_shdr是一个有elf_head.e_shnum个Elf64_Shdr类型的数组
    Elf64_Shdr *elf_shdr=(Elf64_Shdr*)malloc(sizeof(Elf64_Shdr)*elf_head.e_shnum);
    //设置文件偏移量，定位到e_shoff，段表在文件中的偏移，根据ELF文件头得到
    a = fseek(fp, elf_head.e_shoff, SEEK_SET);  
    //读取所有的段表内容到elf_shdr中，大小为sizeof(sizeof(Elf64_Shdr)*elf_head.e_shnum)
    a=fread(elf_shdr,sizeof(Elf64_Shdr)*elf_head.e_shnum,1,fp);
    //重置指针位置到文件头
    rewind(fp);

    //定位到段表字符串表，头文件中有个shstrndx是段表字符串表在段表中的下标，解读段表该数组，有地址和大小，就可以得到段表字符串表的内容。
    char *elf_shstrtab=(char*)malloc(sizeof(char)*elf_shdr[elf_head.e_shstrndx].sh_size);
    //定位到段表字符串表的地址

    //将段表字符串表的内容读到elf_shstrtab数组中
    a=fseek(fp,elf_shdr[elf_head.e_shstrndx].sh_offset,SEEK_SET);//这里应该是偏移量，而不是addr，那个是可以被加载之后
    a=fread(elf_shstrtab,elf_shdr[elf_head.e_shstrndx].sh_size,1,fp);
    rewind(fp);

    char *shname=(char*)malloc(sizeof(char)*1);
    int i=0;

    while(elf_shstrtab[p]!='\0'){
        shname[i++]=elf_shstrtab[p++];
        shname=(char*)realloc(shname,(i+1)*sizeof(char));//后面的是重新分配之后的所有的大小
    }
    shname=(char*)realloc(shname,(i+1)*sizeof(char));//后面的是重新分配之后的所有的大小
    shname[i]='\0';
   // cout<<shname<<endl;

    return shname;

}

//2 输出段表中每个段的名称，大小，偏移，段虚拟地址，段的类型等信息
//readelf -S hello >readelf-Shello.txt
int readShdr(FILE *fp){

    Elf64_Ehdr elf_head;
    int a{};
	// 读取ELF Header到elf_head中
	fread(&elf_head, sizeof(Elf64_Ehdr), 1, fp);

    //定义一个指向Elf64_Shdr类型的指针，也就是说elf_shdr是一个有elf_head.e_shnum个Elf64_Shdr类型的数组
    Elf64_Shdr *elf_shdr=(Elf64_Shdr*)malloc(sizeof(Elf64_Shdr)*elf_head.e_shnum);

    //设置文件偏移量，定位到e_shoff，段表在文件中的偏移，根据ELF文件头得到
    a = fseek(fp, elf_head.e_shoff, SEEK_SET);  
    /*int fseek(FILE *stream, long offset, int whence);将文件位置指针移动到指定位置。
    参数解释：
    stream：指向 FILE 结构体的指针，代表要设置位置指针的文件流。
    offset：偏移量，它表示相对于 whence 参数的偏移。可以是正值、负值或零。
    whence：用于确定 offset 如何解释的整数值。可能的值有：
        SEEK_SET：从文件开头开始偏移。
        SEEK_CUR：从当前位置开始偏移。
        SEEK_END：从文件末尾开始偏移。*/

    //读取所有的段表内容到elf_shdr中，代谢奥为sizeof(sizeof(Elf64_Shdr)*elf_head.e_shnum)
    a=fread(elf_shdr,sizeof(Elf64_Shdr)*elf_head.e_shnum,1,fp);



    //重置指针位置到文件头
    rewind(fp);
    cout<<"___________________________________________________________________________________________"<<endl;
    cout<<"| "<<"段表："<<endl;
    cout<<"|__________________________________________________________________________________________"<<endl;
    cout<<"| [Nr]"<<setw(30)<<"Name"<<setw(15)<<"Type"<<setw(10)<<"Addr"<<setw(10)<<"Off"<<setw(10)<<"Size"<<endl;
    //遍历每一个Section Header
    for(int i=0;i<elf_head.e_shnum;i++)
    {
        cout<<dec;
        //elf_shdr[i].sh_name需要用到段表字符串表
        cout<<"|["<<i<<"]"<<setw(30)<<readshname(fp,elf_shdr[i].sh_name)<<setw(15);

        switch (elf_shdr[i].sh_type)
        {
        case 0:
            cout<<"null";break;
        case 1:
            cout<<"progbits";break;
        case 2:
            cout<<"symtab";break;
        case 3:
            cout<<"strtab";break;
        case 4:
            cout<<"rela";break;
        case 5:
            cout<<"hash";break;
        case 6:
            cout<<"dynamic";break;
        case 7:
            cout<<"note";break;
        case 8:
            cout<<"nobits";break;            
        case 9:
            cout<<"rel";break;            
        case 10:
            cout<<"shlib";break;            
        case 11:
            cout<<"dnysym";break;            

        default:
            cout<<elf_shdr[i].sh_type;//和书上的不一样，书上有缺省
            break;
        }
        cout<<hex<<setw(10)<<elf_shdr[i].sh_addr;
        cout<<setw(10)<<elf_shdr[i].sh_offset<<setw(10)<<elf_shdr[i].sh_size<<endl;

    }
    cout<<"|__________________________________________________________________________________________"<<endl;

    return 0;
}


//3 输出段表字符串表中的内容。elf文件中引用字符串只需给出一个数字的下标，以为'\0'结构就是没了
//readelf -x .shstrtab hello > readelf-x.shstrtab.txt
int readShstrtab(FILE *fp)
{
    Elf64_Ehdr elf_head;
    int a{};
	// 读取ELF Header到elf_head中
	fread(&elf_head, sizeof(Elf64_Ehdr), 1, fp);

    //定义一个指向Elf64_Shdr类型的指针，也就是说elf_shdr是一个有elf_head.e_shnum个Elf64_Shdr类型的数组
    Elf64_Shdr *elf_shdr=(Elf64_Shdr*)malloc(sizeof(Elf64_Shdr)*elf_head.e_shnum);
    //设置文件偏移量，定位到e_shoff，段表在文件中的偏移，根据ELF文件头得到
    a = fseek(fp, elf_head.e_shoff, SEEK_SET);  
    //读取所有的段表内容到elf_shdr中，大小为sizeof(sizeof(Elf64_Shdr)*elf_head.e_shnum)
    a=fread(elf_shdr,sizeof(Elf64_Shdr)*elf_head.e_shnum,1,fp);
    //重置指针位置到文件头
    rewind(fp);

    //定位到段表字符串表，头文件中有个shstrndx是段表字符串表在段表中的下标，解读段表该数组，有地址和大小，就可以得到段表字符串表的内容。
    char *elf_shstrtab=(char*)malloc(sizeof(char)*elf_shdr[elf_head.e_shstrndx].sh_size);
    //定位到段表字符串表的地址

    a=fseek(fp,elf_shdr[elf_head.e_shstrndx].sh_offset,SEEK_SET);//这里应该是偏移量，而不是addr，那个是可以被加载之后
    a=fread(elf_shstrtab,elf_shdr[elf_head.e_shstrndx].sh_size,1,fp);
    rewind(fp);

    cout<<"________________________________________________________________________________________"<<endl;
    cout<<"| "<<"段表字符串表："<<endl;
    cout<<"|_______________________________________________________________________________________"<<endl;
    cout<<"|偏移"<<setw(8)<<"+0"<<setw(8)<<"+1"<<setw(8)<<"+2"<<setw(8)<<"+3"<<setw(8)<<"+4"<<setw(8)<<"+5"<<setw(8)<<"+6"
    <<setw(8)<<"+7"<<setw(8)<<"+8"<<setw(8)<<"+9"<<endl;
    cout<<"|+00";
    for(int i=0;i<elf_shdr[elf_head.e_shstrndx].sh_size;i++){
        if((i+1)%10==0){
            cout<<endl<<"|+"<<i+1;
        }
        cout<<setw(8)<<elf_shstrtab[i];
    }

    cout<<endl<<"________________________________________________________________________________________"<<endl;
    free(elf_shdr);
    free(elf_shstrtab);

    return 0;
}


//4 输出字符串表中的内容
//readelf -x .strtab hello > readelf-x.strtab.txt
int readStrtab(FILE *fp)
{
    Elf64_Ehdr elf_head;
    int a{};

	// 读取ELF Header到elf_head中
	a=fread(&elf_head, sizeof(Elf64_Ehdr), 1, fp);


    //读取ELH Section Header 到elf_shdr
    //定义一个指向Elf64_Shdr类型的指针，也就是说elf_shdr是一个有elf_head.e_shnum个Elf64_Shdr类型的数组
    Elf64_Shdr *elf_shdr=(Elf64_Shdr*)malloc(sizeof(Elf64_Shdr)*elf_head.e_shnum);
    //设置文件偏移量，定位到e_shoff，段表在文件中的偏移，根据ELF文件头得到
    a = fseek(fp, elf_head.e_shoff, SEEK_SET);  
    //读取所有的段表内容到elf_shdr中，大小为sizeof(sizeof(Elf64_Shdr)*elf_head.e_shnum)
    a=fread(elf_shdr,sizeof(Elf64_Shdr)*elf_head.e_shnum,1,fp);
    //重置指针位置到文件头
    rewind(fp);

    //找到字符串表的下标
    int strndx{};//字符串表的下标
    for(int i=1;i<elf_head.e_shnum;i++){
        if(strcmp(readshname(fp,elf_shdr[i].sh_name),".strtab")==0){
            strndx=i;
        }
    }
    cout<<strndx<<endl;

    //定位到字符串表
    char *elf_strtab=(char*)malloc(sizeof(char)*elf_shdr[strndx].sh_size);
    a=fseek(fp,elf_shdr[strndx].sh_offset,SEEK_SET);//这里应该是偏移量，而不是addr，addr是可以被加载之后
    a=fread(elf_strtab,elf_shdr[strndx].sh_size,1,fp);
    rewind(fp);

    cout<<"________________________________________________________________________________________"<<endl;
    cout<<"| "<<"字符串表："<<endl;
    cout<<"|_______________________________________________________________________________________"<<endl;
    cout<<"|偏移"<<setw(8)<<"+0"<<setw(8)<<"+1"<<setw(8)<<"+2"<<setw(8)<<"+3"<<setw(8)<<"+4"<<setw(8)<<"+5"<<setw(8)<<"+6"
    <<setw(8)<<"+7"<<setw(8)<<"+8"<<setw(8)<<"+9"<<endl;
    cout<<"|+00";
    for(int i=0;i<elf_shdr[strndx].sh_size;i++){
        if((i+1)%10==0){
            cout<<endl<<"|+"<<i+1;
        }
        cout<<setw(8)<<elf_strtab[i];
    }

    cout<<endl<<"|_______________________________________________________________________________________"<<endl;

    free(elf_shdr);
    free(elf_strtab);
    return 0;
}



//读取字符串表下标为p开头内容，以为\0结尾的内容。这个是符号表中符号名需要用到的。
char * readsymname(FILE *fp,int p){
    Elf64_Ehdr elf_head;
    int a{};

	// 读取ELF Header到elf_head中
	a=fread(&elf_head, sizeof(Elf64_Ehdr), 1, fp);


    //读取ELH Section Header 到elf_shdr
    //定义一个指向Elf64_Shdr类型的指针，也就是说elf_shdr是一个有elf_head.e_shnum个Elf64_Shdr类型的数组
    Elf64_Shdr *elf_shdr=(Elf64_Shdr*)malloc(sizeof(Elf64_Shdr)*elf_head.e_shnum);
    //设置文件偏移量，定位到e_shoff，段表在文件中的偏移，根据ELF文件头得到
    a = fseek(fp, elf_head.e_shoff, SEEK_SET);  
    //读取所有的段表内容到elf_shdr中，大小为sizeof(sizeof(Elf64_Shdr)*elf_head.e_shnum)
    a=fread(elf_shdr,sizeof(Elf64_Shdr)*elf_head.e_shnum,1,fp);
    //重置指针位置到文件头
    rewind(fp);

    //找到字符串表的下标
    int strndx{};//字符串表的下标
    for(int i=1;i<elf_head.e_shnum;i++){
        if(strcmp(readshname(fp,elf_shdr[i].sh_name),".strtab")==0){
            strndx=i;
        }
    }

    //定位到字符串表
    char *elf_strtab=(char*)malloc(sizeof(char)*elf_shdr[strndx].sh_size);
    a=fseek(fp,elf_shdr[strndx].sh_offset,SEEK_SET);//这里应该是偏移量，而不是addr，addr是可以被加载之后
    a=fread(elf_strtab,elf_shdr[strndx].sh_size,1,fp);
    rewind(fp);

    char *symname=(char*)malloc(sizeof(char)*1);
    int i=0;

    while(elf_strtab[p]!='\0'){
        symname[i++]=elf_strtab[p++];
        symname=(char*)realloc(symname,(i+1)*sizeof(char));//后面的是重新分配之后的所有的大小
    }
    symname=(char*)realloc(symname,(i+1)*sizeof(char));//后面的是重新分配之后的所有的大小
    symname[i]='\0';
   // cout<<shname<<endl;

    return symname;

}


//5 输出符号表中的信息，符号名，符号大小，符号所在段，符号对应的值等信息
//readelf-s hello
int readSymtab(FILE *fp){
    Elf64_Ehdr elf_head;
    int a{};

	// 读取ELF Header到elf_head中
	a=fread(&elf_head, sizeof(Elf64_Ehdr), 1, fp);


    //读取ELH Section Header 到elf_shdr
    //定义一个指向Elf64_Shdr类型的指针，也就是说elf_shdr是一个有elf_head.e_shnum个Elf64_Shdr类型的数组
    Elf64_Shdr *elf_shdr=(Elf64_Shdr*)malloc(sizeof(Elf64_Shdr)*elf_head.e_shnum);
    //设置文件偏移量，定位到e_shoff，段表在文件中的偏移，根据ELF文件头得到
    a = fseek(fp, elf_head.e_shoff, SEEK_SET);  
    //读取所有的段表内容到elf_shdr中，大小为sizeof(sizeof(Elf64_Shdr)*elf_head.e_shnum)
    a=fread(elf_shdr,sizeof(Elf64_Shdr)*elf_head.e_shnum,1,fp);
    //重置指针位置到文件头
    rewind(fp);

    //找到字符串表的下标
    int symndx{};//字符串表的下标
    for(int i=1;i<elf_head.e_shnum;i++){
        if(strcmp(readshname(fp,elf_shdr[i].sh_name),".symtab")==0){
            symndx=i;
        }
    }
    Elf64_Sym *elf_sym=(Elf64_Sym*)malloc(elf_shdr[symndx].sh_size);
    a=fseek(fp,elf_shdr[symndx].sh_offset,SEEK_SET);
    a=fread(elf_sym,elf_shdr[symndx].sh_size,1,fp);
    rewind(fp);


    int symnum=elf_shdr[symndx].sh_size/sizeof(Elf64_Sym);
    cout<<"____________________________________________________________________________________________________________________________"<<endl;
    cout<<"| "<<"符号表有："<<(symnum)<<"个符号"<<endl;
    cout<<"|___________________________________________________________________________________________________________________________"<<endl;
    cout<<"| [Num]"<<setw(70)<<"Name"<<setw(10)<<"Size"<<setw(10)<<"Ndx"<<setw(16)<<"value"<<endl;
    //遍历每一个Section Header
    for(int i=0;i<symnum;i++)
    {
        //elf_shdr[i].sh_name需要用到段表字符串表
        cout<<dec;
        cout<<"| ["<<i<<"]"<<setw(70)<<readsymname(fp,elf_sym[i].st_name)<<
        setw(10)<<elf_sym[i].st_size;
        cout<<setw(10);

        switch (elf_sym[i].st_shndx)
        {
        case 0xfff1:cout<<"ABS";break;
        case 0xfff2:cout<<"ABS";break;        
        case 0:cout<<"UND";break;
        default:
            cout<<elf_sym[i].st_shndx;break;
        }
        cout<<hex;
        cout<<setw(16)<<elf_sym[i].st_value;
        

        cout<<endl;//还没有改动

    }
    cout<<"|______________________________________________________________________________________________________________________________"<<endl;

    return 0;
}




/*./myreadelf -h hello 查看头文件
./myreadelf -S hello    查看段表
./myreadelf -x.shs hello 查看段表字符串表
./myreadelf -x.s hello 查看字符串表
./myreadelf -s hello 查看符号表
./myreadelf - hello 全部执行

*/
int main(int argc,char*argv[])
{
    char *filename;
    char *opt;
    opt=argv[1];
    filename=argv[2];

    FILE *fp;
    fp = fopen(filename, "r");
	if (NULL == fp)
	{
		cout<<"未能打开文件！"<<endl;
        return 0;
	}
    


    if(strcmp(opt,"-h")==0){
        readEhdr(fp);
        return 0;
    }else if(strcmp(opt,"-S")==0){
        readShdr(fp);
        return 0;
    }else if(strcmp(opt,"-x.shs")==0){
        readShstrtab(fp);
        return 0;
    }else if(strcmp(opt,"-x.s")==0){
        readStrtab(fp);
        return 0;
    }else if(strcmp(opt,"-s")==0){
        readSymtab(fp);    
        return 0;   
    }else if(strcmp(opt,"-")==0){
        readEhdr(fp);
        readShdr(fp);
        readShstrtab(fp);
        readStrtab(fp);
        readSymtab(fp); 
    }
    else{
        cout<<"你输入的命令有错误！"<<endl;
    }
    fclose(fp);

    return 0;
}