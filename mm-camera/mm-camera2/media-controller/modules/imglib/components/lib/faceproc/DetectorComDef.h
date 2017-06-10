
#ifndef DETECTORCOMDEF_H__
#define DETECTORCOMDEF_H__
/* Common Definition for Detector Module.*/


/* Roll Angle Definitions. */
#define ROLL_ANGLE_0     (0x00001001)  /* Up            0 degree */
#define ROLL_ANGLE_1     (0x00002002)  /* Upper Right  30 degree */
#define ROLL_ANGLE_2     (0x00004004)  /* Upper Right  60 degree */
#define ROLL_ANGLE_3     (0x00008008)  /* Right        90 degree */
#define ROLL_ANGLE_4     (0x00010010)  /* Lower Right 120 degree */
#define ROLL_ANGLE_5     (0x00020020)  /* Lower Right 150 degree */
#define ROLL_ANGLE_6     (0x00040040)  /* Down        180 degree */
#define ROLL_ANGLE_7     (0x00080080)  /* Lower Left  210 degree */
#define ROLL_ANGLE_8     (0x00100100)  /* Lower Left  240 degree */
#define ROLL_ANGLE_9     (0x00200200)  /* Left        270 degree */
#define ROLL_ANGLE_10    (0x00400400)  /* Upper Left  300 degree */
#define ROLL_ANGLE_11    (0x00800800)  /* Upper Left  330 degree */

#define ROLL_ANGLE_ALL      (0x00ffffff)     /* All angles are detected                            */
#define ROLL_ANGLE_NONE     (0x00000000)     /* None of the angles will be detected                */
#define ROLL_ANGLE_U15      (ROLL_ANGLE_0)   /* Up    +/- 15 degree only will be detected          */
#define ROLL_ANGLE_R15      (ROLL_ANGLE_3)   /* Right +/- 15 degree only will be detected          */
#define ROLL_ANGLE_D15      (ROLL_ANGLE_6)   /* Down  +/- 15 degree only will be detected          */
#define ROLL_ANGLE_L15      (ROLL_ANGLE_9)   /* Left  +/- 15 degree only will be detected          */

#define ROLL_ANGLE_UP       (ROLL_ANGLE_U15)   /* Up    +/- 15 degree only will be detected          */
#define ROLL_ANGLE_RIGHT    (ROLL_ANGLE_R15)   /* Right +/- 15 degree only will be detected          */
#define ROLL_ANGLE_DOWN     (ROLL_ANGLE_D15)   /* Down  +/- 15 degree only will be detected          */
#define ROLL_ANGLE_LEFT     (ROLL_ANGLE_L15)   /* Left  +/- 15 degree only will be detected          */


#define ROLL_ANGLE_U45       (ROLL_ANGLE_0|ROLL_ANGLE_1|ROLL_ANGLE_11)    /* Up +/- 45 degree only will be detected             */
#define ROLL_ANGLE_ULR15     (ROLL_ANGLE_0|ROLL_ANGLE_3|ROLL_ANGLE_9)     /* Up-Left-Right +/- 15 degree only will be detected  */
#define ROLL_ANGLE_ULR45     (ROLL_ANGLE_0|ROLL_ANGLE_1|ROLL_ANGLE_2|ROLL_ANGLE_3|ROLL_ANGLE_4|ROLL_ANGLE_8|ROLL_ANGLE_9|ROLL_ANGLE_10|ROLL_ANGLE_11)
                                                    /* Up-Left-Right +/- 45 degree only will be detected  */

/* Rotation Angle Extention */
#define ROTATION_ANGLE_EXT0     (0x00000000)      /* No rotation angle expansion                            */
#define ROTATION_ANGLE_EXT1     (0x00001001)      /* Rotation angle expansion: Upto left-right 1 direction  */
#define ROTATION_ANGLE_EXT2     (0x00002002)      /* Rotation angle expansion: Upto left-right 2 directions */
#define ROTATION_ANGLE_EXTALL   (0x0000b00b)      /* Rotation angle expansion: In all directions            */


/*  Detection mode */
#define DETECTION_MODE_DEFAULT     (0)   /* Default Mode                              */
#define DETECTION_MODE_MOTION1     (1)   /* Motion Mode: Whole search mode            */
#define DETECTION_MODE_MOTION2     (2)   /* Motion Mode: 3 Partition search mode      */
#define DETECTION_MODE_MOTION3     (3)   /* Motion Mode: Gradual progress search mode */

#define DETECTION_MODE_STILL       (4)   /* Still Mode                                */
#define DETECTION_MODE_MOVIE       (5)   /* Movie Mode                                */

/* Detection Accuracy */
#define DETECTION_ACCURACY_NORMAL   (0)   /* Normal Accuracy mode                      */
#define DETECTION_ACCURACY_HIGH     (1)   /* High Accuracy mode                        */


#endif  /* DETECTORCOMDEF_H__ */

