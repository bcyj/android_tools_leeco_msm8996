#ifndef HEADER_FILE_MD5
#define HEADER_FILE_MD5

/* MD5 context. */
typedef struct {
    unsigned long int state[4]; /* state (ABCD) */
    unsigned long int count[2]; /* number of bits, modulo 2^64 (lsb first) */
    unsigned char buffer[64]; /* input buffer */
} MD5_CTX;

#define HASHLEN 16
typedef char HASH[HASHLEN];
#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN + 1];

void DM_MD5Init(MD5_CTX *);
void DM_MD5Update(MD5_CTX *, char *, unsigned int);
void DM_MD5Final(char[16], MD5_CTX *);
void DM_CvtHex(HASH Bin, HASHHEX Hex);
void DM_DigestCalcHA1(char * pszAlg, char * pszUserName, char * pszRealm,
        char * pszPassword, char * pszNonce, char * pszCNonce,
        HASHHEX SessionKey);
void DM_DigestCalcResponse(HASHHEX HA1, /* H(A1) */
char * pszNonce, /* nonce from server */
char * pszNonceCount, /* 8 hex digits */
char * pszCNonce, /* client nonce */
char * pszQop, /* qop-value: "", "auth", "auth-int" */
char * pszMethod, /* method from the request */
char * pszDigestUri, /* requested URL */
HASHHEX HEntity, /* H(entity body) if qop="auth-int" */
HASHHEX Response /* request-digest or response-digest */
);

#endif

