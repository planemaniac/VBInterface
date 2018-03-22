#include "VBInterface.h"

#include <QSettings>
#include <QDebug>
#include <QThread>


int VBInterface::connect() {
	qInfo() << "Connecting to Voicemeeter";
	return loadDLL();
}

void VBInterface::disconnect() {
	qInfo() << "Disconnecting...";
	if ( loggedIn ) {
		logout();
	}

	lib.unload();
}

bool VBInterface::isConnected() {
	return lib.isLoaded();
}

void VBInterface::login() {
	if ( isConnected() && !loggedIn ) {
		long status = iVMR.VBVMR_Login();
		loggedIn = true;
		if ( status == 1 ) {
			qInfo() << "Voicemeeter not running... waiting...";
			//startVoiceMeeter(); // For some reason this doesn't work
			getVolume( BUS1 ); //  This will block until it can get a value
		}
	}
}

void VBInterface::logout() {
	if ( isConnected() && loggedIn ) {
		qInfo() << "Logging out of Voicemeeter";
		iVMR.VBVMR_Logout();
		loggedIn = false;
	}
}

bool VBInterface::isLoggedIn() {
	return loggedIn;
}

void VBInterface::startVoiceMeeter() {
	iVMR.VBVMR_RunVoicemeeter( 2 );
}

QString VBInterface::readString( QString req ) {
	if ( !isLoggedIn() ) {
		qWarning() << "Attempting to readString when not logged in";
		return "";
	}

	char* cReq = qStringToChar( req );
	char* response = new char[512];

	waitForClean();

	iVMR.VBVMR_GetParameterStringA( cReq, response );

	QString val = QString( response );

	delete cReq;
	delete response;

	return val;
}

float VBInterface::readFloat( QString req ) {
	if ( !isLoggedIn() ) {
		qWarning() << "Attempting to readFloat when not logged in";
		return 0;
	}

	char* cReq = qStringToChar( req );
	float response;

	waitForClean();

	iVMR.VBVMR_GetParameterFloat( cReq, &response );

	delete cReq;

	return response;
}

void VBInterface::setString( QString req, QString val ) {
	if ( !isLoggedIn() ) {
		qWarning() << "Attempting to setString when not logged in";
		return;
	}

	char* cReq = qStringToChar( req );
	char* cVal = qStringToChar( val );

	iVMR.VBVMR_SetParameterStringA( cReq, cVal );

	delete cReq;
	delete cVal;
}

void VBInterface::setFloat( QString req, float val ) {
	if ( !isLoggedIn() ) {
		qWarning() << "Attempting to setFloat when not logged in";
		return;
	}

	char* cReq = qStringToChar( req );

	iVMR.VBVMR_SetParameterFloat( cReq, val );

	delete cReq;
}

float VBInterface::getVolume( Channel channel ) {
	QString req = channelToString( channel ) + ".gain";
	return readFloat( req );
}

void VBInterface::setVolume( Channel channel, float val ) {
	QString req = channelToString( channel ) + ".gain";
	setFloat( req, val );
}

float VBInterface::setVolumeRelative( Channel channel, float amt ) {
	float vol = getVolume( channel );
	setVolume( channel, vol + amt );

	return vol + amt;
}

bool VBInterface::getMute( Channel channel ) {
	QString req = channelToString( channel ) + ".mute";
	return (bool) readFloat( req );
}

void VBInterface::setMute( Channel channel, bool mute ) {
	QString req = channelToString( channel ) + ".mute";
	setFloat( req, mute );
}

bool VBInterface::toggleMute( Channel channel ) {
	bool mute = getMute( channel );

	QString req = channelToString( channel ) + ".mute";
	setFloat( req, !mute );
	return !mute;
}

VBInterface::Channel_Level VBInterface::getChannelLevel( Channel channel ) {
	Channel_Level levels;

	if ( !loggedIn ) {
		qWarning() << "Attempting to getChannelLevel when not logged in";
		return levels;
	}

	long type = isOutputChannel( channel ) ? 3 : 0;
	pair range = channelLevelNums( channel );

	unsigned index = 0;
	for ( int i = range.first; i < range.last; i++ ) {
		waitForClean();

		float* val = indexToLevel( &levels, index );

		iVMR.VBVMR_GetLevel( type, i, val );
		*val = floor( *val * 1000 + 0.5 ) / 1000;
		index++;
	}

	return levels;
}

std::map<VBInterface::Channel, VBInterface::Channel_Level> VBInterface::getAllChannelLevels() {
	std::map<Channel, Channel_Level> levels;

	for ( int i = STRIP1; i <= BUS5; i++ ) {
		levels[(Channel) i] = getChannelLevel( (Channel) i );
	}

	return levels;
}

std::vector<VBInterface::Device> VBInterface::getOutputDevices() {
	std::vector<Device> devices;
	long num, type;
	char name[256];
	char hardwareID[256];

	num = iVMR.VBVMR_Output_GetDeviceNumber();

	for ( int i = 0; i < num; i++ ) {
		long rep = iVMR.VBVMR_Output_GetDeviceDescA( i, &type, name, hardwareID );
		if ( rep != 0 ) {
			continue;
		}

		Device newDevice;
		newDevice.name = QString( name );
		newDevice.hardwareID = QString( hardwareID );

		switch ( type ) {
			default:
			case VBVMR_DEVTYPE_WDM:
				newDevice.type = WDM;
				break;
			case VBVMR_DEVTYPE_KS:
				newDevice.type = KS;
				break;
			case VBVMR_DEVTYPE_MME:
				newDevice.type = MME;
				break;
			case VBVMR_DEVTYPE_ASIO:
				newDevice.type = ASIO;
				break;
		}

		devices.push_back( newDevice );
	}
	return devices;
}

VBInterface::Device VBInterface::getOutputDevice( Channel channel ) {
	Device device;

	if ( !isOutputChannel( channel ) ) {
		qWarning() << "Channel is not an output channel";
		return device;
	}

	QString name = readString( channelToString( channel ) + ".device.name" );
	device.name = name;
	device.type = UNKNOWN;

	return device;
}

void VBInterface::setOutputDevice( Channel channel, Device device ) {
	if ( !isOutputChannel( channel ) ) {
		qWarning() << "Channel is not an output channel";
		return;
	}

	QString req = channelToString( channel ) + ".device";
	switch ( device.type ) {
		case WDM:
			req.append( ".wdm" );
			break;
		case KS:
			req.append( ".ks" );
			break;
		case MME:
			req.append( ".mme" );
			break;
		case ASIO:
			req.append( ".asio" );
			break;
		default:
			setOutputDevice( channel, device.name );
			return;
	}

	setString( req, device.name );
}

void VBInterface::setOutputDevice( Channel channel, QString deviceName ) {
	if ( !isOutputChannel( channel ) ) {
		qWarning() << "Channel is not an output channel";
		return;
	}

	std::vector<Device> devices = getOutputDevices();
	std::vector<Device> viable;
	for ( int i = 0; i < devices.size(); i++ ) {
		if ( devices.at( i ).name == deviceName ) {
			viable.push_back( devices.at( i ) );
		}
	}

	Device chosen;
	for ( int i = 0; i < NUM_PREFERRED_TYPES; i++ ) {
		for ( int k = 0; k < viable.size(); k++ ) {
			if ( viable.at( k ).type == Preferred_Types[i] ) {
				chosen = viable.at( k );
				goto OUTPUTCHOSEN;
			}
		}
	}
OUTPUTCHOSEN:
	setOutputDevice( channel, chosen );
}

void VBInterface::setOutputDevice( Channel channel, int deviceIndex ) {
	setOutputDevice( channel, getOutputDevices().at( deviceIndex ) );
}

std::vector<VBInterface::Device> VBInterface::getInputDevices() {
	std::vector<Device> devices;
	long num, type;
	char name[256];
	char hardwareID[256];

	num = iVMR.VBVMR_Input_GetDeviceNumber();

	for ( int i = 0; i<num; i++ ) {
		long rep = iVMR.VBVMR_Input_GetDeviceDescA( i, &type, name, hardwareID );
		if ( rep != 0 ) {
			continue;
		}


		Device newDevice;
		newDevice.name = QString( name );
		newDevice.hardwareID = QString( hardwareID );

		switch ( type ) {
			default:
				newDevice.type = UNKNOWN;
				break;
			case VBVMR_DEVTYPE_WDM:
				newDevice.type = WDM;
				break;
			case VBVMR_DEVTYPE_KS:
				newDevice.type = KS;
				break;
			case VBVMR_DEVTYPE_MME:
				newDevice.type = MME;
				break;
		}

		devices.push_back( newDevice );
	}
	return devices;
}

VBInterface::Device VBInterface::getInputDevice( Channel channel ) {
	Device device;

	if ( isOutputChannel( channel ) ) {
		qWarning() << "Channel is not an input channel";
		return device;
	}
	QString name = readString( channelToString( channel ) + ".device.name" );
	device.name = name;
	device.type = UNKNOWN;

	return device;
}

void VBInterface::setInputDevice( Channel channel, Device device ) {
	if ( isOutputChannel( channel ) ) {
		qWarning() << "Channel is not an input channel";
		return;
	}

	QString req = channelToString( channel ) + ".device";
	switch ( device.type ) {
		case WDM:
			req.append( ".wdm" );
			break;
		case KS:
			req.append( ".ks" );
			break;
		case MME:
			req.append( ".mme" );
			break;
		default:
			setOutputDevice( channel, device.name );
			return;
	}

	setString( req, device.name );
}

void VBInterface::setInputDevice( Channel channel, QString deviceName ) {
	if ( isOutputChannel( channel ) ) {
		qWarning() << "Channel is not an input channel";
		return;
	}

	std::vector<Device> devices = getInputDevices();
	std::vector<Device> viable;
	for ( int i = 0; i < devices.size(); i++ ) {
		if ( devices.at( i ).name == deviceName ) {
			viable.push_back( devices.at( i ) );
		}
	}

	Device chosen;
	for ( int i = 0; i < NUM_PREFERRED_TYPES; i++ ) {
		for ( int k = 0; k < viable.size(); k++ ) {
			if ( viable.at( k ).type == Preferred_Types[i] ) {
				chosen = viable.at( k );
				goto INPUTCHOSEN;
			}
		}
	}
INPUTCHOSEN:
	setInputDevice( channel, chosen );
}

void VBInterface::setInputDevice( Channel channel, int deviceIndex ) {
	setInputDevice( channel, getInputDevices().at( deviceIndex ) );
}

bool VBInterface::isDirty() {
	long dirty = iVMR.VBVMR_IsParametersDirty();

	return dirty != 0;
}

void VBInterface::waitForClean() {
	while ( isDirty() ) {
		// Blocking loop...
		QThread::usleep( 10 );
	}
}

int VBInterface::loadDLL() {
	QString VB_ID = "VB:Voicemeeter {17359A74-1236-5467}";

	QString path32 = "HKEY_LOCAL_MACHINE\\Software\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + VB_ID;
	QString path64 = "HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + VB_ID;

	// Get Voicemeeter DLL location
	QString path = QSettings( path64, QSettings::NativeFormat ).value( "UninstallString" ).toString();
	
	if ( path.isEmpty() ) {
		path = QSettings( path32, QSettings::NativeFormat ).value( "UninstallString" ).toString();
	}

	if ( path.isEmpty() ) {
		qCritical() << "Can't find installed Voicemeeter";
		return -100; // Can't find installed VoiceMeeter
	}

	QStringList parts = path.split( "\\" );
	parts.removeLast();
	path = parts.join( "/" );
	path.append( "/VoicemeeterRemote64.dll" );

	lib.setFileName( path );
	lib.load();

	if ( !lib.isLoaded() ) {
		qCritical() << lib.errorString();
		return -1;
	}

	iVMR.VBVMR_Login = (T_VBVMR_Login) lib.resolve( "VBVMR_Login" );
	iVMR.VBVMR_Logout = (T_VBVMR_Logout) lib.resolve( "VBVMR_Logout" );
	iVMR.VBVMR_RunVoicemeeter = (T_VBVMR_RunVoicemeeter) lib.resolve( "VBVMR_RunVoicemeeter" );
	iVMR.VBVMR_GetVoicemeeterType = (T_VBVMR_GetVoicemeeterType) lib.resolve( "VBVMR_GetVoicemeeterType" );
	iVMR.VBVMR_GetVoicemeeterVersion = (T_VBVMR_GetVoicemeeterVersion) lib.resolve( "VBVMR_GetVoicemeeterVersion" );

	iVMR.VBVMR_IsParametersDirty = (T_VBVMR_IsParametersDirty) lib.resolve( "VBVMR_IsParametersDirty" );
	iVMR.VBVMR_GetParameterFloat = (T_VBVMR_GetParameterFloat) lib.resolve( "VBVMR_GetParameterFloat" );
	iVMR.VBVMR_GetParameterStringA = (T_VBVMR_GetParameterStringA) lib.resolve( "VBVMR_GetParameterStringA" );
	iVMR.VBVMR_GetParameterStringW = (T_VBVMR_GetParameterStringW) lib.resolve( "VBVMR_GetParameterStringW" );
	iVMR.VBVMR_GetLevel = (T_VBVMR_GetLevel) lib.resolve( "VBVMR_GetLevel" );
	iVMR.VBVMR_GetMidiMessage = (T_VBVMR_GetMidiMessage) lib.resolve( "VBVMR_GetMidiMessage" );

	iVMR.VBVMR_SetParameterFloat = (T_VBVMR_SetParameterFloat) lib.resolve( "VBVMR_SetParameterFloat" );
	iVMR.VBVMR_SetParameters = (T_VBVMR_SetParameters) lib.resolve( "VBVMR_SetParameters" );
	iVMR.VBVMR_SetParametersW = (T_VBVMR_SetParametersW) lib.resolve( "VBVMR_SetParametersW" );
	iVMR.VBVMR_SetParameterStringA = (T_VBVMR_SetParameterStringA) lib.resolve( "VBVMR_SetParameterStringA" );
	iVMR.VBVMR_SetParameterStringW = (T_VBVMR_SetParameterStringW) lib.resolve( "VBVMR_SetParameterStringW" );

	iVMR.VBVMR_Output_GetDeviceNumber = (T_VBVMR_Output_GetDeviceNumber) lib.resolve( "VBVMR_Output_GetDeviceNumber" );
	iVMR.VBVMR_Output_GetDeviceDescA = (T_VBVMR_Output_GetDeviceDescA) lib.resolve( "VBVMR_Output_GetDeviceDescA" );
	iVMR.VBVMR_Output_GetDeviceDescW = (T_VBVMR_Output_GetDeviceDescW) lib.resolve( "VBVMR_Output_GetDeviceDescW" );
	iVMR.VBVMR_Input_GetDeviceNumber = (T_VBVMR_Input_GetDeviceNumber) lib.resolve( "VBVMR_Input_GetDeviceNumber" );
	iVMR.VBVMR_Input_GetDeviceDescA = (T_VBVMR_Input_GetDeviceDescA) lib.resolve( "VBVMR_Input_GetDeviceDescA" );
	iVMR.VBVMR_Input_GetDeviceDescW = (T_VBVMR_Input_GetDeviceDescW) lib.resolve( "VBVMR_Input_GetDeviceDescW" );

	// check pointers are valid
	if ( iVMR.VBVMR_Login == NULL ) return -1;
	if ( iVMR.VBVMR_Logout == NULL ) return -2;
	if ( iVMR.VBVMR_RunVoicemeeter == NULL ) return -2;
	if ( iVMR.VBVMR_GetVoicemeeterType == NULL ) return -3;
	if ( iVMR.VBVMR_GetVoicemeeterVersion == NULL ) return -4;
	if ( iVMR.VBVMR_IsParametersDirty == NULL ) return -5;
	if ( iVMR.VBVMR_GetParameterFloat == NULL ) return -6;
	if ( iVMR.VBVMR_GetParameterStringA == NULL ) return -7;
	if ( iVMR.VBVMR_GetParameterStringW == NULL ) return -8;
	if ( iVMR.VBVMR_GetLevel == NULL ) return -9;
	if ( iVMR.VBVMR_SetParameterFloat == NULL ) return -10;
	if ( iVMR.VBVMR_SetParameters == NULL ) return -11;
	if ( iVMR.VBVMR_SetParametersW == NULL ) return -12;
	if ( iVMR.VBVMR_SetParameterStringA == NULL ) return -13;
	if ( iVMR.VBVMR_SetParameterStringW == NULL ) return -14;
	if ( iVMR.VBVMR_GetMidiMessage == NULL ) return -15;

	if ( iVMR.VBVMR_Output_GetDeviceNumber == NULL ) return -30;
	if ( iVMR.VBVMR_Output_GetDeviceDescA == NULL ) return -31;
	if ( iVMR.VBVMR_Output_GetDeviceDescW == NULL ) return -32;
	if ( iVMR.VBVMR_Input_GetDeviceNumber == NULL ) return -33;
	if ( iVMR.VBVMR_Input_GetDeviceDescA == NULL ) return -34;
	if ( iVMR.VBVMR_Input_GetDeviceDescW == NULL ) return -35;

	return 0;
}

char* VBInterface::qStringToChar( QString input ) {
	std::string str = input.toStdString();
	char* cReq = new char[str.length() + 1];

	strcpy( cReq, str.c_str() );

	return cReq;
}

QString VBInterface::channelToString( Channel channel ) {
	switch ( channel ) {
		case STRIP1:
			return "Strip[0]";
		case STRIP2:
			return "Strip[1]";
		case STRIP3:
			return "Strip[2]";
		case VIRT1:
			return "Strip[3]";
		case VIRT2:
			return "Strip[4]";
		default:
		case BUS1:
			return "Bus[0]";
		case BUS2:
			return "Bus[1]";
		case BUS3:
			return "Bus[2]";
		case BUS4:
			return "Bus[3]";
		case BUS5:
			return "Bus[4]";
	}
}

bool VBInterface::channelSize( Channel channel ) {
	switch ( channel ) {
		case VIRT1:
		case VIRT2:
		case BUS1:
		case BUS2:
		case BUS3:
		case BUS4:
		case BUS5:
			return true;
		default:
			return false;
	}
}

bool VBInterface::isOutputChannel( Channel channel ) {
	switch ( channel ) {
		case BUS1:
		case BUS2:
		case BUS3:
		case BUS4:
		case BUS5:
			return true;
		default:
			return false;
	}
}

VBInterface::pair VBInterface::channelLevelNums( Channel channel ) {
	pair firstLast;
	if ( isOutputChannel( channel ) ) {
		firstLast.first = ( channel - BUS1 ) * 8;
		firstLast.last = firstLast.first + 8;
	} else {
		if ( channel < VIRT1 ) {
			firstLast.first = channel * 2;
			firstLast.last = firstLast.first + 2;
		} else {
			firstLast.first = ( channel - VIRT1 ) * 8 + 6;
			firstLast.last = firstLast.first + 8;
		}
	}
	return firstLast;
}

float* VBInterface::indexToLevel( Channel_Level *levels, unsigned index ) {
	switch ( index ) {
		default:
		case 0:
			return &( levels->left );
		case 1:
			return &( levels->right );
		case 2:
			return &( levels->CH3 );
		case 3:
			return &( levels->CH4 );
		case 4:
			return &( levels->CH5 );
		case 5:
			return &( levels->CH6 );
		case 6:
			return &( levels->CH7 );
		case 7:
			return &( levels->CH8 );
	}
}
