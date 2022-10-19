#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <windows.h>
#include <winuser.h>
#include "fractald.h"

#include "types.h"
#include "fwindow.h"
#include "gifsave.h"

#include <string.h>
#include <math.h>

//#define LOG

int grayscale=0;
unsigned char rgb[3]={0,1,2};
int width=640,height=480;
double zoom=1;
double aspect=1;

int depthbit=0,depthbyte=0;
int key_step=10;
double mx=0,my=0,mz=0;
int dirty=1;
unsigned char *backbytes,*bgmem;
BITMAPINFO *bif;
HWND window;
HINSTANCE hinstance;
HACCEL accel;
fractal_window fw;
HDC dc;
char k_shift,k_ctrl,k_alt;
char f_key;
char bmp_rle=1,topmost=0;

void repaint() {
  SetDIBitsToDevice(dc,0,0,width,height,0,0,
      0,height,bgmem,bif,DIB_RGB_COLORS);
}

int ff_test(real x,real y) {
  return 255*((2+sin(x+y)+sin(x-y))/4);
}

void update_back() {
  fw_resize(&fw,width,height,backbytes);
}

typedef struct {
  unsigned char i,r,g,b;
} palette_item;

palette_item rainbow_palette[]={
 {0,0,0,0},
 {36,255,0,0},
 {73,255,255,0},
 {109,0,255,0},
 {146,0,255,255},
 {182,0,0,255},
 {219,255,0,255},
 {255,255,255,255},
};

palette_item rgb_palette[]={
 {0,0,0,0},
 {64,255,0,0},
 {128,0,0,255},
 {192,0,255,0},
 {255,255,255,255}
};

palette_item gray_palette[]={
  {0,0,0,0},
  {255,255,255,255}
};

palette_item red_palette[]={ {0,0,0,0}, {255,255,128,128} };
palette_item green_palette[]={ {0,0,0,0}, {255,128,255,128} };
palette_item blue_palette[]={ {0,0,0,0}, {255,128,128,255} };

palette_item yellow_palette[]={ {0,0,0,0}, {255,255,255,0} };
palette_item orange_palette[]={ {0,0,0,0}, {255,255,192,128} };
palette_item cyan_palette[]={ {0,0,0,0}, {255,128,255,255} };


palette_item layer_palette[]={
  {0,0,0,0},
  {36,255,0,0},
  {41,0,0,0},
  {73,255,255,0},
  {78,0,0,0},
  {109,0,255,0},
  {114,0,0,0},
  {146,0,255,255},
  {151,0,0,0},
  {182,0,0,255},
  {187,0,0,0},
  {219,255,0,255},
  {224,0,0,0},
  {255,255,255,255}
};

palette_item bluered_palette[]={
  {0,0,0,0},
  {16,0,0,255},
  {128,255,0,0},
  {255,255,255,255}
};

palette_item greenblue_palette[]={
  {0,0,0,0},
  {16,0,255,0},
  {128,0,0,255},
  {255,255,255,255}
};

palette_item redgreen_palette[]={
  {0,0,0,0},
  {16,255,0,0},
  {128,0,255,0},
  {255,255,255,255}
};


void set_pal(palette_item *p) {
  int i,a,b,d;
  for(i=0;i<256;i++) {
    if(p[1].i<i) p++;
    a=p[1].i-i,b=i-p[0].i,d=p[1].i-p[0].i;
    bif->bmiColors[i].rgbRed=(p[0].r*a+p[1].r*b)/d;
    bif->bmiColors[i].rgbGreen=(p[0].g*a+p[1].g*b)/d;
    bif->bmiColors[i].rgbBlue=(p[0].b*a+p[1].b*b)/d;
  }
}

unsigned char random1() {
  int r=rand()*511/RAND_MAX-127;
  return r<0?0:r>255?255:r;
}
void random_pal() {
  palette_item pal[4];
  int i,n;
  srand(GetTickCount());
  n=rand()%3+2;
  for(i=0;i<n;i++) {
    pal[i].i=i*255/(n-1);
    pal[i].r=random1();
    pal[i].g=random1();
    pal[i].b=random1();
  }
  set_pal(pal);
}

void inv_pal() {
  int i;
  for(i=0;i<256;i++) {
    bif->bmiColors[i].rgbRed^=255;
    bif->bmiColors[i].rgbGreen^=255;
    bif->bmiColors[i].rgbBlue^=255;
  }
}

void neg_pal() {
  int i;
  for(i=0;i<256;i++) {
    RGBQUAD rgb=bif->bmiColors[i];
    BYTE mi=rgb.rgbBlue,ma=mi,x;
    x=rgb.rgbGreen;if(x>ma) ma=x;else if(x<mi) mi=x;
    x=rgb.rgbRed;if(x>ma) ma=x;else if(x<mi) mi=x;
    rgb.rgbRed=mi+ma-rgb.rgbRed;;
    rgb.rgbGreen=mi+ma-rgb.rgbGreen;
    rgb.rgbBlue=mi+ma-rgb.rgbBlue;
    bif->bmiColors[i]=rgb;
  }
}

void rev_pal() {
  int i;
  RGBQUAD rgb;
  for(i=0;i<128;i++) {
    rgb=bif->bmiColors[i];
    bif->bmiColors[i]=bif->bmiColors[255-i];
    bif->bmiColors[255-i]=rgb;
  }
}

int full_pal() {
  uchar *h=fw.image,*e,mi=*h,ma=*h;
  int i,j;
  RGBQUAD rgb[256];
  for(e=h+fw.width*fw.height;h<e;h++) {
    if(*h<mi) mi=*h;else if(*h>ma) ma=*h;else continue;
    if(!mi&&!~ma) return 0;
  }
  if(mi==ma) return 0;
  memcpy(rgb,bif->bmiColors,1024);
  for(i=mi;i<=ma;i++) {
    j=(i-mi)*255/(ma-mi);
    bif->bmiColors[i]=rgb[j];
  }
  return 1;
}

void light_pal(uchar level,char dark) {
  int i;
  RGBQUAD *q=bif->bmiColors;
  if(!level) return;
  level=256-level;
  for(i=0;i<256;i++,q++) {
    if(dark) {
      q->rgbRed=(level*q->rgbRed)>>8;
      q->rgbGreen=(level*q->rgbGreen)>>8;
      q->rgbBlue=(level*q->rgbBlue)>>8;
    } else {
      q->rgbRed=255-(level*(255-q->rgbRed)>>8);
      q->rgbGreen=255-(level*(255-q->rgbGreen)>>8);
      q->rgbBlue=255-(level*(255-q->rgbBlue)>>8);
    }
  }
}

uchar div1(uchar level,uchar x) {
  return x<=level?x*(255-level)/level:255-(255-x)*level/(255-level);
}

int div3(uchar level,int x) {
  register int m=765,l=level*3;
  return x<=l?x*(m-l)/l:m-(m-x)*l/(m-l);
}

int i765(int color,int i,char satur) {
  if(i<=0) return 0;else if(i>=765) return 0xffffff;
  int r=color&255,g=(color>>8)&255,b=(color>>16)&255;
  int s=r+g+b;
  if(s==i) goto end;
  if(r==b&&b==g) { r=g=b=i/3;/*s-=3*r;if(s>0) {b++;if(s>1) r++;}*/;goto end2;}          
  if(satur) {
    if(i<s) {
      int mi=r<g?r:g;
      if(b<mi) mi=b;
      if(mi>0) {
        if(satur>1)
          r=255-(255-r)*255/(255-mi),g=255-(255-g)*255/(255-mi),b=255-(255-b)*255/(255-mi);
        else {
          if(3*mi>s-i) mi=(s-i)/3;
          r-=mi;g-=mi;b-=mi;
        }
      }
    } else {
      int ma=r>g?r:g;
      if(b>ma) ma=b;
      if(ma<255) {
        if(satur>1) 
          r=r*255/ma,g=g*255/ma,b=b*255/ma;
	else {
	  ma^=255;
	  if(3*ma>i-s) ma=(i-s)/3;
	  r+=ma;g+=ma;b+=ma;
	}                  

      }            
    }
    s=r+g+b;
  }
  if(i<s) {
    if(i+2<s) r=r*i/s,g=g*i/s;
    b=i-r-g;
  } else {
    i=765-i;s=765-s;
    if(i+2<s) r=255-((255-r)*i/s),g=255-((255-g)*i/s);
    b=(765-i)-r-g;
  }
 end2:
  if(b<0) {
    b++;if(r>0) r--;else if(g>0) g--;
    if(b<0) {b++;if(g>0) g--;else if(r>0) r--;}
  } else if(b>255) {
    b--;if(r<255) r++;else if(g<255) g++;
    if(b>255) {b--;if(g<255) g++;else if(r<255) r++;}
  }
 end: 
  return r|(g<<8)|(b<<16);
}


void div_pal(uchar level,char x3,char satur) {
  uchar *p=(uchar*)bif->bmiColors,*e=p+1024;
  for(;p<e;p+=4) {
    if(x3) {
      int c=*(int*)p;
      c=i765(c,div3(level,p[0]+p[1]+p[2]),satur);
      p[0]=c&255;
      p[1]=(c>>8)&255;
      p[2]=(c>>16)&255;
    } else {
      p[0]=div1(level,p[0]);
      p[1]=div1(level,p[1]);
      p[2]=div1(level,p[2]);
    }
  }
}

void used(char *map) {
  uchar *h=fw.image,*e;
  memset(map,0,256);
  for(e=h+fw.width*fw.height;h<e;h++) map[*h]=1;
}

int mima_pal(char uni) {
  char map[256];
  uchar c;
  RGBQUAD *p=bif->bmiColors,*q=p+*fw.image;
  uchar r,ri=q->rgbRed,ra=ri,g,gi=q->rgbGreen,ga=gi,b,bi=q->rgbBlue,ba=bi;
  int i;
  used(map);
  for(i=0;i<256;i++) {
    if(map[i]) {
      q=p+i;
      c=q->rgbRed;if(c<ri) ri=c;else if(c>ra) ra=c;
      c=q->rgbGreen;if(c<gi) gi=c;else if(c>ga) ga=c;
      c=q->rgbBlue;if(c<bi) bi=c;else if(c>ba) ba=c;
    }
  }
  if(uni) {
    if(gi<ri) ri=gi;if(bi<ri) ri=bi;
    if(ga>ra) ra=ga;if(ba>ra) ra=ba;
    gi=bi=ri;
    ga=ba=ra;
  }
  ra-=ri;r=ra&&ra<255;
  ga-=gi;g=ga&&ga<255;
  ba-=bi;b=ba&&ba<255;
  if(!(r|g|b)) return 0;
  for(i=0,q=p;i<256;i++,q++) {
    if(r) q->rgbRed=255*(q->rgbRed-ri)/ra;
    if(g) q->rgbGreen=255*(q->rgbGreen-gi)/ga;
    if(b) q->rgbBlue=255*(q->rgbBlue-bi)/ba;
  }
  return 1;
}

int imima_pal(char all,char satur) {
  char map[256];
  RGBQUAD *p=bif->bmiColors,*q=p+*fw.image;
  int i,mi=q->rgbRed+q->rgbGreen+q->rgbBlue,ma=mi,s,c;
  if(!all) used(map);
  for(i=0;i<256;i++) {
    if(all||map[i]) {
      q=p+i;
      s=q->rgbRed+q->rgbGreen+q->rgbBlue;
      if(s<mi) mi=s;else if(s>ma) ma=s;
    }
  }
  ma-=mi;
  if(!ma||ma==765) return 0;
  for(i=0;i<256;i++) {
    if(all||map[i]) {
      q=p+i;
      s=q->rgbRed+q->rgbGreen+q->rgbBlue;
      c=q->rgbRed|(q->rgbGreen<<8)|(q->rgbBlue<<16);
      s=765*(s-mi)/ma;
      c=i765(c,s,satur);
      q->rgbRed=c&255;
      q->rgbGreen=(c>>8)&255;
      q->rgbBlue=(c>>16)&255;
    }
  }
  return 1;
}

void mix_pal(uchar level,uchar mix) {
  RGBQUAD *q=bif->bmiColors,*e=q+256,*z=q+level;
  uchar a=256-mix,r=z->rgbRed,g=z->rgbGreen,b=z->rgbBlue;
  for(;q<e;q++) {
    q->rgbRed=(a*q->rgbRed+r*mix)>>8;
    q->rgbGreen=(a*q->rgbGreen+g*mix)>>8;
    q->rgbBlue=(a*q->rgbBlue+b*mix)>>8;
  }
}

void cut_pal(uchar level,char less) {
  RGBQUAD *q=bif->bmiColors,*e=q+256,*z=q+level;
  if(less) e=z;else q=z+1;
  for(;q<e;q++)
    *q=*z;
}
void lim_pal(uchar level,char black) {
  RGBQUAD *q=bif->bmiColors,*e=q+256,*z=q+level;
  int s,sum=2*z->rgbRed+3*z->rgbGreen+z->rgbBlue;
  for(;q<e;q++) {
    s=2*q->rgbRed+3*q->rgbGreen+q->rgbBlue;
    if(black?s<sum:s>sum) *q=*z;
  }
}


void rotrgb_pal(char right,char swap) {
  int i;
  for(i=0;i<256;i++) {
    RGBQUAD rgb=bif->bmiColors[i];
    BYTE x=rgb.rgbBlue;
    if(swap) {
      rgb.rgbBlue=rgb.rgbRed;
      rgb.rgbRed=x;
    } else if(right) {
      rgb.rgbBlue=rgb.rgbGreen;
      rgb.rgbGreen=rgb.rgbRed;
      rgb.rgbRed=x;
    } else {
      rgb.rgbBlue=rgb.rgbRed;
      rgb.rgbRed=rgb.rgbGreen;
      rgb.rgbGreen=x;
    }
    bif->bmiColors[i]=rgb;
  }
}

void rot_pal(BYTE r) {
  RGBQUAD copy[256];
  memcpy(copy,bif->bmiColors,1024);
  memcpy(bif->bmiColors,copy+r,4*(256-r));
  memcpy(bif->bmiColors+256-r,copy,4*r);
}

void dbl_pal(char mirror) {
  int i;
  for(i=0;i<128;i++) 
    bif->bmiColors[i]=bif->bmiColors[255*i/127];
  for(i=128;i<256;i++) 
    bif->bmiColors[i]=bif->bmiColors[mirror?255-i:i-128];
}

void tri_pal() {
  int i;
  for(i=0;i<86;i++) 
    bif->bmiColors[i]=bif->bmiColors[255*i/85];
  for(i=0;i<86;i++) 
    bif->bmiColors[169-i]=bif->bmiColors[170+i]=bif->bmiColors[i];
}

int resize_dc() {
  RECT r;
  int nwidth,nheight;

  GetClientRect(window,&r);
  nwidth=r.right,nheight=r.bottom;
  nwidth=(nwidth+3)&~3;
  if(width==nwidth&&height==nheight&&bif)
    return 0;
  width=nwidth,height=nheight;
  if(!bif) { 
    bif=malloc(sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD[256]));
    memset(bif,0,sizeof(BITMAPINFOHEADER));
    set_pal(gray_palette);
    //for(i=0;i<256;i++)
    //  bif->bmiColors[i].rgbBlue=bif->bmiColors[i].rgbRed=bif->bmiColors[i].rgbGreen=i;
    bif->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    bif->bmiHeader.biPlanes=1;
    bif->bmiHeader.biBitCount=8;
    bif->bmiHeader.biXPelsPerMeter=0;
    bif->bmiHeader.biYPelsPerMeter=0;
    bif->bmiHeader.biCompression=BI_RGB;
    bif->bmiHeader.biClrUsed=256;
    bif->bmiHeader.biClrImportant=256;
  }
  if(backbytes)
    free(backbytes);
  backbytes=(byte*)malloc(width*height);
  bif->bmiHeader.biWidth=width;
  bif->bmiHeader.biHeight=-height;
  bif->bmiHeader.biSizeImage=width*height;
  bgmem=backbytes;
  return 1;
}

long FAR PASCAL main_WndProc (HWND hWnd, UINT iMessage,
    WPARAM wParam, LPARAM lParam);

void create_window(char full) {
  WNDCLASS WndClass;
  DWORD ws;

  accel = LoadAccelerators(hinstance,"accel_main");

  WndClass.cbClsExtra = 0;
  WndClass.cbWndExtra = 0;
  WndClass.hbrBackground = NULL;
  WndClass.hCursor=LoadCursor(NULL, IDC_ARROW);
  WndClass.hIcon=LoadIcon(hinstance,"icon_fractal");
  WndClass.hInstance = hinstance;
  WndClass.style=CS_OWNDC; // CS_HREDRAW | CS_VREDRAW;
  WndClass.lpfnWndProc = main_WndProc;
  WndClass.lpszClassName = "FractalClass";
  WndClass.lpszMenuName = NULL ; //"menu_main";
  RegisterClass(&WndClass); // main window class

  ws=WS_VISIBLE|WS_TILEDWINDOW;
  window=CreateWindowEx(topmost?WS_EX_TOPMOST:0,"FractalClass","Fractal",ws,
      0,0,width,height,NULL,NULL,hinstance,NULL);
  if(full) {
    ShowWindow(window,SW_MAXIMIZE);
    SetWindowLong(window,GWL_STYLE,GetWindowLong(window,GWL_STYLE)^WS_CAPTION);
    SetMenu(window,NULL);
  }
  dc=GetDC(window);
  resize_dc();
}

int fn_ext(const char *filename,int extlen,char *extension) {
  const char *h,*e;
  char *g=extension;
  for(h=filename,e=NULL;*h;h++) 
    switch(*h) {
     case '/':
     case '\\':
     case ':':e=NULL;break;
     case '.':e=h;break;
    }
  if(e++) {
    while(--extlen&&*e)
      *g++=tolower(*e++);
  }
  *g=0;
  return !!*extension;
}

int Savefi=2;
int GetSaveFilename(char *file,char *title) {
  char filename[256],dir[256],ext[8],*h2;
  OPENFILENAME of;
  const char *h,*filter="BMP 8 bpp (CapsLock for RLE encoding) (*.bmp)\0*.bmp\0GIF (*.gif)\0*.gif\0PGM\0*.pgm\0All files (*.*)\0*.*\0";
  int r,n;

  memset(&of,0,sizeof(of));
  of.lStructSize=sizeof(of);
  //of.hwndOwner=esktopWindow();
  of.Flags=OFN_LONGNAMES|OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY;

  filename[0]=0;
  of.lpstrTitle=title;
  of.lpstrFilter=filter;
  GetCurrentDirectory(sizeof(dir),dir);
  of.lpstrInitialDir=dir;
  of.lpstrFile=filename;
  of.nMaxFile=sizeof(filename);
  of.nFilterIndex=Savefi;

  r=GetSaveFileName(&of);
  Savefi=of.nFilterIndex;
  if(r) {
    strcpy(file,filename);
    if(!fn_ext(filename,sizeof(ext),ext)&&Savefi) {
      for(h=filter,n=2*Savefi-1;n;n--) {
        while(*h) h++;
        h++;
      }
      if(h[0]=='*'&&h[1]=='.'&&h[2]&&h[2]!='*') {
        h+=1;
        h2=file+strlen(file);
        while(*h&&*h!=',') *h2++=*h++;
        *h2=0;
      }
    }
  }
  return r;
}

int rle_line(unsigned char *dst,unsigned char *src,int w) {
  int c;
  unsigned char *d=dst,*x,*l=src,*le=l+w;
  while(l<le) {
    if(*l==l[1]&&*l==l[2]) {
      for(x=l+3;x<le&&*x==*l&&(x!=le-3||(*x==x[1]&&*x==x[2]));x++);
      while(l<x) {
	c=x-l;
	if(c>255) c=c<258?c-3:255;
        *d++=c;
        *d++=l[0];
	l+=c;
      }
    } else {
      for(x=l+3;x<le&&(x+2>=le||*x!=x[1]||*x!=x[2]);x++);
      while(l<x) {
	c=x-l;
	if(c>255) c=c<258?c-3:255;
        *d++=0;
        *d++=c;
        memcpy(d,l,c);
        d+=c;
	if(c&1) *d++=0;
	l+=c;
      }
    }
  }
  return d-dst;
}

void set_hdr(char bc,int cc,char rle,int w,int h) {
  int bpl=(w*bc+31)/32*4;
  bif->bmiHeader.biBitCount=bc;
  bif->bmiHeader.biWidth=w;
  bif->bmiHeader.biHeight=h;
  bif->bmiHeader.biSizeImage=bpl*(h<0?-h:h);
  bif->bmiHeader.biCompression=rle?BI_RLE8:BI_RGB;
  bif->bmiHeader.biClrUsed=cc;
  bif->bmiHeader.biClrImportant=0;
}

int color_count(unsigned char *image,int bpl,int w,int h,unsigned char *map) {
  int x,y,n=0;
  unsigned char *p,*pe,use[256];
  for(y=0;y<h;y++,image+=bpl)
    for(p=image,pe=image+w;p<pe;p++)
      use[*p]=1;
  for(x=0,y=0;x<256;x++)
    if(use[x]) {
      n++;
      if(map) map[x]=y,map[256+y++]=x;
    }
  return n;  
}

int fwritemap(unsigned char *data,int len,FILE *f,unsigned char *map,unsigned char *buf) {
  register unsigned char *h,*he,*g;
  for(h=data,he=h+len,g=buf;h<he;*g++=map[*h++]);
  return fwrite(buf,1,len,f);
}

void write_bmp2(const char *filename,char rle,unsigned char *image,int bpl,int w,int h) {
  FILE *f; 
  BITMAPFILEHEADER bfh;
  unsigned char *img,map[512],buf[w];
  int i,bif_size,w2,h2,cc,bpl2=(w+3)&~3;

  if(!(f=fopen(filename,"wb")))
    return;
  cc=color_count(image,bpl,w,h,map);
  bif_size=sizeof(BITMAPINFOHEADER)+cc*sizeof(RGBQUAD);
  memset(&bfh,0,sizeof(bfh));
  bfh.bfType='B'|('M'<<8);
  bfh.bfOffBits=sizeof(bfh)+bif_size;
  bfh.bfSize=bfh.bfOffBits+bpl2*h;
  fwrite(&bfh,1,sizeof(bfh),f);
  w2=bif->bmiHeader.biWidth;
  h2=bif->bmiHeader.biHeight;
  if(fw.width<3) rle=0;
  set_hdr(8,cc,rle,w,h);
  fwrite(bif,1,sizeof(BITMAPINFOHEADER),f);
  for(i=0;i<cc;i++)
    fwrite(bif->bmiColors+map[256+i],1,sizeof(RGBQUAD),f);
  set_hdr(8,256,0,w2,h2);
  for(img=image+bpl*h;img>image;) {
    img-=bpl;
    if(rle) {
      int b,c;
      unsigned char *x,*l=img,*le=l+w;
      while(l<le) {
        if(*l==l[1]&&*l==l[2]) {
	  for(x=l+3;x<le&&*x==*l&&(x!=le-3||(*x==x[1]&&*x==x[2]));x++);
	  while(l<x) {
	    c=x-l;
	    if(c>255) c=c<258?c-3:255;
	    b=c|(map[l[0]]<<8);
	    fwrite(&b,1,2,f);
	    l+=c;
	  }
        } else {
	  //for(x=l+3;x<le&&(x+2>=le||*x!=x[1]||*x!=x[2]);x++);
	  for(x=l+3;x<le&&(x+3>=le||*x!=x[1]||*x!=x[2]||*x!=x[3]);x++);
	  while(l<x) {
	    c=x-l;
	    if(c>255) c=c<258?c-3:255;
	    b=c<<8;
	    fwrite(&b,1,2,f);
	    fwritemap(l,c,f,map,buf);
	    if(c&1) fwrite(&b,1,1,f);
	    l+=c;
	  }
        }
      }
      b=0;
      fwrite(&b,1,2,f);
    } else {
      fwritemap(img,w,f,map,buf);
      fwrite(img,1,bpl2-w,f);
    }
  }
  if(rle) {
    int b;
    fflush(f);
    b=ftell(f);
    fseek(f,2,SEEK_SET);
    fwrite(&b,1,4,f);
    b-=sizeof(bfh)+bif_size;
    fseek(f,sizeof(bfh)+((char*)&bif->bmiHeader.biSizeImage)-(char*)bif,SEEK_SET);
    fwrite(&b,1,4,f);
  }
  fclose(f);
}

char bound(int *x,int *y,int *x2,int *y2) {
  register int i,x0=*x,y0=*y,x1=*x2,y1=*y2;
  if(x1<x0) i=x0,x0=x1,x1=i;
  if(y1<y0) i=y0,y0=y1,y1=i;
  if(x1<0||y1<0||x0>=fw.width||y0>=fw.height) return 0;
  if(x0<0) x0=0;if(x1>=fw.width) x1=fw.width-1;
  if(y1<0) y1=0;if(y1>=fw.height) y1=fw.height-1;
  *x=x0,*y=y0,*x2=x1,*y2=y1;
  return 1;
}

void write_bmp(const char *filename,char rle,int x0,int y0,int x1,int y1) {
  if(bound(&x0,&y0,&x1,&y1)) 
    write_bmp2(filename,rle,fw.image+y0*fw.width+x0,fw.width,x1-x0+1,y1-y0+1);
}

void write_bmpidx(const char *mask,char rle) {
  static int fidx=-1;
  char name[32];
  FILE *f;
  while(1) {
    fidx++;
    sprintf(name,mask,fidx);
    if((f=fopen(name,"r"))) {
      fclose(f);
      fidx++;
    } else {
      write_bmp2(name,rle,fw.image,fw.width,fw.width,fw.height);
      break;
    }
  }
}

int set_clipboard(unsigned char *bits,int bpl,int w,int h) {
  BITMAPINFOHEADER *bih=&bif->bmiHeader;
  int cc,res=0;
  HANDLE sh;
  char *s2;
  RGBQUAD pal[256];
  unsigned char map[512];

  //printf("setclip\n");
  if(!OpenClipboard(NULL))
    return 0;
  EmptyClipboard();
  cc=color_count(bits,bpl,w,h,map);
  int x,y,w2=bih->biWidth,h2=bih->biHeight,bc=cc<=2?1:cc<=16?4:8,size,bpl2=(w*bc+31)/32*4,xm=bc==1?7:bc==4?1:0;
  memset(pal,0,sizeof(pal));
  for(x=0;x<cc;x++) 
    pal[x]=bif->bmiColors[map[256+x]];
  set_hdr(bc,cc,0,w,h);
  sh=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,sizeof(*bih)+cc*sizeof(*pal)+bih->biSizeImage);
  s2=GlobalLock(sh);
  memcpy(s2,bih,size=sizeof(*bih));s2+=size;
  memcpy(s2,pal,size=cc*sizeof(*pal));s2+=size;
  for(y=h;y-->0;s2+=bpl2) {
    unsigned char *ph=bits+y*bpl,*g=s2,r=0;
    for(x=0;x<w;ph++) {
      int ix=map[*ph];
      r=(r<<bc)|ix;
      x++;
      if(!(x&xm)) {*g++=r;r=0;}
    }
    if(x&xm) { do r<<=bc;while(++x&xm);*g=r;}
  }
  GlobalUnlock(sh);

  if(!SetClipboardData(CF_DIB,sh))
    printf("%d",(int)GetLastError());
  CloseClipboard();

  set_hdr(8,256,0,w2,h2);

  return res;
}

void write_gif(const char *filename,int x0,int y0,int x1,int y1) {
  int i,w,h,n;
  RGBQUAD *q;
  unsigned char *img=fw.image+fw.width*y0+x0,map[512];
  if(!bound(&x0,&y0,&x1,&y1)) return;
  w=x1-x0+1,h=y1-y0+1;
  n=color_count(img,fw.width,w,h,map);
  if(GIF_OK==GIF_Create(filename,w,h,n,8)) {
    for(i=0;i<n;i++) {
      q=bif->bmiColors+map[256+i];
      GIF_SetColor(i,q->rgbRed,q->rgbGreen,q->rgbBlue);
    }
    GIF_CompressImage(0,0,w,h,img,fw.width,map);
    GIF_Close();
  }
}

void write_pgm(const char *filename,int x0,int y0,int x1,int y1) {
  unsigned char *img=fw.image+fw.width*y0+x0;
  int w,h;
  FILE *f;
  if(!bound(&x0,&y0,&x1,&y1)) return;
  w=x1-x0+1,h=y1-y0+1;
  if((f=fopen(filename,"wb"))) {
    fputs("P5\n",f);
    //fprintf(f,"# %e %e %e\n",(double)fw.sx,(double)fw.sy,(double)fw.wx);
    fprintf(f,"%d %d\n255\n",fw.width,fw.height);
    for(;h--;img+=fw.width)
      fwrite(img,w,1,f);
    fclose(f);
  }
}

void write_ext(char ext,const char *filename,int x0,int y0,int x1,int y1) {
  if(ext=='b') 
    write_bmp(filename,1,0,0,fw.width,fw.height);
  else if(ext=='p') 
    write_pgm(filename,0,0,fw.width,fw.height);
  else 
    write_gif(filename,0,0,fw.width,fw.height);
}

void write_next() {
  static int fidx=0;
  char name[16],*ext=Savefi==1?"bmp":Savefi==3?"pgm":"gif";
  FILE *f;
  printf("%d\n",Savefi);
  while(1) {
    fidx++;
    sprintf(name,"out%03d.%s",fidx,ext);
    if((f=fopen(name,"r"))) {
      fclose(f);
      fidx++;
    } else {
      write_ext(ext[0],name,0,0,fw.width,fw.height);
      break;
    }
  }
}

char is_ext(const char *filename,const char *ext) {
  char buf[4];
  return fn_ext(filename,4,buf)&&!strcmp(buf,ext);
}

void PrintHelp(HWND wnd) {
  MessageBox(wnd,
"left button: window zoom (ctrl with mirror,shift without aspect recalc)\n"
"right button: click unzoom, drag shifts window (ctrl rotate, ctrl+shift point rotate rotate)\n"
"wheel: point zoom (ctrl up-down, shift left-right)\n"
"\n"
"F1:  this help\n"
"Esc,q: exit\n"
"F11: full screen (or space, ctrl hides/shows menu)\n"
"m:   menu on/off\n"
"arrow: move by pixels\n"
"PgUp: move by page (PgDown,Delete,End)\n"
"Ins: zoom in (or Enter/Num+, Home/Backspace/Num- zoom out)\n"
"Ctrl+C: copy to clipboard\n"
"\n"
"r:   reset zoom (or Num-, shift,ctrl also set mandelbrot set)\n"
"j:   julia (shift juliab, ctrl mandelbrot)\n"
"h:   julia 3 (--||--) \n"
"k:   julia 4 (--||--) \n"
"n:   julia 16 (--||--) \n"
"b:   julia 64 (--||--) \n"
"g:   mandelbrot alternating (shift julia,ctrl mandel2/mandel4) \n"
"\n"
"1-9: color and palletes\n"
"~:   rotate rgb (shift reverse palette,ctrl rgb<=>cmy),ctrl+shift invert\n"
"i:   invert colors,(shift reverse palette)\n"
"d:   double palette (shift with mirror,ctrl triple)\n"
"c:   contrast (ctrl+shift rgb independent, shift full palette)\n"
"v:   intensity contrast (ctrl all colors, shift saturate colors)\n"
"t:   palette modulo on/off\n"
"0:   random palette (or Num0, ctrl invert,alt reverse\n"
"\n"
"<>:  decrease/increase depth 2x (numpad /*)\n"
"[]:  rotate palette (shift 4x,ctrl 8x)\n"
".,:  decrease/increase depth calculation\n"
"a:   2x2 antialias on/off (Ctrl for 3x3,ctrl+shift 4x4)\n"
"s:   4x4 speedup on/off (ctrl for 8x8 nad shift for 16x16)\n"
"u:   flip vertical (o: flip horizontal)\n"
"\n"
"f:   save file with dialog (or F2, shift for 2x image size,ctrl 4x ctrl+shift 8x)\n"
"p:   write out###.gif file (ext from save dialog filter)\n"
"\n"
"Command line:\n"
"-f fullscreen (F11)\n"
"-t topmost (F10)\n"
"-a antialias 2x2 (-aa 3x3)\n"
"-s speedup 4x4 (-ss 8x8,-sss 16x16)\n"
"-eb default file filter extension (-eb bmp, -eg gif, -ep pgm,-ea all)\n"
"-c# use # cores (default 2)\n"
    ,"Fractal 0.16",MB_OK
  );
}

void Wheel(int wheel,POINT pt,int wParam) {
  char ctrl=wParam&0x8,shift=wParam&0x4;
  ScreenToClient(window,&pt);

  if(k_alt) 
    fw_point_rotate(&fw,pt.x,pt.y,(wheel/120)*M_PI/24);
  else if(ctrl&&!shift) 
    fw_move_window(&fw,0,(wheel>0?1:-1)*-0.125*(fw.asp<0?-1:1));
  else if(shift&&!ctrl) 
    fw_move_window(&fw,(wheel>0?1:-1)*-0.125*(fw.wx<0?-1:1),0);
  else 
    fw_point_zoom(&fw,pt.x,pt.y,wheel>0?ctrl?0.9375:0.75:ctrl?1.0625:1.25,0,0);
  fw_recalc(&fw);
}

char xf3;

fractal_function f_function(int key,char ctrl,char shift) {
  fractal_function ff;
  switch(key) {
   case 'B':ff=ctrl?ff_mandel64:shift?ff_julia64b:ff_julia64;break;
   case 'N':ff=ctrl?ff_mandel16:shift?ff_julia16b:ff_julia16;break;
   case 'K':ff=ctrl?ff_mandel4:shift?ff_julia4b:ff_julia4;break;
   case 'H':ff=ctrl?ff_mandel3:shift?ff_julia3b:ff_julia3;break;
   case 'L':ff=ctrl?ff_mandel6:shift?ff_julia6b:ff_julia6;break;
   default:ff=ctrl?xf3?ff_mandelbrotx:ff_mandelbrot:shift?xf3?ff_juliabx:ff_juliab:xf3?ff_juliax:ff_julia;break;
  }
  return ff;
}

char Alt() { return 0!=(GetKeyState(VK_MENU)&0x8000);}
char Shift() { return 0!=(GetKeyState(VK_SHIFT)&0x8000); }
char Ctrl() { return 0!=(GetKeyState(VK_CONTROL)&0x8000);}
char CapsLock() { return 0!=(GetKeyState(VK_CAPITAL)&0x1);}

int px,py,pk; // mouse left button press x,y
int lx,ly,lb,rb,mb; // last mouse position

void XorBox(HDC dc,char line,int x1,int y1,int x2,int y2) {
  int rop;
  HGDIOBJ pen;

  rop=SetROP2(dc,R2_XORPEN);
  pen=SelectObject(dc,GetStockObject(WHITE_PEN)); 
  MoveToEx(dc,x1,y1,NULL);
  if(line)
    LineTo(dc,x2,y2);
  else  {
    LineTo(dc,x2,y1);
    LineTo(dc,x2,y2);
    LineTo(dc,x1,y2);
    LineTo(dc,x1,y1);
  }
  SelectObject(dc,pen);
  SetROP2(dc,rop);
}

void XorStop() {
  if(lb||rb||mb) {
    XorBox(dc,rb,px,py,lx,ly);
    ReleaseCapture();
  }
  lb=rb=mb=0;
}

void SaveOrCopy(char *filename,int scale) {
  void *image=NULL;
  char clip=!filename,rle=0;
  int x0,y0,x1,y1,w,h,w2=fw.width,h2=fw.height;
  HCURSOR hc;
  if(lb) {
    XorStop();
    x0=px,y0=py,x1=lx,y1=ly;
    if(!bound(&x0,&y0,&x1,&y1)) return;
  } else x0=y0=0,x1=fw.width-1,y1=fw.height-1;
  if(!clip) {
    if(!GetSaveFilename(filename,"Save"))
      return;
    rle=CapsLock();
  }
  hc=SetCursor(LoadCursor(NULL,IDC_WAIT));
  if(scale>1) {
    image=fw.image;
    w=(x1-x0+1)*scale,h=(y1-y0+1)*scale;
    fw_resize(&fw,w,h,malloc(w*h));
    while(fw_calc_line(&fw));
    x0=y0=0,x1=w-1,y1=h-1;
  }
  if(clip) 
    set_clipboard(fw.image+fw.width*y0+x0,fw.width,x1-x0+1,y1-y0+1);
  else if(is_ext(filename,"gif"))
    write_gif(filename,x0,y0,x1,y1);
  else if(is_ext(filename,"pgm"))
    write_pgm(filename,x0,y0,x1,y1);
  else
    write_bmp(filename,rle,x0,y0,x1,y1);
  SetCursor(hc);
  if(image) {
    free(fw.image);
    fw_resize(&fw,w2,h2,image);
  }
}

int ProcessKey(int Key,int x,int y) {
  int r=0;
  fractal_function ff;
  real p1x,p1y;
  switch(Key) {
   case 192:k_ctrl?k_shift?inv_pal():neg_pal():k_shift?rev_pal():rotrgb_pal(1,k_alt);goto repaint;
   case 219:rot_pal(k_ctrl?8:k_shift?4:1);goto repaint;
   case 221:rot_pal(k_ctrl?-8:k_shift?-4:-1);goto repaint;
   case VK_NUMPAD5:
   case '1':set_pal(gray_palette);goto repaint;
   case '2':set_pal(red_palette);goto repaint;
   case '3':set_pal(yellow_palette);goto repaint;
   case '4':set_pal(orange_palette);goto repaint;
   case '5':set_pal(rainbow_palette);goto repaint;
   case '6':set_pal(layer_palette);goto repaint;
   case '7':set_pal(bluered_palette);goto repaint;
   case '8':set_pal(greenblue_palette);goto repaint;
   case '9':set_pal(redgreen_palette);goto repaint;
   case VK_NUMPAD0:
   case '0':k_ctrl?inv_pal():k_alt?rev_pal():random_pal();goto repaint;
   case 'D':
    if(k_ctrl) tri_pal();
    else dbl_pal(k_shift);
    goto repaint;
   case 'V':
    if(imima_pal(k_ctrl,k_shift)) goto repaint;
    break;
   case 'C':
    if(k_ctrl&&!k_shift) {
      SaveOrCopy(NULL,1);
      break;
    } 
    if(k_shift&&!k_ctrl?full_pal():mima_pal(!k_ctrl))
      goto repaint;
    break;
   case 'A':if(((k_ctrl&&!k_shift&&fw.aa==3)||(k_ctrl&&k_shift&&fw.aa==4))&&!fw.fast) break;fw.aa=k_ctrl?k_shift?4:3:fw.aa&&!fw.fast?0:2;if(fw.aa) fw.fast=0;goto recalc;
   case 'S':
     if((k_ctrl&&fw.fast==8)||(k_shift&&fw.fast==16)) break;
     fw.fast=k_ctrl?8:k_shift?16:fw.fast?0:4;goto recalc;
   case 'I':
    (k_ctrl?neg_pal:k_shift?inv_pal:rev_pal)();
   repaint:
    repaint();
    break;
   case 'T':
    fw_change_i2u(&fw);
    goto recalc;
   case 'U':
    fw_flip(&fw,0,1);
    goto repaint;
   case 'O':
    fw_flip(&fw,1,0);
    goto repaint;
   case VK_SUBTRACT:
   case 'R':
    if(k_shift||k_ctrl) xf3=0,fw.func=f_function(f_key,1,0),ff_cxx=ff_cyy=1,ff_cxy=ff_cyx=0;
    fw_real_window(&fw,-2,-2,2,2,1);
    goto recalc;
   case VK_DIVIDE:
   case 188:if(k_ctrl|k_shift) { if(k_shift) div_pal(160,1,1);else light_pal(32,1);goto repaint;} fw_dec_maxi(&fw);goto recalc;
   case VK_MULTIPLY:
   case 190:if(k_ctrl|k_shift) { if(k_shift) div_pal(96,1,1);else light_pal(32,0);goto repaint;} fw_inc_maxi(&fw);goto recalc;
   case VK_NUMPAD4:
   case VK_NUMPAD6:
   case VK_NUMPAD1:
   case VK_NUMPAD3:
   case VK_DECIMAL:if(x>=0&&x<fw.width&&y>=0&&y<fw.height) {
    unsigned char l=fw.image[fw.width*y+x];
    if(Key!=VK_DECIMAL) {
      if(Key>=VK_NUMPAD4) cut_pal(l,Key==VK_NUMPAD4);
      else lim_pal(l,Key==VK_NUMPAD1);
    } else mix_pal(l,32);
    goto repaint;
    }
   case VK_PRIOR:fw_move_window(&fw,0,k_ctrl?-0.25:k_shift?-0.5:-1);goto recalc;
   case VK_NEXT:fw_move_window(&fw,0,k_ctrl?+0.25:k_shift?+0.5:+1);goto recalc;
   case VK_DELETE:fw_move_window(&fw,k_ctrl?-0.25:k_shift?-0.5:-1,0);goto recalc;
   case VK_END:fw_move_window(&fw,k_ctrl?+0.25:k_shift?+0.5:+1,0);goto recalc;
   case VK_RETURN:
   case VK_INSERT:fw_zoom_window(&fw,k_ctrl?k_shift?0.9375:0.875:k_shift?0.75:0.5,1);goto recalc;
   case VK_ADD:
   case VK_BACK:
   case VK_HOME:fw_zoom_window(&fw,k_ctrl?k_shift?1.0625:8:k_shift?4:2,1);goto recalc;
   case VK_LEFT:if(k_shift) fw_rotate(&fw,(k_ctrl?10:1)*M_PI/180);else fw_shift_window(&fw,k_ctrl?-100:k_alt?-1:-10,0);goto recalc;
   case VK_RIGHT:if(k_shift) fw_rotate(&fw,-(k_ctrl?10:1)*M_PI/180);else fw_shift_window(&fw,k_ctrl?+100:k_alt?+1:+10,0);goto recalc;
   case VK_UP:if(k_shift) fw_point_rotate(&fw,x,y,(k_ctrl?10:1)*M_PI/180);else fw_shift_window(&fw,0,k_ctrl?-100:k_alt?-1:-10);goto recalc;
   case VK_DOWN:if(k_shift) fw_point_rotate(&fw,x,y,-(k_ctrl?10:1)*M_PI/180);else fw_shift_window(&fw,0,k_ctrl?+100:k_alt?+1:+10);goto recalc;
   case VK_F1:PrintHelp(window);break;
   case VK_F4:if(k_alt) PostQuitMessage(0);break;
   case VK_ESCAPE:
   case ' ':
   case VK_F11:{
     WINDOWPLACEMENT wp;
     char max;
     wp.length=sizeof(wp);
     GetWindowPlacement(window,&wp);
     max=wp.showCmd!=SW_MAXIMIZE;
     if(Key==VK_ESCAPE&&max) goto quit;
     ShowWindow(window,max?SW_MAXIMIZE:SW_RESTORE);
     if(k_ctrl) {
       LONG ws=GetWindowLong(window,GWL_STYLE);
       if(max) ws&=~WS_CAPTION;else ws|=WS_CAPTION;
       SetWindowLong(window,GWL_STYLE,ws);
       SetMenu(window,NULL);
     }
    } break;
   case VK_F10:
    topmost^=1;
    SetWindowPos(window,topmost?HWND_TOPMOST:HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
    break;
   case 'M':
    SetWindowLong(window,GWL_STYLE,GetWindowLong(window,GWL_STYLE)^WS_CAPTION);
    SetMenu(window,NULL);
    goto recalc;
   case 'Q':
   quit:
    PostQuitMessage(0);
    break;
   case 'B':f_key=Key;
    ff=k_ctrl?ff_mandel64:k_shift?ff_julia64b:ff_julia64;
    goto setpt;
   case 'N':f_key=Key;
    ff=k_ctrl?ff_mandel16:k_shift?ff_julia16b:ff_julia16;
    goto setpt;
   case 'K':f_key=Key;
    ff=k_ctrl?ff_mandel4:k_shift?ff_julia4b:ff_julia4;
    //fw.func=k_shift?ff_julia16:ff_julia4;
    goto setpt;
   case 'G':f_key=Key;
    ff=k_ctrl?k_shift?ff_juliaa2:ff_mandela24:k_shift?ff_juliaa:ff_mandela;
    goto setpt;
   case 'H':f_key=Key;
    ff=k_ctrl?ff_mandel3:k_shift?ff_julia3b:ff_julia3;
    goto setpt;
   case 'L':f_key=Key;
    ff=k_ctrl?ff_mandel6:k_shift?ff_julia6b:ff_julia6;
    goto setpt;
   case VK_F3:{real m=1,a=0,c,s;
    xf3=0;
    if(k_shift) xf3=1,a=0.5;
    if(k_ctrl) xf3=1,m=1.001;
    a=a*M_PI/180,c=cos(a),s=sin(a);
    ff_cxx=m*c,ff_cxy=m*s;
    ff_cyx=-m*s,ff_cyy=m*c;
    if(k_alt) xf3=1,ff_cxx=ff_cyy=0,ff_cxy=ff_cyx=m;
    if(xf3) {
      if(fw.func==ff_mandelbrot) fw.func=ff_mandelbrotx;
      else if(fw.func==ff_julia) fw.func=ff_juliax;
      else if(fw.func==ff_juliab) fw.func=ff_juliabx;
    } else {
      if(fw.func==ff_mandelbrotx) fw.func=ff_mandelbrot;
      else if(fw.func==ff_juliax) fw.func=ff_julia;
      else if(fw.func==ff_juliabx) fw.func=ff_juliab;
    }
    } goto recalc;
   case 'J':f_key=Key;
    ff=k_ctrl?xf3?ff_mandelbrotx:ff_mandelbrot:k_shift?xf3?ff_juliabx:ff_juliab:xf3?ff_juliax:ff_julia;
   setpt:
    p1x=fw_x(&fw,x,y),p1y=fw_y(&fw,x,y);
    if(fw.func==ff&&p1x==fw.p1x&&p1y==fw.p1y) break;
    fw.func=ff;
    fw.p1x=p1x,fw.p1y=p1y;
#if LOG
    { FILE *f=fopen("out.log","a"); 
      fprintf(f,"%d,%d %f,%f %f,%f,%f,%f\n",x,y,(double)fw.p1x,(double)fw.p1y,(double)fw.ix,(double)fw.iy,(double)fw.ax,(double)fw.ay);
      fclose(f);
    }
#endif
   recalc:
    fw_recalc(&fw);
    r=1;
    break;
   case 'P':
    write_next();
    break;
/*   case 'Z':
    { char buf[256];
      sprintf(buf,"%d %d %d\n"
        ,(int)&((fractal_window*)0)->p1x,(int)&((fractal_window*)0)->p1y,(int)&((fractal_window*)0)->maxi);
      MessageBox(NULL,buf,"Y",MB_OK);
    } break; */
   case VK_F2:
   case 'F': {
      static char filename[256]="out.bmp";
      int scale=k_ctrl?(k_shift?8:4):1+k_shift;

      SaveOrCopy(filename,scale);
      k_ctrl=Ctrl(),k_shift=Shift();
    //write_bmpidx("out%03d.bmp");
    } 
    break;
   default:r=1;
      /* {
      char buf[64];
      sprintf(buf,"Key:%c [%d:0x%x] %d.%d",Key,Key,Key,x,y);
      MessageBox(NULL,buf,"key",MB_OK); 
    } */
  }
  return r;
}


void ProcessCommand(int Command) {
  switch(Command) {
   case UM_ResetWindow:
    fw_real_window(&fw,-2,-2,2,2,1);
    goto update;
   case UM_SwapTitle:
    SetWindowLong(window,GWL_STYLE,GetWindowLong(window,GWL_STYLE)^WS_CAPTION);
    SetMenu(window,NULL);
    //PostMessage(window,WM_SIZE,0,0);
    //resize_dc();
    goto update;
   default:break;
   update:
    dirty=1;//update_back();
  }; 
}

long FAR PASCAL main_WndProc (HWND hWnd, UINT iMessage,
    WPARAM wParam, LPARAM lParam) {
  int mx,my; // actual mouse x,y

  LRESULT r=0;
  k_alt=Alt();

  switch(iMessage)  {
   case WM_PAINT:
    if(bgmem) {
      PAINTSTRUCT PS;
      BeginPaint(hWnd,&PS);
      repaint();
      EndPaint(hWnd,&PS);
    } break;
   case WM_SYSKEYDOWN:
   case WM_KEYDOWN: { 
      MSG m={window,iMessage,wParam,lParam};
      if(!TranslateAccelerator(window,accel,&m)) {
        switch(wParam) {
         //case ' ':break;
         //case 'M':break;
         case VK_TAB:
          if(lb||rb) {
            POINT pt;
            XorBox(dc,rb,px,py,lx,ly);
            GetCursorPos(&pt);
            mx=px,my=py;
            px=lx,py=ly;
            lx=mx,ly=my;
            SetCursorPos(pt.x+lx-px,pt.y+ly-py);
            XorBox(dc,rb,px,py,lx,ly);
          }
          break;
        case VK_ESCAPE: 
         if(lb||rb) {
          XorBox(dc,rb,px,py,lx,ly);
          lb=rb=0;
          break;
         }
        default:
         r=ProcessKey((int)m.wParam,lx,ly);
         break;
        }
        
      }
    } break;
   case WM_COMMAND:
    ProcessCommand(LOWORD(wParam));
    break;
   case WM_SIZE:
    if(window&&wParam!=SIZE_MINIMIZED) {
      if(resize_dc())
        dirty=1;//update_back();
    }
    break;
   case WM_DESTROY:
    PostQuitMessage(0);
    break;
   case WM_LBUTTONDOWN:
   case WM_MBUTTONDOWN:
   case WM_RBUTTONDOWN:
    if(lb||rb||mb) break;
    px=LOWORD(lParam),py=HIWORD(lParam);
    pk=2*k_ctrl+k_shift;
    SetCapture(hWnd);
    lx=px,ly=py,lb=iMessage==WM_LBUTTONDOWN,rb=iMessage==WM_RBUTTONDOWN,mb=iMessage==WM_MBUTTONDOWN;
    XorBox(dc,rb,px,py,px,py);
    break;
   case WM_MOUSEMOVE:
    mx=(short)LOWORD(lParam),my=(short)HIWORD(lParam);
    if(lb||rb) {
      XorBox(dc,rb,px,py,lx,ly);
      XorBox(dc,rb,px,py,mx,my);
      //dirty=1;//update_back();
    } else if(mb) {
      fw_shift_window(&fw,lx-mx,ly-my);
      fw_recalc(&fw);
    }
    lx=mx,ly=my;
    break;
   case WM_LBUTTONUP:
    if(lb) {
      XorStop();
      mx=(short)LOWORD(lParam),my=(short)HIWORD(lParam);
      if(abs((mx-px)*(my-py))<1024) // pixel
	fw_point_zoom(&fw,mx,my,1,2,1);
      else
	fw_window(&fw,px,py,mx,my,!k_ctrl&&!(pk&2),!k_shift&&!(pk&1));
      fw_recalc(&fw);
    }
    break;
   case WM_MBUTTONUP:mb=0;break;
   case WM_RBUTTONUP:
    if(rb) {
      XorBox(dc,rb,px,py,lx,ly);
      rb=0;
      mx=(short)LOWORD(lParam),my=(short)HIWORD(lParam);
      if(k_ctrl&&(px!=mx||py!=my)) {
        double angle=atan2((ly-py),lx-px);
        if(k_shift) {
  	  fw_point_rotate(&fw,px,py,angle);
        } else {
  	  fw_rotate(&fw,angle);
        }
      } else if(abs((mx-px)*(my-py))<8) { // pixel
	fw_zoom_window(&fw,2,1);
	//fw_window(&fw,px-fw.width,py-fw.height,px+fw.width,py+fw.height);
      } else {
	fw_shift_window(&fw,px-mx,py-my);
      }
      fw_recalc(&fw);
    }
   default: {
    r=DefWindowProc(hWnd, iMessage, wParam, lParam);
   } break;
  }
  return r;
}


int loop() {
  MSG m;
  int lines;

  while(WaitMessage()) {
   peek:
    while(PeekMessage(&m,NULL,0,0,PM_REMOVE)) {
      if(m.message==WM_QUIT)
        return m.wParam;
      if(m.message==WM_KEYDOWN||m.message==WM_SYSKEYDOWN) {
        if(m.wParam==VK_SHIFT) k_shift=1;
        if(m.wParam==VK_CONTROL) k_ctrl=1;
        if(m.wParam==VK_MENU) k_alt=1;
      }
      if(m.message==WM_KEYUP||m.message==WM_SYSKEYUP) {
        if(m.wParam==VK_SHIFT) k_shift=0;
        if(m.wParam==VK_CONTROL) k_ctrl=0;
        if(m.wParam==VK_MENU) k_alt=0;
      }
      if(m.message==WM_MOUSEWHEEL) {
         short wh=m.wParam>>16;
         if(wh) Wheel(wh,m.pt,m.wParam);
         continue;
      }
      TranslateMessage(&m);
      DispatchMessage(&m);
    }
    if(dirty) {
      update_back();
      dirty=0;
    }
    if((lines=fw_calc_line(&fw))) {
      SetDIBitsToDevice(dc,0,fw.line-lines,width,lines,0,height-fw.line,
        0,height,bgmem,bif,DIB_RGB_COLORS);
      goto peek;
    }
  }
  return 0;
}

char IsWhite(char ch) { return ch==' '||ch=='\t'; }
char *ArgExt(char *h) {
  char ch;
  while(IsWhite(ch=*h)) h++;
  switch(ch) {
   case '1':case 'b':Savefi=1;break;
   case '2':case 'g':Savefi=2;break;
   case '3':case 'p':Savefi=3;break;
   case '4':case 'a':Savefi=4;break;
  }
  return h;
}

char *ArgInt(char *h,int *x,int lo,int hi) {
  int i=0;
  char *h2;
  while(IsWhite(*h)) h++;
  h2=h;
  while(*h>='0'&&*h<='9') i=i*10+(*h++)-'0';
  if(h>h2) 
    *x=i<lo?lo:i>hi?hi:i;
  return h;
}

int PASCAL WinMain(HINSTANCE hCurInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow) {
  int r,threads=2;
  char *h,fullscr=0,aa=0,fast=0;
  
  /*{ char buffer[64];
    sprintf(buffer,"%d\n",(char*)&fw.p1y-(char*)&fw);
    MessageBox(NULL,buffer,"title",MB_OK);
  }*/
  for(h=lpCmdLine;*h;) {
    while(IsWhite(*h)) h++;
    if(*h=='-') {
      for(h++;*h&&!IsWhite(*h);h++) 
        switch(*h) {
         case 'f':fullscr=1;break;
         case 't':topmost=1;break;
         case 'a':aa=aa?3:2;break;
         case 's':fast=fast==8?16:fast?8:4;break;
         case 'e':h=ArgExt(h+1);break;
         case 'c':h=ArgInt(h+1,&threads,1,32);break;
         case '?':
         case 'h':PrintHelp(NULL);break;
        }
    } else while(*h&&!IsWhite(*h)) h++; 
  }
  hinstance=hCurInstance;
  create_window(fullscr);
  fw_init(&fw,1,1,NULL,threads-1);
  fw.aa=aa;fw.fast=fast;
  update_back();
  r=loop();
  fw.run=0;

  return r;
}

