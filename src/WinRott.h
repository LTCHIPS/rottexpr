

// winrott.h


//husk at rette i winrott.c

#define MAXSCREENWIDTH 3840
#define MAXSCREENHEIGHT 2160 //max res is set to be 4k


extern int iGLOBAL_SCREENWIDTH;//bna val 800
extern int iGLOBAL_SCREENHEIGHT;//bna val 600

//extern int topBarCenterOffsetX;

extern int iGLOBAL_SCREENBWIDE ;
extern int iG_SCREENWIDTH;// default screen width in bytes

extern int iGLOBAL_HEALTH_X;
extern int iGLOBAL_HEALTH_Y;
extern int iGLOBAL_AMMO_X;
extern int iGLOBAL_AMMO_Y;

extern int iGLOBAL_FOCALWIDTH;
extern double dGLOBAL_FPFOCALWIDTH;

void EnableScreenStretch(void);
void DisableScreenStretch(void);
void WriteNewResolution(void);
void RecalculateFocalWidth(void);
/*
double dYZANGLELIMIT;

#define FOCALWIDTH 160//160
#define FPFOCALWIDTH 160.0//160.0



#define MAXSCREENHEIGHT    480//600//     200*2
#define MAXSCREENWIDTH     640//800//     320*2
#define SCREENBWIDE        640*(96/320)//800*(96/320)//     96*2
#define MAXVIEWWIDTH       640//     320*2
#define SCREENWIDTH        640*(96/320)//800*(96/320)//     96*2              // default screen width in bytes


#define MAXSCREENHEIGHT    600
#define MAXSCREENWIDTH     800
#define SCREENBWIDE        800*(96/320)
#define MAXVIEWWIDTH       800
#define SCREENWIDTH        800*(96/320)// default screen width in bytes

*/
//#define VIEWWIDTH               MAXSCREENWIDTH//320*2             // size of view window
//#define VIEWHEIGHT              MAXSCREENHEIGHT//200*2
//#define MAXSCANLINES            MAXSCREENHEIGHT//200*2             // size of ylookup table







