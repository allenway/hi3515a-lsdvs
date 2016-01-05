#ifndef __FIT_H__
#define __FIT_H__

#include "ttypes.h"

int FitMympiForceIframe( int channel );
void FitEncAddTimer();
int FitFiOsdInitOsdLib();
int FitFiOsdDeinitOsdLib();
int FitMpiServiceStart();
int FitMpiServiceStop();
int FitSnapMpiGetJpgAndToProcon( int snapCh, uint snapType );


int FitMympiSetLevel( int channel, int val );
int FitMympiSetBitrateType( int channel, int val );
int FitMympiSetBitrate( int channel, int val );
int FitMympiSetFramerate( int channel, int val );
int FitMympiSetIframeInterval( int channel, int val );
int FitMympiSetResolution();
int FitMympiSetVideoStandard();           
int FitFiOsdSetLogoOsdConfig( int channel, ConfigOsdLogo *setVal );
int FitFiOsdSetTimeOsdConfig( int channel, ConfigOsdTime *setVal );
int FitMympiSetAvencAccompanyingAudio( int channel, int openFlag );
int FitMympiGetShelterDetect( int channel );
int FitMympiSetAudioEncType();

#endif 

