#ifndef DEBUG_H
#define DEBUG_H

#include <QDebug>

//#define C_DEBUG_CONSTDEST
#ifdef C_DEBUG_CONSTDEST
#define D_CONSTRUCT(x) qDebug() << objectName() << x << "constructed";
#define D_DESTRUCT(x) qDebug() << objectName() << x << "destructed";
#else
#define D_CONSTRUCT(x) ;
#define D_DESTRUCT(x) ;
#endif

//#define C_DEBUG_SLOTCALL
#ifdef C_DEBUG_SLOTCALL
#define D_SLOTCALL(x) qDebug() << "SLOT" << x << "CALLED";
#else
#define D_SLOTCALL(x);
#endif

#endif // DEBUG_H
