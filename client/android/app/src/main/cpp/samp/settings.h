#pragma once

#define MAX_SETTINGS_STRING	0x7F

// Server the client connects to when settings.ini does not say otherwise.
// Point these at your own server — upstream shipped the original author's
// address here, which meant every build connected to their server by default.
#define SAMP_DEFAULT_HOST	"127.0.0.1"
#define SAMP_DEFAULT_PORT	7777

// The settings file lives under Platform::ConfigPath(); the name is all that
// is fixed, because the directory differs per platform.
#define SAMP_SETTINGS_FILE	"settings.ini"

struct stSettings
{
	// client
	char szNickName[24+1];
	char szHost[MAX_SETTINGS_STRING+1];
	int iPort;
	char szPassword[MAX_SETTINGS_STRING+1];
    char szVersion[MAX_SETTINGS_STRING+1];

	// debug
	bool bDebug;
	bool bOnline;
	bool bAutoAim;

	// gui
	char szFont[40];
	float fFontSize;
	int iFontOutline;
	float fChatPosX;
	float fChatPosY;
	float fChatSizeX;
	float fChatSizeY;
	int iChatMaxMessages;
	float fSpawnScreenPosX;
	float fSpawnScreenPosY;
	float fSpawnScreenSizeX;
	float fSpawnScreenSizeY;
	float fHealthBarWidth;
	float fHealthBarHeight;
	float fScoreBoardSizeX;
	float fScoreBoardSizeY;
	bool bPassengerUseTexture;
	float fPassengerTextureSize;
	float fPassengerTextureX;
	float fPassengerTextureY;
	bool bVoiceChatEnable;
	int iVoiceChatKey;
	float fVoiceChatSize;
	float fVoiceChatPosX;
	float fVoiceChatPosY;

	bool iAndroidKeyboard;
	bool iCutout;
	bool iFPSCounter;
	bool iHPArmourText;
	bool iOutfitGuns;
	bool iPCMoney;
	bool iRadarRect;
	bool iSkyBox;
	bool iSnow;
	bool iFirstPerson;

	int iFPSCount;

	bool iDialog;
};

class CSettings
{
public:
	CSettings();
	~CSettings();

	stSettings& Get() { return m_Settings; }

    const stSettings& GetReadOnly();
    stSettings& GetWrite();
	
private:
	struct stSettings m_Settings;
};
