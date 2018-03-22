/******************************************************************************/
/* Voicemeeter Remote API.                                       V.Burel©2015 */
/******************************************************************************/
/* This Library allows communication with Voicemeeter Applications            */
/* 4 Client Applications can be connected to remote Voicemeeter.              */
/******************************************************************************/
/*                                                                            */
/*                  OFFICIAL LINK : WWW.VOICEMEETER.COM                       */
/*                                                                            */
/******************************************************************************/
/* COPYRIGHT: Vincent Burel(c)2015 All Rights Reserved                        */
/******************************************************************************/
/* LICENSING: This header file and VoicemeeterRemote.dll are made available   */
/*            for evaluation and testing purpose only (to make demo and proof */
/*            of concept).                                                    */
/*                                                                            */
/*            for any commercial exploitation or to get specific support,     */
/*            a commercial license needs to be purchased. thanks to contact   */
/*            us by e-mail (get our contact on www.vb-audio.com).             */
/******************************************************************************/


#ifndef __VOICEMEETER_REMOTE_H__
#define __VOICEMEETER_REMOTE_H__

#ifdef __cplusplus
extern "C" {
#endif


#define VBVMR_RESULT_OK			0

/******************************************************************************/
/*                                                                            */
/*                                    Login                                   */
/*                                                                            */
/******************************************************************************/

/** @name Communication Login / logout
* @{ */

	/** 
	@brief Open Communication Pipe With Voicemeeter (typically called on software startup).
	@return :	 0: OK (no error).
				 1: OK but Voicemeeter Application not launched.
				-1: cannot get client (unexpected)
				-2: unexpected login (logout was expected before).
	*/

long __stdcall VBVMR_Login(void);

	/** 
	@brief Close Communication Pipe With Voicemeeter (typically called on software end).
	@return : 0 if ok.
	*/

long __stdcall VBVMR_Logout(void);


	/** 
	@brief Run Voicemeeter Application (get installation directory and run Voicemeeter Application).
	@param vType : Voicemeeter type (1 = Voicemeeter, 2= Voicemeeter Banana).
	@return :	 0: Ok.
				-1: not installed
	*/

long __stdcall VBVMR_RunVoicemeeter(long vType);


/** @}  */











/******************************************************************************/
/*                                                                            */
/*                             General Information                            */
/*                                                                            */
/******************************************************************************/

/** @name General Information 
* @{ */

	/** 
	@brief Get Voicemeeter Type
	@param pType : Pointer on 32bit long receiving the type (1 = Voicemeeter, 2= Voicemeeter Banana).

				 VOICEMEETER STRIP/BUS INDEX ASSIGNMENT

				 | Strip 1 | Strip 2 |Virtual Input|  BUS A  |  BUS B  |
				 +---------+---------+-------------+---------+---------+
				 |    0    |    1    |      2      |    0    |    1    |

				 VOICEMEETER BANANA STRIP/BUS INDEX ASSIGNMENT

				 | Strip 1 | Strip 2 | Strip 2 |Virtual Input|Virtual AUX|BUS A1|BUS A2|BUS A3|BUS B1|BUS B2|
				 +---------+---------+---------+-------------+-----------+------+------+------+------+------+
				 |    0    |    1    |    2    |       3     |     4     |   0  |   1  |   2  |   3  |   4  |



	@return :	 0: OK (no error).
				-1: cannot get client (unexpected)
				-2: no server.
	*/

long __stdcall VBVMR_GetVoicemeeterType(long * pType);

	/** 
	@brief Get Voicemeeter Version
	@param pType : Pointer on 32bit integer receiving the version (v1.v2.v3.v4)
						v1 = (version & 0xFF000000)>>24;
						v2 = (version & 0x00FF0000)>>16;
						v3 = (version & 0x0000FF00)>>8;
						v4 = version & 0x000000FF;

	@return :	 0: OK (no error).
				-1: cannot get client (unexpected)
				-2: no server.
	*/

long __stdcall VBVMR_GetVoicemeeterVersion(long * pVersion);


/** @}  */


















/******************************************************************************/
/*                                                                            */
/*                               Get parameters                               */
/*                                                                            */
/******************************************************************************/

/** @name Getting Parameters
* @{ */

	/** 
	@brief  Check if parameters have changed.
			Call this function periodically (typically every 10 or 20ms).
			(this function must be called from one thread only)
					
	@return:	 0: no new paramters.
				 1: New parameters -> update your display.
				-1: error (unexpected)
				-2: no server.
	*/

long __stdcall VBVMR_IsParametersDirty(void);

	/** 
	@brief get parameter value.
	@param szParamName : Null Terminal ASCII String giving the name of the parameter (see parameters name table)
	@param pValue : Pointer on float (32bit float by reference) receiving the wanted value.
	@return :	 0: OK (no error).
				-1: error
				-2: no server.
				-3: unknown parameter
				-5: structure mismatch
	*/

long __stdcall VBVMR_GetParameterFloat(char * szParamName, float * pValue);

	/** 
	@brief get parameter value.
	@param szParamName : Null Terminal ASCII String giving the name of the parameter (see parameters name table)
	@param pValue : Pointer on String (512 char or wchar) receiving the wanted value.
	@return :	 0: OK (no error).
				-1: error
				-2: no server.
				-3: unknown parameter
				-5: structure mismatch
	*/

long __stdcall VBVMR_GetParameterStringA(char * szParamName, char * szString);
long __stdcall VBVMR_GetParameterStringW(char * szParamName, unsigned short * wszString);

/** @}  */









/******************************************************************************/
/*                                                                            */
/*                                Get levels                                  */
/*                                                                            */
/******************************************************************************/

/** @name Getting RT Data
* @{ */

	/** 
	@brief Get Current levels.
			(this function must be called from one thread only)

	@param nType:	0= pre fader input levels.
					1= post fader input levels.
					2= post Mute input levels.
					3= output levels.

	@param nuChannel: audio channel zero based index 
						for input 0 = in#1 left, 1= in#1 Right, etc...
						for output 0 = busA ch1, 1 = busA ch2...

					 VOICEMEETER CHANNEL ASSIGNMENT

					 | Strip 1 | Strip 2 |             Virtual Input             |
					 +----+----+----+----+----+----+----+----+----+----+----+----+
					 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 |

					 |             Output A1 / A2            |             Virtual Output            |
					 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
					 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 |

					 VOICEMEETER BANANA CHANNEL ASSIGNMENT

					 | Strip 1 | Strip 2 | Strip 3 |             Virtual Input             |            Virtual Input AUX          |
					 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
					 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 |

					 |             Output A1                 |                Output A2              |                Output A3              |
					 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
					 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 |

					 |            Virtual Output B1          |             Virtual Output B2         |
					 +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
					 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 |


	@return :	 0: OK (no error).
				-1: error
				-2: no server.
				-3: no level available
				-4: out of range
	*/

long __stdcall VBVMR_GetLevel(long nType, long nuChannel, float * pValue);


	/** 
	@brief Get MIDI message from M.I.D.I. input device used by Voicemeeter M.I.D.I. mapping.
			(this function must be called from one thread only)

	@param pMIDIBuffer:	pointer on MIDI Buffer. Expected message size is below 4 bytes, 
	                    but it's recommended to use 1024 Bytes local buffer to receive 
						possible multiple M.I.D.I. event message in optimal way: 
						unsigned char pBuffer[1024];


	@return :	>0: number of bytes placed in buffer (2 or 3 byte for usual M.I.D.I. message) 
				-1: error
				-2: no server.
				-5: no MIDI data
				-6: no MIDI data
	*/


long __stdcall VBVMR_GetMidiMessage(unsigned char *pMIDIBuffer, long nbByteMax);


/** @}  */










/******************************************************************************/
/*                                                                            */
/*                               Set Parameters                               */
/*                                                                            */
/******************************************************************************/

/** @name Setting Parameters
* @{ */

	/** 
	@brief Set a single float 32 bits parameters .
	@param szParamName : Null Terminal ASCII String giving the name of the parameter (see parameters name table)
						example:
						Strip[1].gain
						Strip[0].mute
						Bus[0].gain
						Bus[0].eq.channel[0].cell[0].gain

	@param pValue : float 32bit containing the new value.
	@return :	 0: OK (no error).
				-1: error
				-2: no server.
				-3: unknown parameter
	*/

long __stdcall VBVMR_SetParameterFloat(char * szParamName, float Value);



	/** 
	@brief Set a single string parameters .
	@param szParamName : Null Terminal ASCII String giving the name of the parameter (see parameters name table)
						example:
						Strip[1].name
						Strip[0].device.mme
						Bus[0].device.asio

	@param szString : zero terminal string.
	@return :	 0: OK (no error).
				-1: error
				-2: no server.
				-3: unknown parameter
	
	*/


long __stdcall VBVMR_SetParameterStringA(char * szParamName, char * szString);
long __stdcall VBVMR_SetParameterStringW(char * szParamName, unsigned short * wszString);




	/** 
	@brief Set one or several parameters by a script ( < 48 kB ).
	@param szParamName : Null Terminal ASCII String giving the script 
						 (script allows to change several parameters in the same time - SYNCHRO).
						 Possible Instuction separators: ',' ';' or '\n'(CR)
						 EXAMPLE:
						 "Strip[0].gain = -6.0
						 Strip[0].A1 = 0
						 Strip[0].B1 = 1
						 Strip[1].gain = -6.0
						 Strip[2].gain = 0.0
						 Strip[3].name = "Skype Caller" "

	@return :	 0: OK (no error).
				>0: number of line causing script error.
				-1: error
				-2: no server.
				-3: unexpected error
				-4: unexpected error
	*/

long __stdcall VBVMR_SetParameters(char * szParamScript);
long __stdcall VBVMR_SetParametersW(unsigned short * szParamScript);


/** @}  */











/******************************************************************************/
/*                            DEVICES ENUMERATOR                              */
/******************************************************************************/


/** @name Device Enumeration Functions
* @{ */

#define VBVMR_DEVTYPE_MME		1
#define VBVMR_DEVTYPE_WDM		3
#define VBVMR_DEVTYPE_KS		4
#define VBVMR_DEVTYPE_ASIO		5

	/** 
	@brief Get number of Audio Output Device available on the system
	@return : return number of device found.
	*/

long __stdcall VBVMR_Output_GetDeviceNumber(void);

	/** 
	@brief Return pointer on Output Device Descriptor according index
	@param zindex : zero based index
	@param nType : Pointer on 32bit long receiving the type (pointer can be NULL).
	@param szName : Pointer on string (256 char min) receiving the device name (pointer can be NULL).
	@param szHardwareId : Pointer on string (256 char min) receiving the hardware ID (pointer can be NULL).
	@return :	 0: OK (no error).
	*/

long __stdcall VBVMR_Output_GetDeviceDescA(long zindex, long * nType, char * szDeviceName, char * szHardwareId);
long __stdcall VBVMR_Output_GetDeviceDescW(long zindex, long * nType, unsigned short * wszDeviceName, unsigned short * wszHardwareId);

	/** 
	@brief Get number of Audio Input Device available on the system
	@return : return number of device found.
	*/

long __stdcall VBVMR_Input_GetDeviceNumber(void);

	/** 
	@brief Return pointer on Input Device Descriptor according index
	@param zindex : zero based index
	@param nType : Pointer on 32bit long receiving the type (pointer can be NULL).
	@param szName : Pointer on string (256 char min) receiving the device name (pointer can be NULL).
	@param szHardwareId : Pointer on string (256 char min) receiving the hardware ID (pointer can be NULL).
	@return :	 0: OK (no error).
	*/

long __stdcall VBVMR_Input_GetDeviceDescA(long zindex, long * nType, char * szDeviceName, char * szHardwareId);
long __stdcall VBVMR_Input_GetDeviceDescW(long zindex, long * nType, unsigned short * wszDeviceName, unsigned short * wszHardwareId);



/** @}  */










/******************************************************************************/
/*                          'C' STRUCTURED INTERFACE                          */
/******************************************************************************/

typedef long (__stdcall *T_VBVMR_Login)(void);
typedef long (__stdcall *T_VBVMR_Logout)(void);
typedef long (__stdcall *T_VBVMR_RunVoicemeeter)(long vType);

typedef long (__stdcall *T_VBVMR_GetVoicemeeterType)(long * pType);
typedef long (__stdcall *T_VBVMR_GetVoicemeeterVersion)(long * pVersion);

typedef long (__stdcall *T_VBVMR_IsParametersDirty)(void);
typedef long (__stdcall *T_VBVMR_GetParameterFloat)(char * szParamName, float * pValue);
typedef long (__stdcall *T_VBVMR_GetParameterStringA)(char * szParamName, char * szString);
typedef long (__stdcall *T_VBVMR_GetParameterStringW)(char * szParamName, unsigned short * wszString);


typedef long (__stdcall *T_VBVMR_GetLevel)(long nType, long nuChannel, float * pValue);
typedef long (__stdcall *T_VBVMR_GetMidiMessage)(unsigned char *pMIDIBuffer, long nbByteMax);

typedef long (__stdcall *T_VBVMR_SetParameterFloat)(char * szParamName, float Value);
typedef long (__stdcall *T_VBVMR_SetParameters)(char * szParamScript);
typedef long (__stdcall *T_VBVMR_SetParametersW)(unsigned short * szParamScript);
typedef long (__stdcall *T_VBVMR_SetParameterStringA)(char * szParamName, char * szString);
typedef long (__stdcall *T_VBVMR_SetParameterStringW)(char * szParamName, unsigned short * wszString);

typedef long (__stdcall *T_VBVMR_Output_GetDeviceNumber)(void);
typedef long (__stdcall *T_VBVMR_Output_GetDeviceDescA)(long zindex, long * nType, char * szDeviceName, char * szHardwareId);
typedef long (__stdcall *T_VBVMR_Output_GetDeviceDescW)(long zindex, long * nType, unsigned short * wszDeviceName, unsigned short * wszHardwareId);
typedef long (__stdcall *T_VBVMR_Input_GetDeviceNumber)(void);
typedef long (__stdcall *T_VBVMR_Input_GetDeviceDescA)(long zindex, long * nType, char * szDeviceName, char * szHardwareId);
typedef long (__stdcall *T_VBVMR_Input_GetDeviceDescW)(long zindex, long * nType, unsigned short * wszDeviceName, unsigned short * wszHardwareId);



typedef struct tagVBVMR_INTERFACE
{
	T_VBVMR_Login					VBVMR_Login;
	T_VBVMR_Logout					VBVMR_Logout;
	T_VBVMR_RunVoicemeeter			VBVMR_RunVoicemeeter;
	T_VBVMR_GetVoicemeeterType		VBVMR_GetVoicemeeterType;
	T_VBVMR_GetVoicemeeterVersion	VBVMR_GetVoicemeeterVersion;
	T_VBVMR_IsParametersDirty		VBVMR_IsParametersDirty;
	T_VBVMR_GetParameterFloat		VBVMR_GetParameterFloat;
	T_VBVMR_GetParameterStringA		VBVMR_GetParameterStringA;
	T_VBVMR_GetParameterStringW		VBVMR_GetParameterStringW;

	T_VBVMR_GetLevel				VBVMR_GetLevel;
	T_VBVMR_GetMidiMessage			VBVMR_GetMidiMessage;

	T_VBVMR_SetParameterFloat		VBVMR_SetParameterFloat;
	T_VBVMR_SetParameters			VBVMR_SetParameters;
	T_VBVMR_SetParametersW			VBVMR_SetParametersW;
	T_VBVMR_SetParameterStringA		VBVMR_SetParameterStringA;
	T_VBVMR_SetParameterStringW		VBVMR_SetParameterStringW;

	T_VBVMR_Output_GetDeviceNumber	VBVMR_Output_GetDeviceNumber;
	T_VBVMR_Output_GetDeviceDescA	VBVMR_Output_GetDeviceDescA;
	T_VBVMR_Output_GetDeviceDescW	VBVMR_Output_GetDeviceDescW;
	T_VBVMR_Input_GetDeviceNumber	VBVMR_Input_GetDeviceNumber;
	T_VBVMR_Input_GetDeviceDescA	VBVMR_Input_GetDeviceDescA;
	T_VBVMR_Input_GetDeviceDescW	VBVMR_Input_GetDeviceDescW;

} T_VBVMR_INTERFACE, *PT_VBVMR_INTERFACE, *LPT_VBVMR_INTERFACE;

#ifdef VBUSE_LOCALLIB
	// internal used (not public)
	void __stdcall VBVMR_SetHinstance(HINSTANCE hinst);
#endif

#ifdef __cplusplus
}
#endif

#endif /*__VOICEMEETER_REMOTE_H__*/


