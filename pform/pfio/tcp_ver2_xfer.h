// tcp_ver2_xfer struction is used for all tcp transfers in version2 client/server.  It
// allows sending of binary and binary-compressed data.
// There is a command word.  This code still uses the "PR " "GN " and other signatures in
// the data part, this command will be used in the future.

#if !defined (__TCP_ver2_xfer_H__)
#define __TCP_ver2_xfer_H__

// The "correct" length for allocation of a non-compressed null terminated string is
// tcp_ver2_xfer *p = (tcp_ver2_xfer *)malloc(sizeof(tcp_ver2_xfer)+strlen(string));
// memset(p, 0, sizeof(tcp_ver2_xfer));
// p->dwUnpackedLen = p->dwPackedLen = strlen(string)+1;
// strcpy(p->cpData, string);  // note that the null byte is accounted for in the original size of the structure.

// For allocating a binary data blob, you can allocate 1 byte less than the length (plus len of tcp_ver2_xfer)
// structure, but there really is no point.

// For compressed data, allocate 1 byte less than length (plus len of  tcp_ver2_xfer).  Put the compressed len
// into the dwPackedLen, and put the UNCOMPRESSED original len into dwUnpackedLen.  Also put the correct enum
// for the compression type (currently only zLib is supported, since it does well).

#define PFGW_NETWORK_V2_MACNAME_LEN 40

enum { e_COMPRESSION_NONE, e_COMPRESSION_ZLIB, e_COMPRESSION_COMPRESS };  // only ZLIB implemented at this time.

#pragma pack( push, before_tcp_ver2_xfer )
#pragma pack(1)
struct tcp_ver2_xfer
{
	uint32	u32UnpackedLen;		// Length of cpData after it is unpacked (or same as u32PackedLen if not packed)
	uint32	u32PackedLen;		// Length of cpData in this packet (compressed length)
	uint8	u8Command;
	uint8   u8ProtocolVersion;
	uint8	eCompressed;		// Compression which this packet is compressed in
	uint8	eRCompress;			// Compression request for server (should it reply compressed or not)

	// Must be null terminated, so all names MUST be truncated to 39 bytes.
	char	szMachineName[PFGW_NETWORK_V2_MACNAME_LEN];

	uint32	u32Reserved[4];
	char	cpData[1];			// This will be "variable" sized and possibly compressed.
};
#pragma pack( pop, before_tcp_ver2_xfer )

#endif
