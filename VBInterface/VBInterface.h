#pragma once

#include "vbinterface_global.h"

#include <QLibrary>
#include <QString>
#include <vector>

#include "VoicemeeterRemote.h"

#define NUM_PREFERRED_TYPES 4

class VBINTERFACE_EXPORT VBInterface : public QObject {
	Q_OBJECT

public:
	enum Channel {
		STRIP1,
		STRIP2,
		STRIP3,
		VIRT1,
		VIRT2,
		BUS1,
		BUS2,
		BUS3,
		BUS4,
		BUS5
	};

	struct Channel_Level {
		float left = 0.f;
		float right = 0.f;
		float CH3 = 0.f;
		float CH4 = 0.f;
		float CH5 = 0.f;
		float CH6 = 0.f;
		float CH7 = 0.f;
		float CH8 = 0.f;
	};

	struct pair {
		int first;
		int last;
	};

	enum Device_Type {
		WDM,
		MME,
		KS,
		ASIO,
		UNKNOWN
	};

	struct Device {
		Device_Type type;
		QString name, hardwareID;

		QString toString() {
			QString device;
			switch ( type ) {
				default:
				case WDM:
					device = "WDM: ";
					break;
				case MME:
					device = "MME: ";
					break;
				case KS:
					device = "KS: ";
					break;
				case ASIO:
					device = "ASIO: ";
					break;
				case UNKNOWN:
					device = "";
					break;
			}

			return device + name;
		}
	};

	friend bool operator==( const Device& lhs, const Device& rhs ) {
		return lhs.name == rhs.name && lhs.hardwareID == rhs.hardwareID;
	}

	friend bool operator!=( const Device& lhs, const Device& rhs ) {
		return !( lhs == rhs );
	}

	Device_Type Preferred_Types[NUM_PREFERRED_TYPES] = { WDM, ASIO, KS, MME };

private:
	T_VBVMR_INTERFACE iVMR;
	QLibrary lib;

public:
	VBInterface();

public slots:
	/** Find, Connect and Load VB's remote dll */
	int connect();
	/** Unload VB's DLL */
	void disconnect();
	/** Check is we're connected */
	bool isConnected();
	/** Login to the remote server */
	void login();
	/** Logout from remote server */
	void logout();
	/** Start Voicemeeter Banana */
	void startVoiceMeeter();
	/** Check if we're logged in */
	bool isLoggedIn();
	/** Check if remote parameters are dirty */
	bool isDirty();

	/////////////////// Raw Access Functions ///////////////////////

	/** Read raw parameter string */
	QString readString( QString req );
	/** Read raw parameter float */
	float readFloat( QString req );
	/** Set raw parameter string */
	void setString( QString req, QString val );
	/** Set raw parameter string */
	void setFloat( QString req, float val );

	////////////////////// Helper Functions ///////////////////////

	/** Get channel's volume */
	float getVolume( Channel channel );
	/** Set channel's volume */
	void setVolume( Channel channel, float val );
	/** Change a channel's volume by a certain amount */
	float setVolumeRelative( Channel channel, float amt );
	/** Get mute status of channel */
	bool getMute( Channel channel );
	/** Mute/Unmute a channel */
	void setMute( Channel channel, bool mute );
	/** Toggle mute a channel */
	bool toggleMute( Channel channel );
	/** Get current levels of a channel */
	Channel_Level getChannelLevel( Channel channel );
	/** Get all current levels */
	std::map<Channel, Channel_Level> getAllChannelLevels();

	std::vector<Device> getOutputDevices();

	Device getOutputDevice( Channel channel );

	void setOutputDevice( Channel channel, int deviceIndex );
	void setOutputDevice( Channel channel, QString deviceName );
	void setOutputDevice( Channel channel, Device device );

	std::vector<Device> getInputDevices();

	Device getInputDevice( Channel channel );

	void setInputDevice( Channel channel, int deviceIndex );
	void setInputDevice( Channel channel, QString deviceName );
	void setInputDevice( Channel channel, Device device );

private:
	bool loggedIn = false;

	int loadDLL();
	void waitForClean();
	char* qStringToChar( QString input );
	QString channelToString( Channel channel );

	/** Returns whether a channel has 2 or 7 levels
	*
	* true = 7
	* false = 2;
	**/
	bool channelSize( Channel channel );
	bool isOutputChannel( Channel channel );
	pair channelLevelNums( Channel channel );
	float* indexToLevel( Channel_Level* levels, unsigned index );
};


Q_DECLARE_METATYPE( VBInterface::Channel )
Q_DECLARE_METATYPE( VBInterface::Channel_Level )
Q_DECLARE_METATYPE( VBInterface::Device_Type )
Q_DECLARE_METATYPE( VBInterface::Device )

