

// BAM structure
struct tBAM {
    char Track;
    char Sector;
    char Format;
    char pad0;
    char FAT[35][4]; // Sector Allocation
    char Name[18];
    short ID;//array[1..2] of char; }
    char pad1;
    short Chars; //array[1..2] of char;
    char pad2[4];
    char pad3[85];
}

// Directory entry structure
struct TDirEntry {
Kind   :
    byte;
Track  :
    byte;
Sector :
    byte;
Name[1..16] of char;
sTrack :
    byte;
sSector:
    byte;
Len    :
    byte;
pad0   :
    array[1..4] of byte;
oTrack :
    byte;
oSector:
    byte;
Blocks :
    word;
pad1   :
    array[1..2] of byte;
}

{
    Directory block structure
}
TDirectory=record {
               padding:
               array[1..2] of byte;
           }
           NextTrack :
           byte;
NextSector:
byte;
Entry     :
array[0..7] of TDirEntry;
end;

char TBuffer[8192];

//function LoadD64(var FileName:string{; Var BAM:tBAM}):word;
//function Load(Handle:word; Channel:byte; FileName:string):byte;
//function Read(Handle:word; Channel:byte; Var Value:byte):byte;

const int SectorCount[40]=(
                              21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,
                              19,19,19,19,19,19,19,
                              18,18,18,18,18,18,
                              17,17,17,17,17,
                              17,17,17,17,17 // Tracks 36..40
                          );
                          
int SectorOfs[40]={
                      0,21,42,63,84,105,126,147,168,189,210,231,252,273,294,315,336,
                      357,376,395,414,433,452,471,
                      490,508,526,544,562,580,
                      598,615,632,649,666,
                      683,700,717,734,751 // Tracks 36..40
                  };
                  
char KindName[]={
            'DEL','SEQ','PRG','USR','REL','EL?','EQ?','RG?','SR?','EL?','L??',
            'Q??','G??','R??','L??','???'
        };
        
struct TChannelMode
{cmFree,
cmDir};

short HeaderSize;
struct TChannelMode ChannelMode[11];

BAM:
tBAM;

Buffer:
TBuffer;
BufferSize:
word;
Cursor:
word;

long SectorOffset(short Track, short Sector) {
    if ((Track<1)||(Track>40)||
            (Sector<0)||(Sector>=SectorCount[Track])) {
        SectorOffset=-1;
    }
    else {
        SectorOffset=(SectorOfs[Track]+Sector) shl 8+HeaderSize;
    };
    
    
    {
        Read sector (256 bytes)
    }
function ReadSector(Handle,Track,Sector:byte; Var Data):boolean;
    var
offset:
    longint;
    begin
offset:
    =SectorOffset(Track,Sector);
ReadSector:
    =(Offset>=0)
     and(SeekFile(Handle,Offset))
     and(ReadFile(Handle,Data,256)=256);
    end;
    
function LoadD64(var FileName:string {
                         ;
                     Var BAM:
                         tBAM
                     }
                ):word;
    var
h:
    integer;
magic:
    longint;
    
function ReadBAM:
    boolean;
    var
i:
    byte;
    begin
    if (ReadFile(h,Magic,4)=4)
        and (Magic=$64411543) then
HeaderSize:
        =64
         else
 HeaderSize:
             =0;
             
    if ReadSector(h,18,0,BAM)
        then begin
ReadBAM:
        =True;
for i:
    =1 to 18 do
         if BAM.Name[i]
 =#$A0 then BAM.Name[i]:=' ';
            end else
ReadBAM:
                =False;
    end;
    
    begin
    if FileName='' then SelectFile(FileName)
                    ;
if FileName='' then h:
                =0 else
             h:
                     =OpenFile(FileName);
    if (h=0)
        or(FileSize(h)<683*256)or(not ReadBAM) then begin
        CloseFile(h);
h:
    =0;
FileName:
    ='';
    end;
LoadD64:
    =h;
    end;
    
function LoadDirectory(Handle:word):byte;
    var
Dir:
    TDirectory;
e:
    word;
i:
    byte;
q:
    char;
    
FirstLine:
    record
ADDR :
    word;
Link :
    word;
Drive:
    word;
RVSOn:
    byte;
Quot1:
    char;
Name :
    array[1..18] of char;
Quot2:
    char;
ID   :
    word;
Spc1 :
    char;
CHARS:
    word;
ZERO :
    byte;
    end;
    
NewLine:
    record
Link  :
    word;
Blocks:
    word;
Space1:
    char;
Space2:
    char;
Space3:
    char;
    end;
    
EntryLine:
    record
Quote1:
    char;
Name  :
    array[1..16] of char;
Quote2:
    char;
Flag1 :
    char;
Kind  :
    array[1..3] of char;
Flag2 :
    char;
Space2:
    char;
ZERO  :
    byte;
    end;
    
LastLine:
    record
Link:
    word;
Free:
    word;
Text:
    array[1..12] of char;
ZERO1:
    word;
ZERO2:
    word;
    end;
    
    begin {
        Directory title
    }
Cursor:
    =0;
    with FirstLine do
        begin
ADDR :
        =$0401;
Link :
    =$0101;
Drive:
    =0;
RVSOn:
    =$12;
Quot1:
    ='"';
    Move(BAM.Name,Name,SizeOf(Name));
Quot2:
    ='"';
ID   :
    =BAM.Id;
Spc1 :
    =' ';
CHARS:
    =BAM.Chars;
ZERO :
    =0;
    end;
    Move(FirstLine,Buffer[Cursor],SizeOf(FirstLine));
    Inc(Cursor,SizeOf(FirstLine));
    
Dir.NextTrack:
    =BAM.Track;
Dir.NextSector:
    =BAM.Sector;
    
    while(Dir.NextTrack<>0)
        and (ReadSector(Handle,Dir.NextTrack,Dir.NextSector,Dir)) do
            begin {
                Scan all 8 entries of a block
            }
for e:
            =0 to 7 do
                 with Dir.Entry[e] do
                         if Kind<>
                         0 then begin
                         
 NewLine.Link  :
                         =$0101;
NewLine.Blocks:
=Blocks;
NewLine.Space1:
=' ';
NewLine.Space2:
=' ';
NewLine.Space3:
=' ';
i:
=SizeOf(NewLine);
    if Blocks>10  then dec(i)
        ;
    if Blocks>100 then dec(i)
        ;
    Move(NewLine,Buffer[Cursor],i);
    inc(Cursor,i);
    
EntryLine.Quote1:
    ='"';
q:
    ='"';
for i:
    =1 to SizeOf(Name)
         do
             if Name[i]
 =' ' then begin Name[i]:
                  =q;
q:
    =' ';
    end;
    Move(Name,EntryLine.Name,SizeOf(Name));
EntryLine.Quote2:
    =q;
if Kind and $80=0 then EntryLine.Flag1:
                    ='*' else
                 EntryLine.Flag1:
                         =' ';
Move(KindName[Kind and $F],EntryLine.Kind,3);
if Kind and $40=0 then EntryLine.Flag2:
                    =' ' else
                 EntryLine.Flag2:
                         ='<';
EntryLine.Space2:
=' ';
EntryLine.Zero  :
=0;
Move(EntryLine,Buffer[Cursor],SizeOf(EntryLine));
    Inc(Cursor,SizeOf(EntryLine));
    end;
    {
        for entry
    }
end;
{
    while
    }
    
e:
=0;
for i:
    =1 to 34 do
 e:
         =e+BAM.FAT[i,1];
         
    with LastLine do
        begin
Link:
        =$0101;
Free:
    =e;
Text:
    ='BLOCKS FREE.';
ZERO1:
    =0;
ZERO2:
    =0;
    end;
    
    Move(LastLine,Buffer[Cursor],SizeOf(LastLine));
    
BufferSize:
    =Cursor+SizeOf(LastLine);
Cursor:
    =0;
    
LoadDirectory:
    =$00;
    end;
    
Function FindFile(Handle:word; FileName:string; Var ATrack,ASector:byte):boolean;
    var
Dir:
    TDirectory;
e,n:
    byte;
found:
    boolean;
    begin
Dir.NextTrack:
    =BAM.Track;
Dir.NextSector:
    =BAM.Sector;
    
    while(Dir.NextTrack<>0)
        and (ReadSector(Handle,Dir.NextTrack,Dir.NextSector,Dir)) do
            begin {
                Scan all 8 entries of a block
            }
for e:
            =0 to 7 do
                 with Dir.Entry[e] do
                         begin
 found:
                         =true;
for n:
    =1 to Length(FileName)
         do
             if FileName[n]
 <>Name[n] then found:
                 =false;
    if found then begin
ATrack:
    =Track;
ASector:
=Sector;
FindFile:
=True;
exit;
end;
end;
end;
Findfile:
=False;
end;

function Loadfile(Handle:word;FileName:string)
:byte;
    Var
ASector:
    record
Track,Sector:
    byte;
Data:
    array[1..253] of byte;
    end;
    begin
    if not FindFile(Handle,FileName,ASector.Track,ASector.Sector)
        then begin
LoadFile:
        =$00;
    exit;
    end;
Cursor:
    =0;
    while(ASector.Track<>0)
        and(ReadSector(Handle,ASector.Track,ASector.Sector,ASector)) do
            begin
            Move(ASector.Data,Buffer[Cursor],253);
    inc(Cursor,253);
    end;
BufferSize:
    =Cursor;
Cursor:
    =0;
    end;
    
function Load(Handle:word; Channel:byte; FileName:string):byte;
    begin
    if (Handle=0)
        or(Channel>11) then
Load:
        =$80 {
             not present
         }
         else
             begin
 Load:
             =$00;
    {
        OK
    }
    {
        if ChannelMode[Channel]
            <>cmFREE then exit;
    }
    if FileName='$' then
            Load:
                =LoadDirectory(Handle)
                     else
                         begin
             Load:
                         =LoadFile(Handle,FileName);
    end;
    end;
    end;
    
    char Read(short Handle,char Channel) {
        char Value;
        if {Cursor<BufferSize ) {
            Value=Buffer[Cursor]
                  ;
            Cursor++;
            if Cursor<bufferSize ) {
            Read=$00
             }
             else {
                 Read=$40;
             }
             else {
                 Read=$02;
             }
         }
         }
}
}
}