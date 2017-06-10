/****************************************************************************

**

** Copyright (C) 2013 Reverie Language Technologies.

** http://www.reverie.co.in

**

** This file is part of Rendering engine definitions.

****************************************************************************/



#define deva_    0x61766564
#define arab_    0x62617261
#define taml_    0x6c6d6174
#define gur2_	 0x32727567
#define knda_	 0x61646e6b
#define knd2_	 0x32646e6b
#define beng_	 0x676e6562
#define bng2_    0x32676e62
#define thai_    0x69616874
#define gujr_    0x726a7567
#define mlym_    0x6d796c6d
#define orya_    0x6179726f
#define tel2_    0x326c6574


#define deva_L    0x64657661
#define arab_L    0x61726162
#define taml_L    0x74616d6c
#define gur2_L	  0x67757232
#define knda_L	  0x6b6e6461
#define knd2_L	  0x6b6e6432
#define beng_L	  0x62656e67
#define bng2_L    0x626e6732
#define thai_L    0x74686169
#define gujr_L    0x67756a72
#define mlym_L    0x6d6c796d
#define orya_L    0x6f727961
#define tel2_L    0x74656c32

#define ccmp_    0x706d6363
#define isol_    0x6c6f7369
#define fina_    0x616e6966
#define medi_    0x6964656d
#define init_    0x74696e69
#define rlig_    0x67606c72
#define calt_    0x746c6163
#define cjct_    0x74636a63
#define liga_    0x6167696c
#define dlig_    0x67696c64
#define cswh_    0x68777363
#define mset_    0x7465736d
#define abvs_    0x73766261
#define akhn_    0x6e686b61
#define blwf_    0x66776c62
#define blws_    0x73776c62
#define haln_    0x6e6c6168
#define half_    0x666c6168
#define nukt_    0x746b756e
#define psts_    0x73747370
#define pstf_    0x66747370
#define pres_    0x73657270
#define pref_    0x66657270
#define rphf_    0x66687072
#define vatu_    0x75746176
#define locl_    0x6c6f636c

#define ccmp_L    0x63636d70
#define isol_L    0x69736f6c
#define fina_L    0x66696e61
#define medi_L    0x6d656469
#define init_L    0x696e6974
#define rlig_L    0x726c6967
#define calt_L    0x63616c74
#define cjct_L    0x636a6374
#define liga_L    0x6c696761
#define dlig_L    0x646c6967
#define cswh_L    0x63737768
#define mset_L    0x6d736574
#define abvs_L    0x61627673
#define akhn_L    0x616b686e
#define blwf_L    0x626c7766
#define blws_L    0x626c7773
#define haln_L    0x68616c6e
#define half_L    0x68616c66
#define nukt_L    0x6e756b74
#define psts_L    0x70737473
#define pstf_L    0x70737466
#define pres_L    0x70726573
#define pref_L    0x70726566
#define rphf_L    0x72706866
#define vatu_L    0x76617475
#define locl_L    0x6c636f6c

#define abvm_    0x6d766261
#define blwm_    0x6d776c62

#define abvm_L    0x6162766d
#define blwm_L    0x626c776d


//Indic Scripts Features list in order
//Scripts
#define DEVA            0
#define ARABIC          1
#define TAML            2
#define GURM            3
#define KNDA            4
#define BENG            5
#define GUJR            6
#define ORYA            7
#define MLYM            8
#define TELG            9
#define THAI            10
#define NSCRIPTS        11

//GSUB
#define LOCL    0
#define NUKT    1
#define AKHN    2
#define RPHF    3
#define PREF    4
#define BLWF    5
#define HALF    6
#define PSTF    7
#define VATU    8
#define CJCT    9
#define PRES    10
#define ABVS    11
#define BLWS    12
#define PSTS    13
#define HALN    14
#define CALT    15
#define ABVMS   16
#define BLWMS   17
#define CCMP    18
#define ISOL    19
#define FINA    20
#define MEDI    21
#define INIT    22
#define RLIG    23
#define LIGA    24
#define DLIG    25
#define CSWH    26
#define MSET    27

#define NGSUBFEATURES   28
#define DVNGSUBFEATURES   18

//GPOS
#define kern_    0x6e72656b
#define curs_    0x73727563
#define mark_    0x6b72616d
#define mkmk_    0x6b6d6b6d

#define kern_L    0x6b65726E
#define curs_L    0x63757273
#define mark_L    0x6d61726b
#define mkmk_L    0x6d6b6d6b



#define KERN    0
#define DIST    1
#define ABVM    2
#define BLWM    3
#define CURS    4
#define MARK    5
#define MKMK    6
#define NGPOSFEATURES   7




struct SCRIPTFEATURES{
	char *Feature[NGSUBFEATURES];
};

typedef struct TT_GSUB_{
    unsigned char *Header;
    unsigned short scripts;
    unsigned char *script;
    struct SCRIPTFEATURES Script[NSCRIPTS];
    unsigned short Features;
    unsigned char *Feature;
    unsigned short LookupCount;
    unsigned char *Lookup;
}TT_GSUB;

typedef struct TT_GDEF_{
    unsigned char *Header;
    unsigned char *GlyphClassDef;
}TT_GDEF;

typedef struct TT_GPOS_{
	unsigned char *Header;
	unsigned short ScriptCount;
	unsigned short FeatureCount;
	unsigned short LookupCount;
	unsigned short *offset;
	unsigned short *lookup;
	unsigned short *subTable;
}TT_GPOS;

