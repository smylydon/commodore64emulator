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
    unsigned int xcoord,ycoord;
    unsigned int menuWidth,height;
    unsigned int titleWidth;
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

extern void mns_fileselecter(struct fileselector *F);
extern void mns_inializeMenus(struct mns_Menu* aMenu);
extern void mns_installMenuBar(unsigned int aWidth,unsigned int aHeight);
extern void mns_uninstallMenuBar(void);
extern unsigned int mns_handleMenu(unsigned int m) ;
