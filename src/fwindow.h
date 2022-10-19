
typedef int(*fractal_function)(real,real);


struct  s_fractal_window {
  real p1x,p1y;  // startovaci bod pro julii (julia2) (+76,+88)
  int maxi;      // maximalni pocet iteraci (+72)
  fractal_function func;
  uchar (*f2)(int,int);
  real sx,sy,wx,asp;
  double angle;
  int width,height;
  volatile int line,run;
  uchar *image;
  void **doit,*ok,**done;
  char aa,fast,recalc;
  int threads;
};

typedef struct s_fractal_window fractal_window;

/* fractal_functions */

int ff_mandelbrot(real x,real y);
int ff_juliab(real x, real y);
int ff_julia(real x, real y);
int ff_mandel3(real x, real y);
int ff_julia3(real x, real y);
int ff_julia3b(real x, real y);
int ff_mandel4(real x, real y);
int ff_julia4(real x, real y);
int ff_julia4b(real x, real y);
int ff_mandel6(real x, real y);
int ff_julia6(real x, real y);
int ff_julia6b(real x, real y);
int ff_mandel16(real x, real y);
int ff_julia16(real x, real y);
int ff_julia16b(real x, real y);
int ff_mandel64(real x, real y);
int ff_julia64(real x, real y);
int ff_julia64b(real x, real y);
int ff_newton(real x, real y);
int ff_mandela(real x, real y);
int ff_mandela24(real x, real y);
int ff_juliaa(real x, real y);
int ff_juliaa2(real x, real y);

int ff_mandelbrotx(real x,real y);
int ff_juliabx(real x, real y);
int ff_juliax(real x, real y);
/* int2uchar functions */

uchar i2u_mod(int i,int maxi);
uchar i2u_lin(int i,int maxi);

void fw_recalc(fractal_window *fw);
int fw_calc_line(fractal_window *fw);
void fw_resize(fractal_window *fw,int width,int height,void *data);
void fw_real_window(fractal_window *fw,real ix,real iy,real ax,real ay,char aspect);
void fw_move_window(fractal_window *fw,double mx,double my);
void fw_zoom_window(fractal_window *fw,double mul,double div);
void fw_point_zoom(fractal_window *fw,int x,int y,double mul,double div,char center);
void fw_shift_window(fractal_window *fw,int sx,int sy);
void fw_init(fractal_window *fw,int width,int height,void *data,int threads);
int fw_done(fractal_window *fw);
real fw_x(fractal_window *fw,int x,int y);
real fw_y(fractal_window *fw,int x,int y);
void fw_window(fractal_window *fw,int ix,int iy,int ax,int ay,char abs,char aspect);
void fw_rotate(fractal_window *fw,double angle);
void fw_point_rotate(fractal_window *fw,int x,int y,double angle);
void fw_flip(fractal_window *fw,char x,char y);
void fw_inc_maxi(fractal_window *fw);
void fw_dec_maxi(fractal_window *fw);
void fw_change_i2u(fractal_window *fw);

extern fractal_window *sfw;
extern real ff_cxx,ff_cxy,ff_cyx,ff_cyy;
