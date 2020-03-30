void hslMask3x3(HSUINT8* in, HSUINT8* out, 
                HSUINT16 wid, HSUINT16 hgt, 
                HSUINT16 offsetX, HSUINT16 offsetY,
                HSUINT16 validWid, HSUINT16 validHgt,
                HSINT16* mtrx);
void hslMask3x1(HSUINT8* in, HSUINT8* out, 
                HSUINT16 wid, HSUINT16 hgt, 
                HSUINT16 offsetX, HSUINT16 offsetY,
                HSUINT16 validWid, HSUINT16 validHgt,
                HSINT16* mtrx);
void hslMask9x1(HSUINT8* in, HSUINT8* out, 
                HSUINT16 wid, HSUINT16 hgt, 
                HSUINT16 offsetX, HSUINT16 offsetY,
                HSUINT16 validWid, HSUINT16 validHgt,
                HSINT16* mtrx);
void hslMaskSobel(HSUINT8* in, HSUINT8* out, 
                HSUINT16 wid, HSUINT16 hgt, 
                HSUINT16 offsetX, HSUINT16 offsetY,
                HSUINT16 validWid, HSUINT16 validHgt);

