/*-----------------------------------------------------------------------------------------*/
// Gui.c
// Orignal (c) MNS 1998
// revised (c) MNS 1999
// revised (C) MNS 2000
/*-----------------------------------------------------------------------------------------*/
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include "AddressBus.h"

#include "gfx.h"
#define MAX_ENTRIES 10
#define MIN_ENTRIES 1;
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned int UINT;

struct mns_Items {
    char *name;    //item name
    struct mns_Items *previous;      //prev item
    struct mns_Items *next;       //next item
};

struct mns_Menu {
    char* title;
    struct mns_Menu *next;         //next menu
    struct mns_Items *first;       //first item
    struct mns_Items *last;       //last item
    unsigned int select,entries;  //cursor
    unsigned int menuid;
    unsigned int xcoord,ycoord;   //filled by menubar
    unsigned int menuWidth,height;
    unsigned int titleWidth;
};

struct mns_MenuBase {
    char *name;
    struct mns_Menu *first;        //first menubase
    struct mns_Menu *last;         //last menubase
    unsigned int *memBuffer;     //background buffer1
    unsigned int select,entries,page;  //cursor
    unsigned int baseid,xcord,ycord;
    unsigned int width,height;
};

struct filer {
    unsigned char name[260];
    unsigned int size;
    struct filer *next;
    struct filer *previous;
};

struct fileselector {
    struct filer *first;
    struct filer *last;
    struct filer *current;
    unsigned int select,scroller,numfiles;
    unsigned int msg;
    unsigned int size;
    unsigned int xcoord,ycoord;
    unsigned char filename[260];
    unsigned char path[260];
};


struct mns_MenuBase mns_baseMenu;  //pointer to menus

void mns_drawFiles(struct fileselector *F);
void mns_fileselecter(struct fileselector *F);
unsigned int mns_handleFiler(struct fileselector *F);
void grabMenuBar(void);
void replaceMenuBar(void);
unsigned int mns_handleMenu(unsigned int m) ;
void mns_installMenuBar(unsigned int aWidth,unsigned int aHeight) ;
void mns_uninstallMenuBar(void);

/*-----------------------------------------------------------------------------------------*/
void mycpystr(char *s1,char *s2){
    char *l1, *l2;
    l1=s1;
    l2=s2;
    while(*l2!=0){
        *l1++=*l2++;
    }
    *l1=0;
}

void mns_drawFiles(struct fileselector* F) {
	unsigned int x=F->xcoord;
	unsigned int y=F->ycoord;
	struct filer* I=F->first;
	unsigned int numfiles=F->numfiles;
	unsigned int select=F->select;
    unsigned int indexx=0;//count the number of items printed
	//while theres another item to print
    while(indexx<numfiles) {
		//is this item the current item
        if(indexx==select) {
            mns_rectangle(x,y,364,8,1);	//draw black border
            mns_invertColors();			//invert colours
            mns_textOut(x,y,I->name);	//print inverted text
            mns_invertColors();			//retore colours
        }
        else {
            mns_invertColors();
            mns_rectangle(x,y,364,8,1);
            mns_invertColors();
            mns_textOut(x,y,I->name);
        }
        I=I->next;						//next item
        indexx++;
        y+=8;							//next line
    }
}

unsigned int mns_handleFiler(struct fileselector *F) {
    SDL_Event event;

    int done,msg,GUIKey;
    msg=GUIKey=0;
    done=0xff;
    grabMenuBar();
    F->current=F->first;
    F->select=0;
    while(done) {
        F->xcoord=0;
        F->ycoord=0;
        mns_setColors(cWhite,cBlack);
        mns_clearScreen();
        mns_rectangle(F->xcoord,F->ycoord,368,255,0);
        F->xcoord+=2;
        F->ycoord+=2;
//        strcpy(F->filename,F->current->name);
        mns_drawFiles(F);
        mns_swapScreens();

        msg=GUIKey=0;
        if(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                switch(event.key.keysym.sym ) {
                    case SDLK_DOWN:
                    if(F->current>=F->last) {
                        F->current=F->first;
                        F->select=0;
                    }
                    else {
                        F->current=F->current->next;
                        F->select++;
                    }
                    break;
                    case SDLK_UP:
                    if(F->current==F->first) {
                        F->current=F->last;
                        F->select=F->numfiles-1;
                    }
                    else {
                        F->current=F->current->previous;
                        F->select--;
                    }
                    break;
                    case SDLK_RETURN:
                    strcpy(F->filename,F->current->name);
                    msg=0xf00f;
                    done=0;
                    break;
                    case SDLK_F9:
                    msg=0xeeee;
                    F->msg=msg;
                    done=0x0;
                    break;
                }// end inner switch
                break;
                case SDL_KEYUP:
                // If escape is pressed, return (and thus, quit)
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    msg=0xdead;
                    done=0;
                }
                break;
                case SDL_QUIT:
                msg=0xdead;
                done=0;
                break;
            }// end outter switch
        }// end if
    }// end while
    replaceMenuBar();
    return(msg);
}

//fileselector

//find files in this directory
void mns_fileselecter(struct fileselector *F) {
    struct _finddata_t blk;
    struct filer *currentfiler,*firstfiler,*tempfiler;
    long fhandle,done;
    unsigned int error=0;
    unsigned int numfiles=0;
    done=0;
    currentfiler=firstfiler=tempfiler=0; // intialise pointers
    done=fhandle = _findfirst(F->path,&blk);  // find first file
    if(fhandle!=-1) {
        while ((done==0)&&(error!=-1)) {
            currentfiler=(struct filer*) malloc(sizeof(struct filer));
            // is this the first file
            if((firstfiler==0)&&(currentfiler)) {
                firstfiler=currentfiler;
                firstfiler->previous=0;                 //no previous files
            }

            // did we find a file
            if(currentfiler) {
                strcpy(currentfiler->name,blk.name);    //copy filename
                currentfiler->size=blk.size;        //copy attributes
                currentfiler->previous=tempfiler;       //previous file
                if(tempfiler) {
                    tempfiler->next=currentfiler;
                }
                tempfiler=currentfiler;
                numfiles++;                             //increase files found
                done=_findnext(fhandle,&blk);           //find the next file
            }
            else {
                //there was an error
                error=-1;
            }
        }//end while
        done=_findclose(fhandle);
        F->first=firstfiler;
        F->last=currentfiler;
        F->current=firstfiler;
        F->numfiles=numfiles;
        F->select=0;
        if(error!=-1) {
            error=mns_handleFiler(F);
        }
        else{
            error=-1;
        };
        currentfiler=firstfiler;
        while(currentfiler) {
            tempfiler=currentfiler->next;
            free(currentfiler);
            currentfiler=tempfiler;
        }

        if(currentfiler) {
            free(currentfiler);
        }

    }// end of outter if
    else {
        error=-1;
    }
    F->msg=error;
}

/*---------------------------------------------------------*/
/* Initialise linked list of menu items                    */
/*---------------------------------------------------------*/
unsigned int count_width(char *aString) {
    unsigned int count=0;
    while(*aString++!=0) {
        count++;
    }
    return count;
}

unsigned int install_items(struct mns_Items *items,unsigned int itemWidth) {
    /* this function links all items in the list */
    struct mns_Items *current,*temp;
    unsigned int count=0;
    unsigned int width=0;
    current=items++;
    temp=items;
    current->previous=0;            //no previous items
    /* Initialising linked list */
    while((count<0x10)&&(temp->name)) {
        temp->previous=current;
        current->next=temp;
        width=count_width(current->name);
        if(width>itemWidth) {
            itemWidth=width;
        }
        current=temp;
        temp=items++;
        count++;
    }
    current->next=0;
    return(count);
}

/*----------------------------------------------------------*/
/* start up menus                                           */
/*----------------------------------------------------------*/
unsigned int install_menus(struct mns_Menu *aMenu) {
    unsigned int indexx=0;
    unsigned int width=0;
    while(indexx<0x10) {
        aMenu->entries=install_items(aMenu->first,aMenu->menuWidth);
        aMenu->menuid=indexx;
        aMenu->titleWidth=count_width(aMenu->title);
        if(aMenu->next) {
            aMenu=aMenu->next;
        }
        else {
            aMenu->next=0;
            mns_baseMenu.last=aMenu;
            return(indexx);
        }
        indexx++;
    }
    aMenu->next=0;
    mns_baseMenu.last=aMenu;
    mns_baseMenu.height=indexx+1;
    return(indexx);
}


void mns_installMenuBar(unsigned int aWidth,unsigned int aHeight) {
    unsigned int x,y,xy;
    x=aWidth;
    y=aHeight+1;
    xy=x*(4*8);
    mns_baseMenu.first=0;
    mns_baseMenu.last=0;
    mns_baseMenu.width=aWidth;
    mns_baseMenu.height=aHeight;
    mns_baseMenu.memBuffer=malloc(x*y*sizeof(int));
}

void mns_uninstallMenuBar(void) {
    if(mns_baseMenu.memBuffer) {
        free(mns_baseMenu.memBuffer);
    }
}

void mns_inializeMenus(struct mns_Menu* aMenu) {
    struct mns_Menu *temp;
    unsigned int xcoord,ycoord,done;
    xcoord=2;
    ycoord=5;
    done=0;
    mns_baseMenu.first=aMenu;
    temp=aMenu;
    mns_baseMenu.entries=install_menus(aMenu);

    while(done==0) {
        temp->xcoord=xcoord;
        temp->ycoord=ycoord;
        xcoord+=(temp->titleWidth*8);

        if(temp->next) {
            temp=temp->next;
        }
        else {
            done=0xff;
        }
    }
}

void grabMenuBar(void) {
    mns_fillBuffer(mns_baseMenu.memBuffer,0,0,
                   mns_baseMenu.width,mns_baseMenu.height);
}


void replaceMenuBar(void) {
    mns_emptyBuffer(mns_baseMenu.memBuffer,0,0,
                    mns_baseMenu.width,mns_baseMenu.height);
}

void drawMenuBar(void) {
    struct mns_Menu *temp;
    unsigned int done=0xff;
    temp=mns_baseMenu.first;
    mns_setColors(cWhite,cBlack);
    mns_rectangle(0,0,320,16,1);
    mns_setColors(cBlack,cWhite);
    while(done!=0) {
        if(mns_baseMenu.select==temp->menuid) {
            mns_invertColors();
            mns_textOut(temp->xcoord,temp->ycoord,temp->title);
            mns_invertColors();
        }
        else {
            mns_textOut(temp->xcoord,temp->ycoord,temp->title);
        }
        if(temp->next) {
            temp=temp->next;
        }
        else {
            done=0;
        }
    }
    mns_rectangle(2,1,316,14,0); //draw an unfilled rectangle
}

void drawItems(struct mns_Menu* G) {
	int x=G->xcoord;
	int y=G->ycoord;
	int select=G->select;
	int entries=G->entries;
	struct mns_Items* I=G->first; //first menu item

    unsigned int indexx=0;
    y+=8+2;
    while(indexx<entries) {
        if(indexx==select) {
            mns_invertColors();
            mns_textOut(x,y,I->name);
            mns_invertColors();
        }
        else {
            mns_textOut(x,y,I->name);
        }
        I=I->next;
        indexx++;
        y+=8;
    }
}

struct mns_Menu *findMenu(unsigned int m) {
    struct mns_Menu *menu=mns_baseMenu.first;
    if(m>mns_baseMenu.entries)
        return (0);
    while(menu->menuid!=m) {
        if(menu->next) {
            menu=menu->next;
        }
        else {
            mns_baseMenu.select=m;
            return(menu);
        }
    }
    mns_baseMenu.select=m;
    return(menu);
}


unsigned int mns_handleMenu(unsigned int menu) {
    SDL_Event event;

    int done,msg;
    struct mns_Menu* G;
    menu&=0xf;
    done=0xff;
    G=findMenu(menu);
	if(G==0){
        return 0;
	}
    grabMenuBar();
    while(done) {
		//was ESC hit
        if(msg!=0xdead) {
            replaceMenuBar();
            drawMenuBar();
            mns_setColors(cBlack,cWhite);
            drawItems(G);
            mns_swapScreens();
        }

        msg=0;
        if(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                switch(event.key.keysym.sym ) {
                    case SDLK_DOWN:
                    G->select++;
                    if(G->select>=G->entries) {
                        G->select=0;
                    }
                    break;
                    case SDLK_UP:
                    if(G->select==0) {
                        G->select=G->entries-1;
                    }
                    else {
                        G->select--;
                    }
                    break;
                    case SDLK_RETURN:
                    msg=0xf00f;
                    done=0;
                    break;
                    case SDLK_F9:
                    msg=0xeeee;
                    done=0x0;
                    break;
                }
                break;
                case SDL_KEYUP:
                // If escape is pressed, return (and thus, quit)
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    msg=0xdead;
                    done=0;
                }
                break;
                case SDL_QUIT:
                msg=0xdead;
                done=0;
                break;
            }
        }

    }// end while

    replaceMenuBar();
    return(msg);
}

