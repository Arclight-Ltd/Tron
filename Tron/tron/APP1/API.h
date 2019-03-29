
void putstr0(char*i);
void putchar(char i);
void endapp(void);
int getfifo();
int window(char *buf, int xsize, int ysize, int color, char *title);
void putstrwin(int winsheet, int x, int y, int color, int len, char *str);
void squerwin(int x0, int x1, int y0,  int y1, int winsheet, int color);
void closewin(int win);
void initmalloc(void);
char *malloc(int size);
void free(char *addr, int size);
int timeralloc(unsigned int timeout,char data);
void timeron(unsigned int timer);
void timeroff(unsigned int timer);
