#ifndef __MYMPI_H__
#define __MYMPI_H__

int MympiForceIframe( int channel );
int MympiSetLevel( int channel, int val );
int MympiSetBitrateType( int channel, int val );
int MympiSetBitrate( int channel, int val );
int MympiSetFramerate( int channel, int val );
int MympiSetIframeInterval( int channel, int val );
int MympiSetResolution();
int MympiSetVideoStandard();
int MympiSetAudioEncType();

int MympiSetAvencAccompanyingAudio( int channel, int openFlag );

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

void MympiStopAudioEnc();    //  mpiAudio.cpp
int MympiAudioStartEnc();


#ifdef __cplusplus
#if __cplusplus
}
#endif 
#endif /* End of #ifdef __cplusplus */

#endif // __MYMPI_H__

