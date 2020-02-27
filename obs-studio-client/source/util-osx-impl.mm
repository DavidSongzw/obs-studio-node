#include "util-osx-impl.h"
#include <iostream>

@implementation UtilImplObj

UtilObjCInt::UtilObjCInt(void)
    : self(NULL)
{   }

UtilObjCInt::~UtilObjCInt(void)
{
    [(id)self dealloc];
}

void UtilObjCInt::init(void)
{
    self = [[UtilImplObj alloc] init];

	m_webcam_perm = false;
	m_mic_perm    = false;
}

void UtilObjCInt::getPermissionsStatus(bool &webcam, bool &mic)
{
	AVAuthorizationStatus camStatus = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
	webcam = camStatus == AVAuthorizationStatusAuthorized;

	AVAuthorizationStatus micStatus = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio];
	mic = micStatus == AVAuthorizationStatusAuthorized;

	m_webcam_perm = webcam;
	m_mic_perm    = mic;
}

void UtilObjCInt::requestPermissions(void *async_cb, perms_cb cb)
{
	m_async_cb = async_cb;
	[AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL granted)
	{
		m_webcam_perm = granted;
		cb(m_async_cb, m_webcam_perm, m_mic_perm);
	}];

	[AVCaptureDevice requestAccessForMediaType:AVMediaTypeAudio completionHandler:^(BOOL granted)
	{
		m_mic_perm = granted;
		cb(m_async_cb, m_webcam_perm, m_mic_perm);
	}];
}

@end