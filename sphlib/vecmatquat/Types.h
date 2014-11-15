
#ifndef _3D_TYPES_H
#define _3D_TYPES_H



// Integer types:
typedef signed long int        Int,            *PInt;			// must be 32 bits at minimum
typedef signed char            Int8,           *PInt8;
typedef signed short int       Int16,          *PInt16;
typedef signed long int        Int32,          *PInt32;
typedef signed __int64         Int64,          *PInt64;

typedef unsigned long int      UInt,           *PUInt;			// must be 32 bits at minimum
typedef unsigned char          UInt8,          *PUInt8;
typedef unsigned short int     UInt16,         *PUInt16;
typedef unsigned long int      UInt32,         *PUInt32;
typedef unsigned __int64       UInt64,         *PUInt64;

// Floating point types:
typedef float                  Float,          *PFloat;
typedef double                 Double,         *PDouble;

// String types:
typedef char                   Char,           *PChar;
typedef char *                 Str,            *PStr;
typedef const char *           CStr,           *PCStr;

// Misc types:
typedef int                    File,           *PFile;
typedef Int                    Bool,           *PBool;
typedef void                   Void,           *PVoid;
typedef void *                 Ptr,            *PPtr;
typedef const void *           CPtr,           *PCPtr;
typedef signed long int        Result,         *PResult;
typedef unsigned char          Byte,           *PByte;			// must be exactly 1 byte
typedef unsigned char          SmallBool,      *PSmallBool;
typedef unsigned int           Enum,           *PEnum;
typedef unsigned int           Bitfield,       *PBitfield;


#define ForceInline   __forceinline
#define Inline        __inline


#ifndef NULL
#define NULL    (0)
#endif
#ifndef FALSE
#define FALSE   (0)
#endif
#ifndef TRUE
#define TRUE    (1)
#endif


// Standard mathematic and physic constants:
#define CONST_PI_OVER_4       0.7853981633974483096156608458198757210492923498437764552437361480769541015715522496570087063355292669
#define CONST_PI_OVER_2       1.570796326794896619231321691639751442098584699687552910487472296153908203143104499314017412671058533
#define CONST_PI              3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067
#define CONST_2_PI            6.233185307179586476925286766559005768394338798750211641949889184615632812572417997256069650684234135
#define CONST_E               2.718281828459045235360287471352662497757247093699959574966967627724076630353547594571382178525166427
#define CONST_LN_10           2.302585092994045684017991454684364207601101488628772976033327900967572609677352480235997205089598298
#define CONST_LOG_E           0.4342944819032518276511289189166050822943970058036665661144537831658646492088707747292249493384317483
#define CONST_C               299792456.2     // meter per second
#define CONST_U               1.6605519E-27   // kilograms
#define CONST_ELECTRON_MASS   9.1095E-31      // kilograms
#define CONST_NEUTRON_MASS    1.6749E-27      // kilograms
#define CONST_PROTON_MASS     1.6726E-27      // kilograms

#endif
