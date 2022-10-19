#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include "fwindow.h"
#include "math.h"
#include "windows.h"

/* fractal functions */

#define BOUND 4

fractal_window *sfw;

/*int ff_newton(real x,real y) {
  register real zx,zy,zx2,zy2,r0,r1,r2;
  zx=x,zy=y;
  register int i=sfw->maxi;
  do {
    zx2=zx*zx;
    zy2=zy*zy;
    r0=zx2-zy2-1;
    if(r0*r0+4*zx2*zy2<1e-20) break;
    r2=4*(zx2+zy2);
    zx=2*zx*(r0+2*zy2)/r2,zy=2*zy*(2*zx2-r0)/r2;
  } while(--i);
  return i;
}*/
real ff_cxx=1,ff_cxy=0,ff_cyx=0,ff_cyy=1;
int ff_mandela(real x,real y) {
  register real zx,zy,cx,cy,cx2,cy2,r0,r1;
  int i;

  cx2=sfw->p1x,cy2=sfw->p1y;
  cx=zx=x,cy=zy=y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    zy*=zx;
    zy+=zy+cy;
    zx=r0-r1+cx;
    if(!--i) break;
    r0=cx,cx=ff_cxx*cx+ff_cxy*cy;
    cy=ff_cyx*r0+ff_cyy*cy;
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    zy*=zx;
    zy+=zy+cy2;
    zx=r0-r1+cx2;
  } while(--i);
  return i;
}

int ff_mandela24(real x,real y) {
  register real zx,zy,cx,cy,cx2,cy2,r0,r1;
  int i;

  cx2=sfw->p1x,cy2=sfw->p1y;
  cx=zx=x,cy=zy=y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    zy*=zx;
    zy+=zy+cy;
    zx=r0-r1+cx;
    if(!--i) break;
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    r0=r0-r1;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1+cx2;
    zy=2*r0*r1+cy2;
    r0=cx,cx=ff_cxx*cx+ff_cxy*cy;
    cy=ff_cyx*r0+ff_cyy*cy;
  } while(--i);
  return i;
}

int ff_juliaa(real x,real y) {
  register real zx,zy,cx,cy,cx2,cy2,r0,r1;
  int i;

  zx=cx2=x,zy=cy2=y;
  cx=sfw->p1x,cy=sfw->p1y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    zy*=zx;
    zy+=zy+cy;
    zx=r0-r1+cx;
    if(!--i) break;
    r0=cx,cx=ff_cxx*cx+ff_cxy*cy;
    cy=ff_cyx*r0+ff_cyy*cy;
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    zy*=zx;
    zy+=zy+cy2;
    zx=r0-r1+cx2;
  } while(--i);
  return i;
}

int ff_juliaa2(real x,real y) {
  register real zx,zy,cx,cy,cx2,cy2,r0,r1;
  int i;

  cx2=x,zy=cy2=y;
  zx=cx=sfw->p1x,zy=cy=sfw->p1y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    zy*=zx;
    zy+=zy+cy;
    zx=r0-r1+cx;
    if(!--i) break;
    r0=cx,cx=ff_cxx*cx+ff_cxy*cy;
    cy=ff_cyx*r0+ff_cyy*cy;
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    zy*=zx;
    zy+=zy+cy2;
    zx=r0-r1+cx2;
  } while(--i);
  return i;
}

int ff_juliaa24(real x,real y) {
  register real zx,zy,cx,cy,cx2,cy2,r0,r1;
  int i;

  cx2=x,cy2=y;
  zx=cx=sfw->p1x,zy=cy=sfw->p1y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    zy*=zx;
    zy+=zy+cy;
    zx=r0-r1+cx;
    if(!--i) break;
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    zy*=zx;
    zy+=zy+cy2;
    zx=r0-r1+cx2;
  } while(--i);
  return i;
}

int ff_mandel3(real x,real y) {
  register real zx,zy,cx,cy,r0,r1,r2;
  int i;
  cx=zx=x,cy=zy=y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    r0=r0-r1;
    r1=2*zx*zy;
    r2=zx;
    zx=r0*r2-r1*zy+cx;
    zy=r0*zy+r1*r2+cy;
  } while(--i);
  return i;
}

int ff_julia3(real x,real y) {
  register real zx,zy,cx,cy,r0,r1,r2;
  int i;
  zx=x,zy=y;
  cx=sfw->p1x,cy=sfw->p1y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    r0=r0-r1;
    r1=2*zx*zy;
    r2=zx;
    zx=r0*r2-r1*zy+cx;
    zy=r0*zy+r1*r2+cy;
//    zx=r0*r0-r1*r1+cx;
//    zy=2*r0*r1+cy;
  } while(--i);
  return i;
}

int ff_julia3b(real x,real y) {
  register real zx,zy,cx,cy,r0,r1,r2;
  int i;
  cx=x,cy=y;
  zx=sfw->p1x,zy=sfw->p1y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    r0=r0-r1;
    r1=2*zx*zy;
    r2=zx;
    zx=r0*r2-r1*zy+cx;
    zy=r0*zy+r1*r2+cy;
//    zx=r0*r0-r1*r1+cx;
//    zy=2*r0*r1+cy;
  } while(--i);
  return i;
}

int ff_mandel6(real x,real y) {
  register real zx,zy,cx,cy,r0,r1,r2;
  int i;
  cx=zx=x,cy=zy=y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    r0=r0-r1;
    r1=2*zx*zy;
    r2=zx;
    zx=r0*r2-r1*zy;
    zy=r0*zy+r1*r2;
    r2=zx;
    zx=zx*zx-zy*zy+cx;
    zy=2*r2*zy+cy;
  } while(--i);
  return i;
}

int ff_julia6(real x,real y) {
  register real zx,zy,cx,cy,r0,r1,r2;
  int i;
  zx=x,zy=y;
  cx=sfw->p1x,cy=sfw->p1y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    r0=r0-r1;
    r1=2*zx*zy;
    r2=zx;
    zx=r0*r2-r1*zy;
    zy=r0*zy+r1*r2;
    r2=zx;
    zx=zx*zx-zy*zy+cx;
    zy=2*r2*zy+cy;
  } while(--i);
  return i;
}

int ff_julia6b(real x,real y) {
  register real zx,zy,cx,cy,r0,r1,r2;
  int i;
  cx=x,cy=y;
  zx=sfw->p1x,zy=sfw->p1y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    r0=r0-r1;
    r1=2*zx*zy;
    r2=zx;
    zx=r0*r2-r1*zy;
    zy=r0*zy+r1*r2;
    r2=zx;
    zx=zx*zx-zy*zy+cx;
    zy=2*r2*zy+cy;
  } while(--i);
  return i;
}

int ff_mandel4(real x,real y) {
  register real zx,zy,cx,cy,r0,r1;
  int i;
  zx=cx=x,zy=cy=y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    r0=r0-r1;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1+cx;
    zy=2*r0*r1+cy;
  } while(--i);
  return i;
}

int ff_julia4(real x,real y) {
  register real zx,zy,cx,cy,r0,r1;
  int i;
  zx=x,zy=y;
  cx=sfw->p1x,cy=sfw->p1y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    r0=r0-r1;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1+cx;
    zy=2*r0*r1+cy;
  } while(--i);
  return i;
}

int ff_julia4b(real x,real y) {
  register real zx,zy,cx,cy,r0,r1;
  int i;
  cx=x,cy=y;
  zx=sfw->p1x,zy=sfw->p1y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    r0=r0-r1;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1+cx;
    zy=2*r0*r1+cy;
  } while(--i);
  return i;
}

int ff_mandel16(real x,real y) {
  register real zx,zy,cx,cy,r0,r1;
  int i;
  cx=zx=x,cy=zy=y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    r0=r0-r1;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1;
    zy=2*r0*r1;
    r0=zx*zx-zy*zy;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1+cx;
    zy=2*r0*r1+cy;
  } while(--i);
  return i;
}

int ff_julia16(real x,real y) {
  register real zx,zy,cx,cy,r0,r1;
  int i;
  zx=x,zy=y;
  cx=sfw->p1x,cy=sfw->p1y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    r0=r0-r1;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1;
    zy=2*r0*r1;
    r0=zx*zx-zy*zy;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1+cx;
    zy=2*r0*r1+cy;
  } while(--i);
  return i;
}

int ff_julia16b(real x,real y) {
  register real zx,zy,cx,cy,r0,r1;
  int i;
  cx=x,cy=y;
  zx=sfw->p1x,zy=sfw->p1y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    r0=r0-r1;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1;
    zy=2*r0*r1;
    r0=zx*zx-zy*zy;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1+cx;
    zy=2*r0*r1+cy;
  } while(--i);
  return i;
}

int ff_mandel64(real x,real y) {
  register real zx,zy,cx,cy,r0,r1;
  int i;
  cx=zx=x,cy=zy=y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    r0=r0-r1;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1;
    zy=2*r0*r1;
    r0=zx*zx-zy*zy;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1+cx;
    zy=2*r0*r1+cy;
    r0=zx*zx-zy*zy;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1+cx;
    zy=2*r0*r1+cy;
  } while(--i);
  return i;
}

int ff_julia64(real x,real y) {
  register real zx,zy,cx,cy,r0,r1;
  int i;
  zx=x,zy=y;
  cx=sfw->p1x,cy=sfw->p1y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    r0=r0-r1;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1;
    zy=2*r0*r1;
    r0=zx*zx-zy*zy;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1+cx;
    zy=2*r0*r1+cy;
    r0=zx*zx-zy*zy;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1+cx;
    zy=2*r0*r1+cy;
  } while(--i);
  return i;
}

int ff_julia64b(real x,real y) {
  register real zx,zy,cx,cy,r0,r1;
  int i;
  cx=x,cy=y;
  zx=sfw->p1x,zy=sfw->p1y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    r0=r0-r1;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1;
    zy=2*r0*r1;
    r0=zx*zx-zy*zy;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1+cx;
    zy=2*r0*r1+cy;
    r0=zx*zx-zy*zy;
    r1=2*zx*zy;
    zx=r0*r0-r1*r1+cx;
    zy=2*r0*r1+cy;
  } while(--i);
  return i;
}


int ff_mandelbrotx(real x,real y) {
  register real zx,zy,cx,cy,r0,r1;
  int i;
  cx=zx=x,cy=zy=y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    zy*=zx;
    zy+=zy+cy;
    zx=r0-r1+cx;
    r0=cx,cx=ff_cxx*cx+ff_cxy*cy;
    cy=ff_cyx*r0+ff_cyy*cy;
  } while(--i);
  return i;
}

int ff_juliabx(real x, real y) {
  register real zx,zy,cx,cy,r0,r1;
  int i;
  cx=x,cy=y;
  zx=sfw->p1x,zy=sfw->p1y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    zy*=zx;
    zy+=zy+cy;
    zx=r0-r1+cx;
    r0=cx,cx=ff_cxx*cx+ff_cxy*cy;
    cy=ff_cyx*r0+ff_cyy*cy;
  } while(--i);
  return i;
}

int ff_juliax(real x, real y) {
  register real zx,zy,cx,cy,r0,r1;
  int i;
  zx=x,zy=y;
  cx=sfw->p1x,cy=sfw->p1y;
  i=sfw->maxi;
  do {
    r0=zx*zx;
    r1=zy*zy;
    if(r0+r1>4) break;
    zy*=zx;
    zy+=zy+cy;
    zx=r0-r1+cx;
    r0=cx,cx=ff_cxx*cx+ff_cxy*cy;
    cy=ff_cyx*r0+ff_cyy*cy;
  } while(--i);
  return i;
}

/* int2uchar functions */

uchar i2u_mod(int i,int maxi) { return  i;}
uchar i2u_lin(int i,int maxi) { return i*255/maxi;}

void fw_recalc(fractal_window *fw) {
  fw->recalc=1;
  //fw->line=0;
}


void fw_calc_linei(fractal_window *fw,int line) {
  double co=cos(fw->angle),si=sin(fw->angle);
  register real sx,sy,dxx,dxy,dyx,dyy,cx,cy;
  unsigned char *p,*q,r,aa=fw->aa,fast=fw->fast,g=0;
  int j,s,w=fw->width,h=fw->height;
  p=fw->image+line*w;
  if(fast<2) fast=0;
  else {
    aa=0;
    g=fast-1;
    while(g&&line+g>=fw->height) g--;
  }
  if(fast&&(line%fast)) return;
  sx=fw->sx-co*fw->wx+si*fw->wx*h/w*fw->asp,sy=fw->sy-si*fw->wx-co*fw->wx*h/w*fw->asp;
  dxx=2*co*fw->wx*256/w,dxy=-2*si*fw->wx/w*fw->asp*256;
  dyx=2*si*fw->wx*256/w,dyy=+2*co*fw->wx/w*fw->asp*256;
  for(j=0;j<fw->width;) {
        s=fw->func(cx=sx+(dxx*j+dxy*line)/256,cy=sy+(dyx*j+dyy*line)/256);
        if(aa) {
          if(aa>3) {
            int ax,ay;
            for(ax=0;ax<aa;ax++)
              for(ay=ax?0:1;ay<aa;ay++)
	        s+=fw->func(cx+(ax*dxx+ay*dyx)/aa/256,cy+(ax*dxy+ay*dyy)/aa/256);
            s=(s+((aa*aa-1)/2))/aa/aa;
          } else {
	    int d=aa==3?768:512;
	    s+=fw->func(cx+dxx/d,cy+dxy/d);
	    s+=fw->func(cx+dyx/d,cy+dyy/d);
	    s+=fw->func(cx+(dxx+dyx)/d,cy+(dxx+dyx)/d);
	    if(aa==3) {
	      s+=fw->func(cx+2*dyx/768,cy+2*dxy/768);
	      s+=fw->func(cx+(dxx+2*dyx)/768,cy+(dxy+2*dyy)/768);
	      s+=fw->func(cx+3*dxx/768,cy+3*dxy/768);
	      s+=fw->func(cx+(2*dxx+dyx)/768,cy+(2*dxy+dyy)/768);
	      s+=fw->func(cx+(2*dxx+2*dyx)/768,cy+(2*dxy+2*dyy)/768);
	      s=(s+5)/9;
	    } else
	      s=(s+2)/4;
          }
        }
        r=255-fw->f2(s,fw->maxi);
	*p++=r;j++;
        if((s=fast)) {
          for(;--s&&j<w;j++) *p++=r;
          for(q=p-fast+w,s=g;s;s--,q+=w)
            memset(q,r,fast);
        }
         
  }
}

DWORD WINAPI fw_calc_thread(LPVOID param) {
  fractal_window *fw=(fractal_window*)param;
  int id=fw->threads,line;
  SetEvent(fw->ok);
  while(1) {
    WaitForSingleObject(fw->doit[id],INFINITE);
    if(!fw->run) break;
    line=fw->line;
    SetEvent(fw->ok);
    if(line<fw->height) fw_calc_linei(fw,line);
    SetEvent(fw->done[id]);
  }
  ExitThread(0);
  return 0;
}
int fw_calc_line(fractal_window *fw) {
  int line,l2,h=fw->height,n=0,i=fw->fast<2?1:fw->fast;
  if(fw->recalc) {
    l2=fw->recalc=0;
  } else if((l2=fw->line)>=h)
    return 0;
  //printf("line %d\n",fw->line);
  line=l2;
  sfw=fw;
  if(fw->doit) 
    for(;n<fw->threads&&line<h;n++) {
      fw->line=line;
      SetEvent(fw->doit[n]);
      WaitForSingleObject(fw->ok,INFINITE);
      line+=i;
    }
  if(line<h) {
    fw_calc_linei(fw,line);
    line+=i;
  }
  if(line>h) line=h;
  fw->line=line;
  if(n>0) {
    //for(i=0;i<n;i++) WaitForSingleObject((HANDLE)fw->done[i],INFINITE);
    WaitForMultipleObjects(n,fw->done,1,INFINITE);
  }
  return line-l2;
}


void fw_resize(fractal_window *fw,int width,int height,void *data) {
  fw->width=fw->height=0;
  fw->image=data;
  fw->width=width;
  fw->height=height;
  fw_recalc(fw);
}

void fw_real_window(fractal_window *fw,real ix,real iy,real ax,real ay,char aspect) {
  fw->sx=(ix+ax)/2,fw->sy=(iy+ay)/2;
  fw->wx=(ax-ix)/2;fw->asp=(ay-iy)/2/fw->wx*fw->width/fw->height;
  if(aspect) {
    double a=fw->asp;
    fw->asp=fw->asp<0?-1:1;
    if(a<-1||a>1)
      fw->wx*=abs(a);
    /*real dx=fw->wx,dy=fw->hy;
    int nx=dx<0?-1:1,ny=dy<0?-1:1;
    dx*=nx;dy*=ny;
    ax=dy*fw->width/fw->height;
    if(dx<ax) fw->wx=ax*nx;
    else fw->hy=dx*fw->height/fw->width*ny;*/
  }
  fw->angle=0;
}

void fw_move_window(fractal_window *fw,double mx,double my) {
  double co=cos(fw->angle),si=sin(fw->angle);
  fw->sx+=mx*2*co*fw->wx-my*2*si*fw->wx*fw->height/fw->width;
  fw->sy+=mx*2*si*fw->wx+my*2*co*fw->wx*fw->height/fw->width;
}

void fw_zoom_window(fractal_window *fw,double mul,double div) {
  fw->wx=fw->wx*(mul?mul:1)/(div?div:1);
}


void fw_shift_window(fractal_window *fw,int sx,int sy) {
  double co=cos(fw->angle),si=sin(fw->angle);
  int w=fw->width;
  fw->sx+=2*sx*co*fw->wx/w-2*sy*si*fw->wx*fw->asp/w;
  fw->sy+=2*sx*si*fw->wx/w+2*sy*co*fw->wx*fw->asp/w;
}


void fw_point_zoom(fractal_window *fw,int x,int y,double mul,double div,char center) {
  double co=cos(fw->angle),si=sin(fw->angle);
  int w=fw->width,h=fw->height,rx=2*x-w,ry=2*y-h;
  fw->sx+=rx*co*fw->wx/w-ry*si*fw->wx/w*fw->asp;
  fw->sy+=rx*si*fw->wx/w+ry*co*fw->wx/w*fw->asp;
  fw_zoom_window(fw,mul,div);
  if(!center) {
    fw->sx-=rx*co*fw->wx/w-ry*si*fw->wx/w*fw->asp;
    fw->sy-=rx*si*fw->wx/w+ry*co*fw->wx/w*fw->asp;
  }
}

int bitcount(unsigned x) {
  int bc=0;
  while(x>0) bc+=x&1,x>>=1;
  return bc;
}
void fw_init(fractal_window *fw,int width,int height,void *data,int threads) {
  DWORD pam,sam;
  fw->func=ff_mandelbrot;
  fw->f2=i2u_lin;
  fw->line=0;
  fw->image=0;
  fw_real_window(fw,-2,-2,2,2,1);
  fw->maxi=255;
  fw->p1x=0;
  fw->p1y=0;
  fw->run=1;
  fw->threads=fw->recalc=0;
  if(threads>0&&GetProcessAffinityMask(GetCurrentProcess(),&pam,&sam)&&bitcount(pam)>1) { 
    HANDLE thread;
    DWORD i,thid;
    fw->doit=malloc(threads*sizeof(*fw->done));
    fw->ok=CreateEvent(NULL,FALSE,FALSE,NULL);
    fw->done=malloc(threads*sizeof(*fw->done));
    for(i=0;i<threads;i++) {
      fw->doit[i]=CreateEvent(NULL,FALSE,FALSE,NULL);
      fw->done[i]=CreateEvent(NULL,FALSE,FALSE,NULL);
      thread=CreateThread(NULL,0,fw_calc_thread,fw,0,(DWORD*)&thid);
      CloseHandle(thread);
      WaitForSingleObject(fw->ok,INFINITE);
      fw->threads++;
    }
  } else 
    fw->doit=fw->done=NULL;
  //MessageBox(NULL,fw->doit?"dual core":"single core","paralelism",MB_OK); 
  fw_resize(fw,width,height,data);
}

int fw_done(fractal_window *fw) {
  return fw->line>=fw->height;
}

real fw_x(fractal_window *fw,int x,int y) {
  double co=cos(fw->angle),si=sin(fw->angle);
  int w=fw->width,h=fw->height;
  return fw->sx+(2*x-w)*co*fw->wx/w-(2*y-h)*si*fw->wx*fw->asp/w;
}
real fw_y(fractal_window *fw,int x,int y) {
  double co=cos(fw->angle),si=sin(fw->angle);
  int w=fw->width,h=fw->height;
  return fw->sy+(2*x-w)*si*fw->wx/w+(2*y-h)*co*fw->wx*fw->asp/w;
}

void fw_window(fractal_window *fw,int ix,int iy,int ax,int ay,char abs,char aspect) {
  double co=cos(fw->angle),si=sin(fw->angle),a;
  int r,w=fw->width,h=fw->height,sx=ix+ax-w,sy=iy+ay-h,dx,dy;
  if(abs) {
    if(ax<ix) r=ix,ix=ax,ax=r;
    if(ay<iy) r=iy,iy=ay,ay=r;
  }
  dx=ax-ix,dy=ay-iy;
  /*if(aspect) {
    if(dx<0) dx=-dx;if(dy<0) dy=-dy;
    dx2=dy*w/h,dy2=dx*h/w;
    if(dx2>dx) rx=1.0*dx2/dx;
    else ry=1.0*dy2/dy;
  }*/
  fw->sx+=sx*co*fw->wx/w-sy*si*fw->wx/w*fw->asp;
  fw->sy+=sx*si*fw->wx/w+sy*co*fw->wx/w*fw->asp;
  fw->wx=fw->wx*dx/w;

  a=1.0*dy/h/dx*w;
  if(aspect) {
    if(dy<0)
      fw->asp*=-1;
    if(a<-1||a>1)
      fw->wx*=a<0?-a:a;
  } else fw->asp*=a;
  fw_recalc(fw);
}

void fw_rotate(fractal_window *fw,double angle) {
  fw->angle+=angle*(fw->asp<0?-1:1);
}

void fw_point_rotate(fractal_window *fw,int x,int y,double angle) {
  double co=cos(fw->angle),si=sin(fw->angle);
  int w=fw->width,h=fw->height,sx=2*x-w,sy=2*y-h;
  fw->sx+=sx*co*fw->wx/w-sy*si*fw->wx*fw->asp/w;
  fw->sy+=sx*si*fw->wx/w+sy*co*fw->wx*fw->asp/w;
  fw_rotate(fw,angle);
  co=cos(fw->angle),si=sin(fw->angle);
  fw->sx-=sx*co*fw->wx/w-sy*si*fw->wx*fw->asp/w;
  fw->sy-=sx*si*fw->wx/w+sy*co*fw->wx*fw->asp/w;
}

void fw_flip(fractal_window *fw,char x,char y) {
  uchar u,*h,*g;
  int i,w=fw->width;
  if(x) {
    //fw->wx*=-1;
    fw->angle+=M_PI;
    fw->asp*=-1;
    for(i=0;i<fw->height;i++) 
      for(h=fw->image+i*w,g=h+w-1;h<g;h++,g--)
        u=*h,*h=*g,*g=u;
  }
  if(y) {
    fw->asp*=-1;
    for(i=0;i<w;i++) 
      for(h=fw->image+i,g=h+w*(fw->height-1);h<g;h+=w,g-=w)
        u=*h,*h=*g,*g=u;
  }
}

void fw_inc_maxi(fractal_window *fw) {
  fw->maxi*=2;
  fw_recalc(fw);
}

void fw_dec_maxi(fractal_window *fw) {
  if(fw->maxi>1) {
	fw->maxi/=2;
	fw_recalc(fw);
  }
}

void fw_change_i2u(fractal_window *fw) {
  fw->f2=fw->f2==i2u_mod?i2u_lin:i2u_mod;
  fw_recalc(fw);
}

