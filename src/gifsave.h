
enum GIF_Code {GIF_OK,GIF_ERRCREATE,GIF_ERRWRITE,GIF_OUTMEM};

int GIF_Create(const char* filename,int width,int height,int numcolors,int colorres);
void GIF_SetColor(int colornum,int red,int green,int blue);
int GIF_CompressImage(int left, int top, int width, int height,unsigned char *image,int bpl,unsigned char *map);
int  GIF_Close(void);

