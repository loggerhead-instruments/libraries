#define ULONG unsigned long
#define DFORM_LONG 4
#define DFORM_I24 3
#define DFORM_SHORT 2
#define EVTYPE_STREAM 0x00008101
#define SID_MAX 4

struct TIME_HEAD
{
	byte 	sec;  
	byte 	minute;  
	byte 	hour;  
	byte 	day;  
	byte 	mday;  
	byte 	month;  
	byte 	year;  
	byte	timezone;  
};

struct DF_HEAD
{
	ULONG Version;  
  	ULONG UserID;
  	TIME_HEAD RecStartTime;
	float Lat;
	float Lon;
	float depth;
	float DSGcal;
	float hydroCal;
	float lpFilt;
};

// DM added RECPTS and RECINT
struct SID_SPEC
{
	ULONG	SID;
	ULONG 	nBytes;			// Size in bytes of this record (excluding header)
	ULONG	NumChan;		// Number of expected channels in store
	ULONG	StoreType;		// See TTank stuff
	ULONG	SensorType;		// used to encode what data are saved: bitmask bit 1-5 (accel, magnetometer, gyro, press,mic)
	ULONG	DForm;			// See TTank stuff
	ULONG	SPus;			// Sample period us 
	ULONG 	RECPTS;			// rec points =0 for continuous; otherwise stutter
	ULONG	RECINT;		    // interval between rec pts; 
};

struct SID_REC
{
	byte	nSID;			// This is record I/D
	byte	Chan;			// Channel indicating which sensors are stored
	ULONG	nbytes;			// Number of bytes recorded since start of sampling for this SID
    ULONG   nbytes_2;        // roll counter for bytestamp (arduino does not support UINT64)
};
