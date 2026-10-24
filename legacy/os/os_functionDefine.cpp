#include "os_typedefine.h"
SEGMENT make_segment(bool state,unsigned pageSize,PAGE *pageAddress)
{
    SEGMENT s;
    s.state=state;
    s.pageSize=pageSize;
    s.pageAddress=pageAddress;
    return s;
}
PAGE make_page(bool state,unsigned blockID)
{
    PAGE p;
    p.blockID=blockID;
    p.state=state;
    return p;
}
BLOCK make_block(unsigned blockID,int blockMark)
{
    BLOCK b;
    b.blockMark=blockMark;
    return b;
}
